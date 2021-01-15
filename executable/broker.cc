//
// Created by jt on 12/18/20.
//

#include "broker.h"

#include <atomic>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <sstream>

#include "db/thrift/gen/rpc_types.h"
#include "rocksdb/db.h"
#include "rocksdb/status.h"

using namespace ROCKSDB_NAMESPACE;

void Broker::Put(std::string& key, std::string& value) {
  TStatus status;
  for (unsigned int i = 0; i < clients.size(); ++i) {
    if (i == leader_pos) {
      continue;
    }
    clients[i]->Put(status, key, value);
    if (status.code != Status::Code::kOk) {
      std::cout << "An error occurred during put " << status.code << ":"
                << status.state << std::endl;
    }
  }
  leader_client->Put(status, key, value);
  if (status.code != Status::Code::kOk) {
    std::cout << "An error occurred during put " << status.code << ":"
              << status.state << std::endl;
  }
}

void Broker::Get(std::string& key, std::string& value) { Get(key, value, 0); }

void Broker::Get(std::string& key, std::string& value, uint32_t client_idx) {
  GetResult result;
  clients[client_idx]->Get(result, key);
  if (result.status.code != Status::Code::kOk) {
    std::cout << "An error occurred during get " << result.status.code << ":"
              << result.status.state << std::endl;
  } else {
    value = result.value;
  }
}

void Broker::Flush() {
  TStatus status;
  for (unsigned int i = 0; i < clients.size(); ++i) {
    if (i == leader_pos) {
      continue;
    }
    clients[i]->Flush(status);
    if (status.code != Status::Code::kOk) {
      std::cout << "An error occurred during flush " << status.code << ":"
                << status.state << std::endl;
    }
  }
  leader_client->Flush(status);
  if (status.code != Status::Code::kOk) {
    std::cout << "An error occurred during flush " << status.code << ":"
              << status.state << std::endl;
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

void ParseNodes(std::string& nodes_str, std::vector<ClusterNode*>& nodes) {
  std::vector<std::string> splits;
  boost::split(splits, nodes_str, boost::is_any_of(","));
  std::cout << "Read " << splits.size() << " nodes" << std::endl;
  for (auto& split : splits) {
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

  std::string compaction_leader_str =
      root_node.get<std::string>("compaction_leader");
  std::string all_nodes_str = root_node.get<std::string>("all_nodes");

  compaction_leader = new ClusterNode();
  ParseNode(compaction_leader_str, compaction_leader);
  ParseNodes(all_nodes_str, nodes);

  leader_client = RpcUtils::CreateClient(compaction_leader);
  for (uint32_t i = 0; i < nodes.size(); i++) {
    auto* node = nodes[i];
    if (*node == *compaction_leader) {
      leader_pos = i;
    }
    clients.push_back(RpcUtils::CreateClient(node));
  }
}
Broker::Broker(char* config_file_path) { init(config_file_path); }
Broker::~Broker() {
  if (leader_client != nullptr) {
    delete leader_client;
    leader_client = nullptr;
  }
  for (auto* client : clients) {
    client->getInputProtocol()->getTransport()->close();
    delete client;
  }
  clients.clear();
}
uint32_t Broker::ClientNum() { return nodes.size(); }

void simple_test(int argc, char** argv) {
  std::cout << "Broker started" << std::endl;

  Broker* broker;
  if (argc > 1) {
    broker = new Broker(argv[1]);
  } else {
    std::cerr << "Please provide the configuration path" << std::endl;
    return;
  }
  std::string key;
  std::string value;

  for (int i = 0; i < 9; ++i) {
    key = std::to_string(i);
    value = std::to_string(i);
    broker->Put(key, value);
  }
  broker->Flush();
  broker->Compact();

  std::cout << "Before overwriting" << std::endl;
  for (int i = 0; i < 9; ++i) {
    key = std::to_string(i);
    broker->Get(key, value);
    std::cout << key << " " << value << std::endl;
  }

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      key = std::to_string(i * 3 + j);
      value = std::to_string(i * 3 + j + 100);
      broker->Put(key, value);
    }
    broker->Flush();
  }

  std::cout << "Before compaction" << std::endl;
  for (int i = 0; i < 9; ++i) {
    key = std::to_string(i);
    broker->Get(key, value);
    std::cout << key << " " << value << std::endl;
  }

  broker->Compact();

  std::cout << "After compaction" << std::endl;
  for (uint32_t k = 0; k < broker->ClientNum(); ++k) {
    std::cout << "client " << k << std::endl;
    for (int i = 0; i < 9; ++i) {
      key = std::to_string(i);
      broker->Get(key, value, k);
      std::cout << key << " " << value << std::endl;
    }
  }

  delete broker;
}

void write_stress(int argc, char** argv) {
  if (argc <= 1) {
    std::cerr << "Please provide the configuration path" << std::endl;
    return;
  }

  std::atomic_int i(0);

  std::vector<std::thread> threads;
  clock_t t_start = clock();

  for (int k = 0; k < 9; ++k) {
    threads.emplace_back([&i, &argv, &t_start] {
      Broker* b = new Broker(argv[1]);
      int rand_max = 1000000;

      std::string key_;
      std::string value_;
      char buf[256];
      clock_t t;
      for (;;) {
        uint32_t j = ++i;
        uint32_t k_v = rand() % rand_max;
        key_ = std::to_string(k_v);
        value_ = std::to_string(k_v);
        b->Put(key_, value_);
        if (j % 10000 == 0) {
          t = clock();
          double avg = (double) j / (t - t_start) * CLOCKS_PER_SEC;
          t /= CLOCKS_PER_SEC;
          strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
          std::cout << buf << " " << j << " " << avg << std::endl;
        }
      }
    });
  }

  for (auto& th : threads) {
    th.join();
  }
}

int main(int argc, char** argv) {
//   simple_test(argc, argv);

  write_stress(argc, argv);
  return 0;
}