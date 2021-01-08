
#include "RpcUtils.h"

#include <db/version_edit.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransport.h>

#include <iostream>

#include "rocksdb/rocksdb_namespace.h"

namespace ROCKSDB_NAMESPACE {
ThriftServiceClient* RpcUtils::CreateClient(ClusterNode* node) {
  std::shared_ptr<apache::thrift::transport::TTransport> transport;
  transport.reset(
      new apache::thrift::transport::TSocket(node->getIp(), node->getPort()));
  std::shared_ptr<apache::thrift::transport::TTransport> buffered_transport(
      new apache::thrift::transport::TBufferedTransport(transport));
  std::shared_ptr< ::apache::thrift::protocol::TProtocol> protocol;
  protocol.reset(
      new ::apache::thrift::protocol::TBinaryProtocol(buffered_transport));
  auto* client = new ThriftServiceClient(protocol);
  try {
    transport->open();
    return client;
  } catch (apache::thrift::transport::TTransportException& e) {
    return nullptr;
  }
}

uint64_t RpcUtils::DownloadFile(std::string& file_name, ClusterNode* node,
                                const std::string& output_name,
                                const std::shared_ptr<Logger>& logger) {
  uint64_t file_length = 0;
  const uint64_t block_length = 4 * 1024 * 1024;
  auto* client = CreateClient(node);
  if (client == nullptr) {
    ROCKS_LOG_ERROR(logger, "Node %s is unreachable", node->ToString().c_str());
    return -1;
  }

  std::string buffer;
  std::ofstream output_stream(output_name);

  while (true) {
    client->DownLoadFile(buffer, file_name, file_length, block_length);
    if (buffer.empty()) {
      break;
    }
    file_length += buffer.size();
    output_stream << buffer;
  }
  output_stream.close();
  delete client;
  return file_length;
}

FileMetaData RpcUtils::ToFileMetaData(const TFileMetadata& tmetadata) {
  FileMetaData metaData;
  metaData.fd = ToFilDescriptor(tmetadata.fd);
  metaData.smallest.DecodeFrom(Slice(tmetadata.smallest_key));
  metaData.largest.DecodeFrom(Slice(tmetadata.largest_key));
  metaData.num_entries = tmetadata.num_entries;
  metaData.num_deletions = tmetadata.num_deletions;
  metaData.raw_key_size = tmetadata.raw_key_size;
  metaData.raw_value_size = tmetadata.raw_value_size;
  metaData.oldest_ancester_time = tmetadata.oldest_ancester_time;
  metaData.file_creation_time = tmetadata.file_creation_time;
  metaData.file_checksum = tmetadata.file_checksum;
  metaData.file_checksum_func_name = tmetadata.file_checksum_func_name;
  return metaData;
}

FileDescriptor RpcUtils::ToFilDescriptor(const TFileDescriptor& tdesceiptor) {
  FileDescriptor descriptor(tdesceiptor.flush_number, tdesceiptor.merge_number,
                            tdesceiptor.path_id, tdesceiptor.file_size,
                            tdesceiptor.smallest_seqno,
                            tdesceiptor.largest_seqno);
  return descriptor;
}

TFileMetadata RpcUtils::ToTFileMetaData(const FileMetaData& metaData) {
  TFileMetadata tmetadata;
  tmetadata.fd = ToTFileDescriptor(metaData.fd);
  tmetadata.smallest_key = metaData.smallest.Encode().ToString();
  tmetadata.largest_key = metaData.largest.Encode().ToString();
  tmetadata.num_entries = metaData.num_entries;
  tmetadata.num_deletions = metaData.num_deletions;
  tmetadata.raw_key_size = metaData.raw_key_size;
  tmetadata.raw_value_size = metaData.raw_value_size;
  tmetadata.oldest_ancester_time = metaData.oldest_ancester_time;
  tmetadata.file_creation_time = metaData.file_creation_time;
  tmetadata.file_checksum = metaData.file_checksum;
  tmetadata.file_checksum_func_name = metaData.file_checksum_func_name;
  return tmetadata;
}

TFileDescriptor RpcUtils::ToTFileDescriptor(const FileDescriptor& descriptor) {
  TFileDescriptor tdescriptor;
  tdescriptor.flush_number = descriptor.flush_number;
  tdescriptor.merge_number = descriptor.merge_number;
  tdescriptor.largest_seqno = descriptor.largest_seqno;
  tdescriptor.smallest_seqno = descriptor.smallest_seqno;
  tdescriptor.file_size = descriptor.file_size;
  return tdescriptor;
}

Status RpcUtils::ToStatus(TStatus& status) {
  Status s;
  s.code_ = static_cast<Status::Code>(status.code);
  s.subcode_ = static_cast<Status::SubCode>(status.sub_code);
  s.sev_ = static_cast<Status::Severity>(status.severity);
  int state_size = status.state.size();
  s.state_ =
      strncpy(new char[state_size + 1], status.state.c_str(), state_size);
  return s;
}

TStatus RpcUtils::ToTStatus(const Status& status) {
  TStatus s;
  s.code = status.code();
  s.sub_code = status.subcode_;
  s.severity = status.severity();
  s.state = std::string(status.state_ ? status.state_ : "");
  return s;
}
}  // namespace ROCKSDB_NAMESPACE
