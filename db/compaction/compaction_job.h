//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).
//
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#pragma once

#include <db/thrift/gen/rpc_types.h>

#include <atomic>
#include <deque>
#include <functional>
#include <limits>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "db/column_family.h"
#include "db/compaction/compaction_iterator.h"
#include "db/dbformat.h"
#include "db/flush_scheduler.h"
#include "db/internal_stats.h"
#include "db/job_context.h"
#include "db/log_writer.h"
#include "db/memtable_list.h"
#include "db/range_del_aggregator.h"
#include "db/version_edit.h"
#include "db/write_controller.h"
#include "db/write_thread.h"
#include "logging/event_logger.h"
#include "options/cf_options.h"
#include "options/db_options.h"
#include "port/port.h"
#include "rocksdb/compaction_filter.h"
#include "rocksdb/compaction_job_stats.h"
#include "rocksdb/db.h"
#include "rocksdb/env.h"
#include "rocksdb/memtablerep.h"
#include "rocksdb/transaction_log.h"
#include "table/scoped_arena_iterator.h"
#include "util/autovector.h"
#include "util/stop_watch.h"
#include "util/thread_local.h"

namespace ROCKSDB_NAMESPACE {

class Arena;
class ErrorHandler;
class MemTable;
class SnapshotChecker;
class TableCache;
class Version;
class VersionEdit;
class VersionSet;

// CompactionJob is responsible for executing the compaction. Each (manual or
// automated) compaction corresponds to a CompactionJob object, and usually
// goes through the stages of `Prepare()`->`Run()`->`Install()`. CompactionJob
// will divide the compaction into subcompactions and execute them in parallel
// if needed.
class CompactionJob {
 public:
  CompactionJob(
      int job_id, Compaction* compaction, const ImmutableDBOptions& db_options,
      const FileOptions& file_options, VersionSet* versions,
      const std::atomic<bool>* shutting_down,
      const SequenceNumber preserve_deletes_seqnum, LogBuffer* log_buffer,
      FSDirectory* db_directory, FSDirectory* output_directory,
      Statistics* stats, InstrumentedMutex* db_mutex,
      ErrorHandler* db_error_handler,
      std::vector<SequenceNumber> existing_snapshots,
      SequenceNumber earliest_write_conflict_snapshot,
      const SnapshotChecker* snapshot_checker,
      std::shared_ptr<Cache> table_cache, EventLogger* event_logger,
      bool paranoid_file_checks, bool measure_io_stats,
      const std::string& dbname, CompactionJobStats* compaction_job_stats,
      Env::Priority thread_pri, const std::shared_ptr<IOTracer>& io_tracer,
      const std::atomic<int>* manual_compaction_paused = nullptr,
      const std::string& db_id = "", const std::string& db_session_id = "");

  ~CompactionJob();

  // no copy/move
  CompactionJob(CompactionJob&& job) = delete;
  CompactionJob(const CompactionJob& job) = delete;
  CompactionJob& operator=(const CompactionJob& job) = delete;

  // REQUIRED: mutex held
  // Prepare for the compaction by setting up boundaries for each subcompaction
  void Prepare();
  // REQUIRED mutex not held
  // Launch threads for each subcompaction and wait for them to finish. After
  // that, verify table is usable and finally do bookkeeping to unify
  // subcompaction results
  Status Run();

  // REQUIRED: mutex held
  // Add compaction input/output to the current version
  Status Install(const MutableCFOptions& mutable_cf_options);

  void LogCompactionEnd(ColumnFamilyData* cfd, Status& status);
  void CleanupCompaction();

  // Return the IO status
  IOStatus io_status() const { return io_status_; }

 private:
  struct SubcompactionState;

  void AggregateStatistics();

  // Generates a histogram representing potential divisions of key ranges from
  // the input. It adds the starting and/or ending keys of certain input files
  // to the working set and then finds the approximate size of data in between
  // each consecutive pair of slices. Then it divides these ranges into
  // consecutive groups such that each group has a similar size.
  void GenSubcompactionBoundaries();

  // update the thread status for starting a compaction.
  void ReportStartedCompaction(Compaction* compaction);

