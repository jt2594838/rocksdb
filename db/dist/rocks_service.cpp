#include "rocks_service.h"

#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <iostream>

#include "db/thrift/RpcUtils.h"
#include "db/version_edit.h"
#include "file/filename.h"

namespace ROCKSDB_NAMESPACE {
void RocksService::CompactFiles(TCompactionResult& _return,
                                const TCompactFilesRequest& request) {
  CompactionOptions compactionOptions;
  std::vector<std::string> file_names;
  for (auto num : request.file_nums) {
    auto packed_file_num = static_cast<uint64_t>(num);
    uint64_t file_num = packed_file_num & kFileNumberMask;
    uint64_t path_id =
        static_cast<uint32_t>(packed_file_num / (kFileNumberMask + 1));
    file_names.emplace_back(
        TableFileName(db->GetDBOptions().db_paths, path_id, file_num));
  }
  ROCKS_LOG_INFO(db->immutable_db_options_.info_log,
                 "Received a compaction "
                 "request of %ld files, range: [%s, %s]",
                 file_names.size(), request.comp_start.c_str(),
                 request.comp_end.c_str());
  Slice begin(request.comp_start);
  Slice end(request.comp_end);
  std::vector<std::string> output_file_names;
  CompactionJobInfo jobInfo;
  Status status = db->CompactExactly(
      compactionOptions, db->DefaultColumnFamily(), file_names,
      request.output_level, request.start_file_num, request.max_file_num,
      &begin, &end, -1, &output_file_names, &jobInfo);
  for (auto meta : jobInfo.output_file_meta) {
    _return.output_files.emplace_back(RpcUtils::ToTFileMetaData(meta));
  }
  _return.status.code = status.code();
  _return.status.sub_code = status.subcode();
  _return.status.state = std::string(status.state_);
  _return.status.severity = status.severity();
  _return.total_bytes = jobInfo.stats.total_output_bytes;
  _return.num_output_records = jobInfo.stats.num_output_records;
  ROCKS_LOG_INFO(db->immutable_db_options_.info_log,
                 "Compaction request "
                 "executed with result: "
                 "%s, and %ld files",
                 status.ToString().c_str(), _return.output_files.size());
}
void RocksService::DownLoadFile(std::string& _return,
                                const std::string& file_name,
                                const int64_t offset, const int32_t size) {
  std::unique_ptr<RandomAccessFile> raf;
  EnvOptions envOptions;
  Status s = db->env_->NewRandomAccessFile(file_name, &raf, envOptions);
  if (s.ok()) {
    Slice result;
    char* data = new char[size];
    raf->Read(offset, size, &result, data);
    _return = std::string(data, result.size());
  }
}

void RocksService::PushFiles(const TCompactionResult& result,
                             const std::string& source_ip,
                             const int32_t source_port) {
  auto source_node = ClusterNode(source_ip, source_port);
  for (auto meta : result.output_files) {
    FileMetaData& file_metadata = RpcUtils::ToFileMetaData(meta);
    uint64_t file_num = file_metadata.fd.GetNumber();
    uint64_t path_num = file_metadata.fd.GetPathId();
    const std::string& cf_path =
        db->default_cf_handle_->cfd()->ioptions()->cf_paths[path_num].path;
    std::string source_file_path =
        TableFileName(db->immutable_db_options_.db_paths, file_num, path_num);
    ROCKS_LOG_INFO(db->immutable_db_options_.info_log, "Downloading file %s",
                   source_file_path.c_str());
    const std::string& target_file_name =
        Env::TransFormToLocalFile(source_file_path, cf_path);
    RpcUtils::DownloadFile(source_file_path, &source_node, target_file_name);
    ROCKS_LOG_INFO(db->immutable_db_options_.info_log,
                   "File %s is downloaded at %s", source_file_path.c_str(),
                   target_file_name.c_str());
  }
}

void RocksService::SetFileNumber(const int64_t new_file_num) {
  db->mutex()->Lock();
  if (db->versions_->GetFileNumber() < static_cast<uint64_t>(new_file_num)) {
    db->versions_->SetFileNumber(new_file_num);
    ROCKS_LOG_INFO(db->immutable_db_options_.info_log,
                   "New file number is "
                   "updated to %lld",
                   new_file_num);
  }
  db->mutex()->Unlock();
}

void RocksService::InstallCompaction(TStatus& _return,
                                     const TInstallCompactionRequest& request) {
  VersionEdit edit;
  for (auto& deleted_file : request.deleted_inputs) {
    int level = deleted_file.level;
    uint64_t file_num = deleted_file.file_num;
    edit.DeleteFile(level, file_num);
  }

  for (auto& installed_file : request.installed_outputs) {
    int level = installed_file.level;
    FileMetaData& metaData = RpcUtils::ToFileMetaData(installed_file.metadata);
    edit.AddFile(level, metaData);
  }
  ROCKS_LOG_INFO(db->immutable_db_options_.info_log,
                 "Received an "
                 "compaction installation "
                 "request of %ld inputs, %ld outputs",
                 request.deleted_inputs.size(),
                 request.installed_outputs.size());
  edit.SetColumnFamily(db->DefaultColumnFamily()->GetID());
  db->mutex()->Lock();
  Status s = db->versions_->LogAndApply(
      db->default_cf_handle_->cfd(),
      *db->default_cf_handle_->cfd()->GetCurrentMutableCFOptions(), &edit,
      db->mutex(), db->directories_.GetDbDir());
  db->mutex()->Unlock();
  ROCKS_LOG_INFO(db->immutable_db_options_.info_log,
                 "Compaction installation"
                 " completed");
  _return = RpcUtils::ToTStatus(s);
}
RocksService::RocksService(DBImpl* db) : db(db) {}
void RocksService::Start() {
  std::shared_ptr<ThriftServiceIf> interface(
      static_cast<ThriftServiceIf*>(this));
  std::shared_ptr<ThriftServiceProcessor::TProcessor> processor(
      new ThriftServiceProcessor(interface));
  auto* node = db->immutable_db_options_.this_node;
  std::shared_ptr<apache::thrift::server::TServerTransport> serverTransport(
      new apache::thrift::transport::TServerSocket(node->getIp(),
                                                   node->getPort()));
  std::shared_ptr<apache::thrift::server::TTransportFactory> transportFactory(
      new apache::thrift::transport::TBufferedTransportFactory());
  std::shared_ptr<apache::thrift::protocol::TProtocolFactory> protocolFactory(
      new apache::thrift::protocol::TBinaryProtocolFactory());

  auto* server = new apache::thrift::server::TThreadPoolServer(
      processor, serverTransport, transportFactory, protocolFactory);
  db->env_->Schedule(RocksService::RunServer, server);
  ROCKS_LOG_INFO(db->immutable_db_options_.info_log,
                 "Compaction service "
                 "started");
}

void RocksService::RunServer(void* arg) {
  auto* server =
      reinterpret_cast<apache::thrift::server::TThreadPoolServer*>(arg);
  server->serve();
}
void RocksService::Put(TStatus& _return, const std::string& key,
                       const std::string& value) {
  Slice _key(key);
  Slice _value(value);
  Status status = db->Put(writeOptions, _key, _value);
  _return = RpcUtils::ToTStatus(status);
}
void RocksService::Get(GetResult& _return, const std::string& key) {
  Slice _key(key);
  Status status = db->Get(readOptions, _key, &_return.value);
  _return.status = RpcUtils::ToTStatus(status);
}
}  // namespace ROCKSDB_NAMESPACE
