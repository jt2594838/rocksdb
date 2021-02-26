#include "rocks_service.h"

#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <iostream>

#include "db/thrift/RpcUtils.h"
#include "db/version_edit.h"
#include "file/filename.h"
#include "rocksdb/write_batch.h"

int CORE_NUM = 30;

namespace ROCKSDB_NAMESPACE {

class WriteBatch;

void RocksService::CompactFiles(TCompactionResult &_return,
                                const TCompactFilesRequest &request) {
  CompactionOptions compactionOptions;
  std::vector<std::string> file_names;
  for (uint32_t i = 0; i < request.flush_nums.size(); ++i) {
    uint64_t flush_num = request.flush_nums[i];
    uint64_t compaction_num = request.compaction_nums[i];
    file_names.emplace_back(MakeTableFileName(flush_num, compaction_num));

    int file_level;
    FileMetaData *fileMetaData;
    ColumnFamilyData *cfd;
    while (db->versions_
               ->GetMetadataForFile(flush_num, compaction_num, &file_level,
                                    &fileMetaData, &cfd)
               .IsNotFound()) {
      usleep(100 * 1000);
    }
  }
  ROCKS_LOG_INFO(
      db->immutable_db_options_.info_log,
      "Received a compaction "
      "request of %ld files, range: [%s, %s], assigned file nums: [%ld, %ld)",
      file_names.size(), request.comp_start.c_str(), request.comp_end.c_str(),
      request.start_file_num, request.max_file_num + request.start_file_num);
  Slice begin(request.comp_start);
  Slice end(request.comp_end);
  Slice *_begin = request.comp_start.empty() ? nullptr : &begin;
  Slice *_end = request.comp_end.empty() ? nullptr : &end;
  std::vector<std::string> output_file_names;
  CompactionJobInfo jobInfo;
  Status status = db->CompactExactly(
      compactionOptions, db->DefaultColumnFamily(), file_names,
      request.output_level, request.start_file_num, request.max_file_num,
      _begin, _end, -1, &output_file_names, &jobInfo);
  for (const auto &meta : jobInfo.output_file_meta) {
    _return.output_files.emplace_back(RpcUtils::ToTFileMetaData(meta));
  }
  _return.status.code = status.code();
  _return.status.sub_code = status.subcode();
  _return.status.state =
      std::string(status.state_ == nullptr ? "" : status.state_);
  _return.status.severity = status.severity();
  _return.total_bytes = jobInfo.stats.total_output_bytes;
  _return.num_output_records = jobInfo.stats.num_output_records;

  std::string output_files_str = "[";
  for (auto &file : _return.output_files) {
    output_files_str.append(std::to_string(file.fd.merge_number)).append(" ");
  }
  output_files_str.append("]");
  ROCKS_LOG_INFO(db->immutable_db_options_.info_log,
                 "Compaction request "
                 "executed with result: "
                 "%s, and %ld files: %s",
                 status.ToString().c_str(), _return.output_files.size(),
                 output_files_str.c_str());
}

void RocksService::DownLoadFile(std::string &_return,
                                const std::string &file_name,
                                const int64_t offset, const int32_t size) {
  std::unique_ptr<RandomAccessFile> raf;
  Status s = db->env_->NewRandomAccessFile(file_name, &raf, envOptions);
  if (s.ok()) {
    Slice result;
    char *data = new char[size];
    raf->Read(offset, size, &result, data);
    _return = std::string(data, result.size());
  }
}

void RocksService::PushFiles(const TCompactionResult &result,
                             const std::string &source_ip,
                             const int32_t source_port) {
  auto source_node = ClusterNode(source_ip, source_port);
  for (const auto &meta : result.output_files) {
    FileMetaData &&file_metadata = RpcUtils::ToFileMetaData(meta);
    uint64_t flush_num = file_metadata.fd.GetFlushNumber();
    uint64_t compaction_num = file_metadata.fd.GetMergeNumber();
    uint64_t path_num = file_metadata.fd.GetPathId();
    const std::string &local_cf_path =
        db->default_cf_handle_->cfd()->ioptions()->cf_paths[path_num].path;
    const std::string &target_file_name =
        local_cf_path + "/" + MakeTableFileName(flush_num, compaction_num);
    if (!db->env_->FileExists(target_file_name).ok()) {
      const std::string &remote_path = result.db_paths[path_num];
      std::string source_file_path =
          remote_path + "/" + MakeTableFileName(flush_num, compaction_num);
      ROCKS_LOG_INFO(db->immutable_db_options_.info_log, "Downloading file %s",
                     source_file_path.c_str());
      RpcUtils::DownloadFile(source_file_path, &source_node, target_file_name,
                             db->immutable_db_options_.info_log);
      ROCKS_LOG_INFO(db->immutable_db_options_.info_log,
                     "File %s is downloaded at %s", source_file_path.c_str(),
                     target_file_name.c_str());
    }
  }
}

void RocksService::SetCompactionNumber(const int64_t new_compaction_num) {
  db->mutex()->Lock();
  if (db->versions_->GetCompactionNumber() <
      static_cast<uint64_t>(new_compaction_num)) {
    db->versions_->SetCompactionNumber(new_compaction_num);
    ROCKS_LOG_INFO(db->immutable_db_options_.info_log,
                   "New compaction number is "
                   "updated to %ld",
                   new_compaction_num);
  }
  db->mutex()->Unlock();
}

void RocksService::InstallCompaction(TStatus &_return,
                                     const TInstallCompactionRequest &request) {
  std::string input_files_str = "[";
  for (auto &file : request.deleted_inputs) {
    input_files_str.append(std::to_string(file.flush_num))
        .append("-")
        .append(std::to_string(file.compaction_num))
        .append(" ");
  }
  input_files_str.append("]");
  std::string output_files_str = "[";
  for (auto &file : request.installed_outputs) {
    output_files_str.append(std::to_string(file.metadata.fd.merge_number))
        .append(" ");
  }
  output_files_str.append("]");
  ROCKS_LOG_INFO(db->immutable_db_options_.info_log,
                 "Received a "
                 "compaction installation "
                 "request of %ld inputs: %s, %ld outputs: %s",
                 request.deleted_inputs.size(), input_files_str.c_str(),
                 request.installed_outputs.size(), output_files_str.c_str());

  VersionEdit edit;
  for (auto &deleted_file : request.deleted_inputs) {
    int level = deleted_file.level;
    uint64_t flush_num = deleted_file.flush_num;
    uint64_t compaction_num = deleted_file.compaction_num;

    int file_level;
    FileMetaData *fileMetaData;
    ColumnFamilyData *cfd;
    while (db->versions_
        ->GetMetadataForFile(flush_num, compaction_num, &file_level,
                             &fileMetaData, &cfd)
        .IsNotFound()) {
      usleep(100 * 1000);
    }
    edit.DeleteFile(level, flush_num, compaction_num);
  }

  for (auto &installed_file : request.installed_outputs) {
    int level = installed_file.level;
    FileMetaData &&metaData = RpcUtils::ToFileMetaData(installed_file.metadata);
    edit.AddFile(level, metaData);
  }

  edit.SetColumnFamily(db->DefaultColumnFamily()->GetID());
  db->mutex()->Lock();
  Status s = db->versions_->LogAndApply(
      db->default_cf_handle_->cfd(),
      *db->default_cf_handle_->cfd()->GetCurrentMutableCFOptions(), &edit,
      db->mutex(), db->directories_.GetDbDir());
  db->mutex()->Unlock();
  ROCKS_LOG_INFO(db->immutable_db_options_.info_log,
                 "Compaction installation completed: %s", s.ToString().c_str());
  _return = RpcUtils::ToTStatus(s);
}

RocksService::RocksService(DBImpl *_db) : db(_db) {
  // writeOptions.disableWAL = true;
}

void RocksService::Start() {
  // create a new RocksService to avoid deleting this one when server stops
  std::shared_ptr<ThriftServiceIf> interface(
      static_cast<ThriftServiceIf *>(new RocksService(db)));
  std::shared_ptr<ThriftServiceProcessor::TProcessor> processor(
      new ThriftServiceProcessor(interface));
  auto *node = db->immutable_db_options_.this_node;

  internal_server_transport.reset(new apache::thrift::transport::TServerSocket(
      node->getIp(), node->getPort()));
  external_server_transport.reset(new apache::thrift::transport::TServerSocket(
      node->getIp(), node->getPort() + 1));
  std::shared_ptr<apache::thrift::server::TTransportFactory> transportFactory(
      new apache::thrift::transport::TBufferedTransportFactory());
  std::shared_ptr<apache::thrift::protocol::TProtocolFactory> protocolFactory(
      new apache::thrift::protocol::TBinaryProtocolFactory());

  std::shared_ptr<apache::thrift::concurrency::ThreadManager>
      internal_thread_manager(
          apache::thrift::concurrency::ThreadManager::newSimpleThreadManager(
              CORE_NUM, 100));
  std::shared_ptr<apache::thrift::concurrency::ThreadManager>
      external_thread_manager(
          apache::thrift::concurrency::ThreadManager::newSimpleThreadManager(
              CORE_NUM, 100));
  std::shared_ptr<apache::thrift::concurrency::ThreadFactory> threadFactory(
      new apache::thrift::concurrency::ThreadFactory(false));
  internal_thread_manager->threadFactory(threadFactory);
  external_thread_manager->threadFactory(threadFactory);
  internal_thread_manager->start();
  external_thread_manager->start();

  internal_server.reset(new apache::thrift::server::TThreadPoolServer(
      processor, internal_server_transport, transportFactory, protocolFactory,
      internal_thread_manager));
  external_server.reset(new apache::thrift::server::TThreadPoolServer(
      processor, external_server_transport, transportFactory, protocolFactory,
      external_thread_manager));
  db->env_->Schedule(RocksService::RunServer, internal_server.get());
  db->env_->Schedule(RocksService::RunServer, external_server.get());
  ROCKS_LOG_INFO(db->immutable_db_options_.info_log,
                 "Compaction service "
                 "started");
}

void RocksService::RunServer(void *arg) {
  auto *server =
      reinterpret_cast<apache::thrift::server::TThreadPoolServer *>(arg);
  try {
    server->serve();
  } catch (apache::thrift::transport::TTransportException &exception) {
    // ignore
  } catch (std::exception &exception) {
    std::cerr << "Unexpected error in RocksService " << exception.what()
              << std::endl;
  } catch (...) {
    std::cerr << "Unexpected error in RocksService " << std::endl;
  }
}

void RocksService::Put(TStatus &_return, const std::string &key,
                       const std::string &value) {
  Slice _key(key);
  Slice _value(value);
  Status status =
      db->Put(writeOptions, db->DefaultColumnFamily(), _key, _value);
  _return = RpcUtils::ToTStatus(status);
}

void RocksService::Get(GetResult &_return, const std::string &key) {
  Slice _key(key);
  PinnableSlice _value;
  Status status =
      db->Get(readOptions, db->DefaultColumnFamily(), _key, &_value);
  if (status.ok()) {
    _return.value = _value.ToString();
  }
  _return.status = RpcUtils::ToTStatus(status);
}

RocksService::~RocksService() { Stop(); }

void RocksService::Stop() {
  if (internal_server != nullptr) {
    ROCKS_LOG_INFO(db->immutable_db_options().info_log, "Stopping RocksServer");
    internal_server_transport->close();
    external_server_transport->close();
    internal_server->stop();
    external_server->stop();
    internal_server.reset(nullptr);
    external_server.reset(nullptr);
    ROCKS_LOG_INFO(db->immutable_db_options().info_log, "Stopped RocksServer");
  }
}

void RocksService::FullCompaction(TStatus &_return) {
  _return =
      RpcUtils::ToTStatus(db->CompactRange(compactOptions, nullptr, nullptr));
  db->DumpStats();
}

void RocksService::Flush(TStatus &_return) {
  _return = RpcUtils::ToTStatus(db->Flush(flushOptions));
}

void RocksService::UpLoadTableFile(const std::string &file_name,
                                   const std::string &data, const bool is_last,
                                   int32_t path_num) {
  const std::string &local_file_name =
      db->immutable_db_options_.db_paths[path_num].path + "/" + file_name;
  std::unique_ptr<WritableFile> *writer = GetFileWriter(local_file_name);
  (*writer)->Append(Slice(data));
  if (is_last) {
    (*writer)->Close();
    ReleaseFileWriter(local_file_name);
    uint64_t file_size;
    db->env_->GetFileSize(local_file_name, &file_size);
    ROCKS_LOG_INFO(db->immutable_db_options_.info_log,
                   "File %s[%ld] is uploaded to this node, last data size: %ld",
                   local_file_name.c_str(), file_size, data.size());
  }
}

std::unique_ptr<WritableFile> *RocksService::GetFileWriter(
    const std::string &file_name) {
  std::unique_ptr<WritableFile> *writableFile;
  mutex.Lock();
  auto iter = file_writer_cache.find(file_name);
  if (iter == file_writer_cache.end()) {
    writableFile = new std::unique_ptr<WritableFile>;
    db->env_->NewWritableFile(file_name, writableFile, envOptions);
    file_writer_cache.emplace(file_name, writableFile);
  } else {
    writableFile = (*iter).second;
  }
  mutex.Unlock();
  return writableFile;
}

void RocksService::ReleaseFileWriter(const std::string &file_name) {
  mutex.Lock();
  file_writer_cache.erase(file_name);
  mutex.Unlock();
}

void RocksService::PutBatch(TStatus &_return,
                            const std::vector<std::string> &key,
                            const std::vector<std::string> &value) {
  WriteBatch batch;
  for (uint32_t i = 0; i < key.size(); ++i) {
    batch.Put(key[i], value[i]);
  }
  Status s = db->Write(writeOptions, &batch);
  _return = RpcUtils::ToTStatus(s);
}
}  // namespace ROCKSDB_NAMESPACE
