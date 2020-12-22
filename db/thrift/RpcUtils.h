#pragma once

#ifndef ROCKSDB_RPCUTILS_H
#define ROCKSDB_RPCUTILS_H

#include "rocksdb/node.h"
#include "db/thrift/gen/ThriftService.h"
#include "rocksdb/rocksdb_namespace.h"
#include "db/version_edit.h"

namespace ROCKSDB_NAMESPACE {
class RpcUtils {
 public:
  static ThriftServiceClient* CreateClient(
      ROCKSDB_NAMESPACE::ClusterNode* node);
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
};
}  // namespace rocksT

#endif  // ROCKSDB_RPCUTILS_H
