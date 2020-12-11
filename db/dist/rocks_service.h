#pragma once
#ifndef ROCKSDB_ROCKS_SERVICE_H
#define ROCKSDB_ROCKS_SERVICE_H

#include <db/db_impl/db_impl.h>

#include "db/thrift/gen/ThriftService.h"
#include "rocksdb/db.h"
#include "rocksdb/rocksdb_namespace.h"

namespace ROCKSDB_NAMESPACE {
class RocksService : ThriftServiceIf {
  void CompactFiles(TCompactionResult& _return,
                    const TCompactFilesRequest& request) override;
  void DownLoadFile(std::string& _return, const std::string& file_name,
                    int64_t offset, const int32_t size) override;
  void PushFiles(const TCompactionResult& result,
                 const std::string& source_ip,
                 int32_t source_port) override;
  void SetFileNumber(const int64_t new_file_num) override;
  void InstallCompaction(TStatus& _return,
                         const TInstallCompactionRequest& request) override;
  void Put(TStatus& _return, const std::string& key,
           const std::string& value) override;
  void Get(GetResult& _return, const std::string& key) override;

  DBImpl* db;
  WriteOptions writeOptions;
  ReadOptions readOptions;

 public:
  explicit RocksService(DBImpl* db);

  void Start();

  static void RunServer(void* arg);
};
}

#endif  // ROCKSDB_ROCKS_SERVICE_H
