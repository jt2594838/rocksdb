#pragma once

#ifndef ROCKSDB_RPCUTILS_H
#define ROCKSDB_RPCUTILS_H

#include "rocksdb/node.h"
#include "db/thrift/gen/ThriftService.h"
#include "rocksdb/rocksdb_namespace.h"
#include "db/version_edit.h"

namespace ROCKSDB_NAMESPACE {
class RpcUtils {
  static port::Mutex mutex;
  static std::map<ClusterNode*, std::vector<ThriftServiceClient*>*> client_cache;

 public:
  static ThriftServiceClient* GetClient(ClusterNode* node);
  static void ReleaseClient(ClusterNode* node, ThriftServiceClient* client);
  static ThriftServiceClient* NewClient(ClusterNode* node);
  static uint64_t DownloadFile(std::string& file_name, ClusterNode* node,
                               const std::string& output_name,
                               const std::shared_ptr<Logger>& ptr);
  static FileMetaData ToFileMetaData(const TFileMetadata& tmetadata);
  static FileDescriptor ToFilDescriptor(const TFileDescriptor& tdesceiptor);
  static TFileMetadata ToTFileMetaData(const FileMetaData& metaData);
  static TFileDescriptor ToTFileDescriptor(const FileDescriptor&
                                               fileDescriptor);
  static Status ToStatus(TStatus& status);
  static TStatus ToTStatus(const Status& status);

  static uint64_t ToUint64(const std::string& str);
};
}  // namespace rocksT

#endif  // ROCKSDB_RPCUTILS_H