  // Call compaction filter. Then iterate through input and compact the
  // kv-pairs
  void ProcessKeyValueCompaction(SubcompactionState* sub_compact);
  void ProcessLocalKVCompaction(SubcompactionState* sub_compact);
  void ProcessRemoteKVCompaction(SubcompactionState* sub_compact);
  void GenFileNumbers();
  void PushResultToNode(TCompactionResult& result, ClusterNode* node) const;
  void PushCompactionOutputToNodes(FileDescriptor& output, char* buf);

  Status FinishCompactionOutputFile(
      const Status& input_status, SubcompactionState* sub_compact,
      CompactionRangeDelAggregator* range_del_agg,
      CompactionIterationStats* range_del_out_stats,
      const Slice* next_table_min_key = nullptr);
  Status InstallCompactionResults(const MutableCFOptions& mutable_cf_options);
  void RecordCompactionIOStats();
  Status OpenCompactionOutputFile(SubcompactionState* sub_compact);

  void UpdateCompactionJobStats(
      const InternalStats::CompactionStats& stats) const;
  void RecordDroppedKeys(const CompactionIterationStats& c_iter_stats,
                         CompactionJobStats* compaction_job_stats = nullptr);

  void UpdateCompactionStats();
  void UpdateCompactionInputStatsHelper(int* num_files, uint64_t* bytes_read,
                                        int input_level);

  void LogCompaction();

  void assignSubJobNode();
  static void ConstructCompactionOutPut(TFileMetadata& file_meta,
                                        SubcompactionState* sub_comp);
  void AdvanceOtherFileNumbers(uint64_t new_compaction_num);
  void LogSubcompactions();

  int job_id_;
  uint32_t curr_node_index = 0;

  // CompactionJob state
  struct CompactionState;
  CompactionState* compact_;
  CompactionJobStats* compaction_job_stats_;
  InternalStats::CompactionStats compaction_stats_;

  // DBImpl state
  const std::string& dbname_;
  const std::string db_id_;
  const std::string db_session_id_;
  const ImmutableDBOptions& db_options_;
  const FileOptions file_options_;

  Env* env_;
  FileSystemPtr fs_;
  // env_option optimized for compaction table reads
  FileOptions file_options_for_read_;
  VersionSet* versions_;
  const std::atomic<bool>* shutting_down_;
  const std::atomic<int>* manual_compaction_paused_;
  const SequenceNumber preserve_deletes_seqnum_;
  LogBuffer* log_buffer_;
  FSDirectory* db_directory_;
  FSDirectory* output_directory_;
  Statistics* stats_;
  InstrumentedMutex* db_mutex_;
  ErrorHandler* db_error_handler_;
  // If there were two snapshots with seq numbers s1 and
  // s2 and s1 < s2, and if we find two instances of a key k1 then lies
  // entirely within s1 and s2, then the earlier version of k1 can be safely
  // deleted because that version is not visible in any snapshot.
  std::vector<SequenceNumber> existing_snapshots_;

  // This is the earliest snapshot that could be used for write-conflict
  // checking by a transaction.  For any user-key newer than this snapshot, we
  // should make sure not to remove evidence that a write occurred.
  SequenceNumber earliest_write_conflict_snapshot_;

  const SnapshotChecker* const snapshot_checker_;

  std::shared_ptr<Cache> table_cache_;

  EventLogger* event_logger_;

  // Is this compaction creating a file in the bottom most level?
  bool bottommost_level_;
  bool paranoid_file_checks_;
  bool measure_io_stats_;
  // Stores the Slices that designate the boundaries for each subcompaction
  std::vector<Slice> boundaries_;
  // Stores the approx size of keys covered in the range of each subcompaction
  std::vector<uint64_t> sizes_;
  std::vector<uint64_t> start_file_nums;
  Env::WriteLifeTimeHint write_hint_;
  Env::Priority thread_pri_;
  IOStatus io_status_;
  int max_file_per_sub_comp = 100;
  int new_file_start_num;

  uint64_t file_start_num;
  uint64_t max_file_num;
  bool is_remote = false;
  std::vector<FileMetaData> compact_output;

 public:
  uint64_t getFileStartNum() const;
  void setFileStartNum(uint64_t fileStartNum);
  uint64_t getMaxFileNum() const;
  void setMaxFileNum(uint64_t maxFileNum);
  bool isRemote() const;
  void setIsRemote(bool isRemote);

  const std::vector<FileMetaData>& getCompactOutput() const;
};

}  // namespace ROCKSDB_NAMESPACE
