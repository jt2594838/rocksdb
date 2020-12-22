//
// Created by jt on 12/18/20.
//

#include "broker.h"

#include "rocksdb/db.h"
#include "rocksdb/status.h"
#include "db/thrift/gen/rpc_types.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <sstream>

using namespace ROCKSDB_NAMESPACE;

void Broker::Put(std::string& key, std::string& value) {
  TStatus status;
  for (unsigned int i = 0; i < clients.size(); ++i) {
    clients[i]->Put(status, key, value);
    if (status.code != Status::Code::kOk) {
      std::cout << "An error occurred during put " << status.code << ":" << status.state << std::endl;
    }
  }
}
void Broker::Get(std::string& key, std::string& value) {
  GetResult result;
  clients[0]->Get(result, key);
  if (result.status.code != Status::Code::kOk) {
    std::cout << "An error occurred during get " << result.status.code << ":" << result.status.state << std::endl;
  } else {
    value = result.value;
  }
}
void Broker::Flush() {
  TStatus status;
  for (unsigned int i = 0; i < clients.size(); ++i) {
    clients[i]->Flush(status);
    if (status.code != Status::Code::kOk) {
      std::cout << "An error occurred during flush " << status.code << ":" << status.state << std::endl;
    }
  }
}
void Broker::Compact() {
  TStatus status;
  leader_client->FullCompaction(status);
}

void ParseNode(std::string& node_str, ClusterNode* node) {
  std::vector<std::string> splits;
  boost::split(splits, node_str, boost::is_any_of(":"));
  node->setIp(splits[0]);
  node->setPort(atoi(splits[1].c_str()));
}

void ParseNodes(std::string& nodes_str, std::vector<ClusterNode*> &nodes) {
  std::vector<std::string> splits;
  boost::split(splits, nodes_str, boost::is_any_of(","));
  std::cout << "Read " << splits.size() << " nodes" << std::endl;
  for (auto & split : splits) {
    auto* node = new ClusterNode();
    ParseNode(split, node);
    nodes.push_back(node);
  }
}

void Broker::init(char* config_file_path) {
  if (!boost::filesystem::exists(config_file_path)) {
    std::cout << config_file_path << " not exists." << std::endl;
    exit(-1);
  }

  boost::property_tree::ptree root_node;
  boost::property_tree::ini_parser::read_ini(config_file_path, root_node);

  std::string compaction_leader_str = root_node.get<std::string>("compaction_leader");
  std::string all_nodes_str = root_node.get<std::string>("all_nodes");

  compaction_leader = new ClusterNode();
  ParseNode(compaction_leader_str, compaction_leader);
  ParseNodes(all_nodes_str, nodes);

  leader_client = RpcUtils::CreateClient(compaction_leader);
  for (auto* node : nodes) {
    clients.push_back(RpcUtils::CreateClient(node));
  }
}
Broker::Broker(char* config_file_path) {
  init(config_file_path);
}
Broker::~Broker() {
  if (leader_client != nullptr) {
    delete leader_client;
    leader_client = nullptr;
  }
  for (auto* client : clients) {
    delete client;
  }
  clients.clear();
}

int main(int argc, char** argv) {
  std::cout << "Broker started" << std::endl;

  Broker* broker;
  if (argc > 1) {
    broker = new Broker(argv[1]);
  } else {
    std::cerr << "Please provide the configuration path" << std::endl;
    return -1;
  }
  std::string key;
  std::string value;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      key = std::to_string(i * 3 + j);
      value = std::to_string(i * 3 + j);
      broker->Put(key, value);
    }
    broker->Flush();
  }
  for (int i = 0; i < 9; ++i) {
    key = std::to_string(i);
    value = std::to_string(i);
    broker->Put(key, value);
  }
  broker->Flush();
  broker->Compact();

  delete broker;
  return 0;
}