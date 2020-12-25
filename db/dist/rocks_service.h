#pragma once
#ifndef ROCKSDB_ROCKS_SERVICE_H
#define ROCKSDB_ROCKS_SERVICE_H

#include <db/db_impl/db_impl.h>
#include <thrift/server/TThreadPoolServer.h>

#include "db/thrift/gen/ThriftService.h"
#include "rocksdb/db.h"
#include "rocksdb/rocksdb_namespace.h"

namespace ROCKSDB_NAMESPACE {
class RocksService : ThriftServiceIf {
  void CompactFiles(TCompactionResult& _return,
                    const TCompactFilesRequest& request) override;
  void DownLoadFile(std::string& _return, const std::string& file_name,
                    int64_t offset, int32_t size) override;
  void PushFiles(const TCompactionResult& result,
                 const std::string& source_ip,
                 int32_t source_port) override;
  void SetCompactionNumber(int64_t new_file_num) override;
  void InstallCompaction(TStatus& _return,
                         const TInstallCompactionRequest& request) override;
  void Put(TStatus& _return, const std::string& key,
           const std::string& value) override;
  void Get(GetResult& _return, const std::string& key) override;

  void FullCompaction(TStatus& _return) override;
  void Flush(TStatus& _return) override;

  DBImpl* db;
  WriteOptions writeOptions;
  ReadOptions readOptions;
  std::unique_ptr<apache::thrift::server::TThreadPoolServer> server = nullptr;
  std::shared_ptr<apache::thrift::server::TServerTransport> serverTransport;
  FlushOptions flushOptions;
  CompactRangeOptions compactOptions;

 public:
  explicit RocksService(DBImpl* db);

  void Start();
  void Stop();

  static void RunServer(void* arg);
  ~RocksService() override;
};
}

#endif  // ROCKSDB_ROCKS_SERVICE_H
