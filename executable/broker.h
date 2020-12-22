//
// Created by jt on 12/18/20.
//

#pragma once
#ifndef ROCKSDB_BROKER_H
#define ROCKSDB_BROKER_H

#include <vector>

#include "db/thrift/RpcUtils.h"
#include "rocksdb/node.h"
#include "rocksdb/slice.h"
#include "db/thrift/gen/ThriftService.h"

using namespace ROCKSDB_NAMESPACE;

class Broker {
  std::vector<ClusterNode*> nodes;
  std::vector<ThriftServiceClient*> clients;
  ClusterNode* compaction_leader = nullptr;
  ThriftServiceClient* leader_client = nullptr;

  void init(char* config_file_path);

 public:
  explicit Broker(char* config_file_path);
  ~Broker();
  void Put(std::string& key, std::string& value);
  void Get(std::string& key, std::string& value);
  void Flush();
  void Compact();
};

#endif  // ROCKSDB_BROKER_H
