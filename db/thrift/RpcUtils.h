

#ifndef ROCKSDB_RPCUTILS_H
#define ROCKSDB_RPCUTILS_H

#include "rocksdb/node.h"
#include "db/thrift/gen/ThriftService.h"
#include "rocksdb/rocksdb_namespace.h"

namespace ROCKSDB_NAMESPACE {
class RpcUtils {
 public:
  static ThriftServiceClient* CreateClient(
      ROCKSDB_NAMESPACE::ClusterNode* node);
  static uint64_t DownloadFile(std::string& file_name,
                               ROCKSDB_NAMESPACE::ClusterNode* node,
                               const std::string& output_name);
  static FileMetaData& ToFileMetaData(const TFileMetadata& tmetadata);
  static FileDescriptor ToFilDescriptor(const TFileDescriptor& tdesceiptor);
  static TFileMetadata ToTFileMetaData(const FileMetaData& metaData);
  static TFileDescriptor ToTFileDescriptor(const FileDescriptor&
                                               fileDescriptor);
  static Status ToStatus(const TStatus& status);
  static TStatus ToTStatus(const Status& status);
};
}  // namespace rocksT

#endif  // ROCKSDB_RPCUTILS_H
