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
#include <random>
#include <sstream>
#include <thread>

#include "db/thrift/gen/rpc_types.h"
#include "rocksdb/db.h"
#include "rocksdb/env.h"
#include "rocksdb/status.h"

using namespace ROCKSDB_NAMESPACE;

void Broker::Put(std::vector<std::string>& keys,
                 std::vector<std::string>& values) {
  TStatus status;
  for (unsigned int i = 0; i < clients.size(); ++i) {
    if (i == leader_pos) {
      continue;
    }
    uint64_t t_start = clock();
    clients[i]->PutBatch(status, keys, values);
    uint64_t t_consumed = clock() - t_start;
    client_ticks[i] += t_consumed;
    if (status.code != Status::Code::kOk) {
      std::cout << "An error occurred during put " << status.code << ":"
                << status.state << std::endl;
    }
  }

  uint64_t t_start = clock();
  leader_client->PutBatch(status, keys, values);
  uint64_t t_consumed = clock() - t_start;
  client_ticks[leader_pos] += t_consumed;
  if (status.code != Status::Code::kOk) {
    std::cout << "An error occurred during put " << status.code << ":"
              << status.state << std::endl;
  }
}

void Broker::Put(std::string& key, std::string& value) {
  TStatus status;
  for (unsigned int i = 0; i < clients.size(); ++i) {
    if (i == leader_pos) {
      continue;
    }
    uint64_t t_start = clock();
    clients[i]->Put(status, key, value);
    uint64_t t_consumed = clock() - t_start;
    client_ticks[i] += t_consumed;
    if (status.code != Status::Code::kOk) {
      std::cout << "An error occurred during put " << status.code << ":"
                << status.state << std::endl;
    }
  }

  uint64_t t_start = clock();
  leader_client->Put(status, key, value);
  uint64_t t_consumed = clock() - t_start;
  client_ticks[leader_pos] += t_consumed;
  if (status.code != Status::Code::kOk) {
    std::cout << "An error occurred during put " << status.code << ":"
              << status.state << std::endl;
  }
}

void Broker::Get(std::string& key, std::string& value) { Get(key, value, 0); }

void Broker::Get(std::string& key, std::string& value, uint32_t client_idx) {
  GetResult result;
  clients[client_idx]->Get(result, key);
  if (result.status.code != Status::Code::kOk &&
      result.status.code != Status::Code::kNotFound) {
    std::cout << "An error occurred during get " << result.status.code << ":"
              << result.status.state << std::endl;
  } else {
    value = result.status.code == Status::Code::kOk ? result.value : "NULL";
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

  leader_client = RpcUtils::NewClient(compaction_leader);
  client_ticks = new uint64_t[nodes.size()];
  for (uint32_t i = 0; i < nodes.size(); i++) {
    auto* node = nodes[i];
    if (*node == *compaction_leader) {
      leader_pos = i;
    }
    clients.push_back(RpcUtils::NewClient(node));
    client_ticks[i] = 0;
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
std::string Broker::GetTicks() {
  std::string str;
  for (uint32_t i = 0; i < nodes.size(); ++i) {
    str.append(std::to_string(client_ticks[i]));
    str.append(" ");
  }
  return str;
}
void Broker::ClearTicks() {
  for (uint32_t i = 0; i < nodes.size(); ++i) {
    client_ticks[i] = 0;
  }
}
void Broker::CompactEach() {
  std::vector<std::thread> threads;
  for (unsigned int i = 0; i < clients.size(); ++i) {
    Broker* b = this;
    threads.emplace_back([i, &b] {
      TStatus status;
      b->clients[i]->FullCompaction(status);
    });
  }
  for (auto& t : threads) {
    t.join();
  }
}

void simple_test(int argc, char** argv) {
  std::cout << "Broker started" << std::endl;

  uint32_t file_num = 3;
  uint32_t pt_per_file = 8000000;
  bool compact_each = true;

  Broker* broker;
  if (argc > 1) {
    broker = new Broker(argv[1]);
  } else {
    std::cerr << "Please provide the configuration path" << std::endl;
    return;
  }

  std::string key_;
  std::string value_;
  std::vector<std::string> keys_;
  std::vector<std::string> values_;
  uint32_t batch_size = 5000;
  keys_.reserve(batch_size);
  values_.reserve(batch_size);

  for (uint32_t i = 0; i < file_num * pt_per_file; ++i) {
    key_ = std::to_string(i);
    value_ = std::to_string(i);

    keys_.emplace_back(key_);
    values_.emplace_back(value_);

    if ((i + 1) % batch_size == 0) {
      broker->Put(keys_, values_);
      keys_.clear();
      values_.clear();
    }
    if ((i + 1) % (pt_per_file / 20) == 0) {
      std::cout << (i + 1) << "/" << file_num * pt_per_file << std::endl;
    }
  }
  broker->Flush();
  broker->Compact();

  std::cout << "Before overwriting" << std::endl;
  //  for (int i = 0; i < 9; ++i) {
  //    key = std::to_string(i);
  //    broker->Get(key, value);
  //    std::cout << key << " " << value << std::endl;
  //  }

  for (uint32_t i = 0; i < file_num; ++i) {
    for (uint32_t j = 0; j < pt_per_file; ++j) {
      key_ = std::to_string(i * pt_per_file + j);
      value_ = std::to_string(i * pt_per_file + j + 100);

      keys_.emplace_back(key_);
      values_.emplace_back(value_);

      if ((i * pt_per_file + j + 1) % batch_size == 0) {
        broker->Put(keys_, values_);
        keys_.clear();
        values_.clear();
      }
    }
    broker->Flush();
  }

  std::cout << "Before compaction" << std::endl;
  //  for (int i = 0; i < 9; ++i) {
  //    key = std::to_string(i);
  //    broker->Get(key, value);
  //    std::cout << key << " " << value << std::endl;
  //  }

  if (compact_each) {
    broker->CompactEach();
  } else {
    broker->Compact();
  }

  std::cout << "After compaction" << std::endl;
  //  for (uint32_t k = 0; k < broker->ClientNum(); ++k) {
  //    std::cout << "client " << k << std::endl;
  //    for (int i = 0; i < 9; ++i) {
  //      key = std::to_string(i);
  //      broker->Get(key, value, k);
  //      std::cout << key << " " << value << std::endl;
  //    }
  //  }

  delete broker;
}

void write_stress(int argc, char** argv) {
  if (argc <= 1) {
    std::cerr << "Please provide the configuration path" << std::endl;
    return;
  }

  Env* env = Env::Default();

  bool compactEach = false;
  uint32_t batch_size = 1000;
  uint32_t batch_num = 100000;
  uint32_t batch_report_interval = 1000;
  uint32_t read_num_per_batch = 10;
  uint64_t seed = 21516347;
  std::atomic_int i(0);

  std::vector<std::thread> threads;
  uint64_t t_start = env->NowMicros();
  uint64_t t_last = t_start;

  for (int k = 0; k < 9; ++k) {
    threads.emplace_back([&env, &i, &argv, t_start, &t_last, batch_size,
                          batch_num, batch_report_interval, seed,
                          read_num_per_batch] {
      Broker b(argv[1]);
      std::default_random_engine e(seed);
      std::uniform_int_distribution<uint32_t> distribution;

      std::vector<std::string> keys_;
      std::vector<std::string> values_;
      keys_.reserve(batch_size);
      values_.reserve(batch_size);
      uint64_t t;
      for (;;) {
        for (uint32_t l = 0; l < batch_size; l++) {
          uint32_t k_v = distribution(e);
          keys_.emplace_back(std::to_string(k_v));
          values_.emplace_back(std::to_string(k_v));
        }
        uint32_t j = ++i;
        b.Put(keys_, values_);
        keys_.clear();
        values_.clear();

        for (uint32_t r = 0; r < read_num_per_batch; r++) {
          uint32_t key = distribution(e);
          std::string key_string = std::to_string(key);
          std::string value;
          b.Get(key_string, value, r % b.ClientNum());
        }

        if (j % batch_report_interval == 0) {
          t = env->NowMicros();
          double temp_avg = (double)batch_report_interval / (t - t_last) *
                            CLOCKS_PER_SEC * batch_size;
          t_last = t;
          double total_avg =
              (double)j / (t - t_start) * CLOCKS_PER_SEC * batch_size;
          double total_consumed_time = (double)(t - t_start) / CLOCKS_PER_SEC;
          std::cout << total_consumed_time << " " << j * batch_size << " "
                    << total_avg << " " << temp_avg << "|" << b.GetTicks()
                    << std::endl;
          // b->ClearTicks();
        }
        if (j >= batch_num) {
          break;
        }
      }
    });
  }

  for (auto& th : threads) {
    th.join();
  }
  Broker b(argv[1]);
  b.Flush();
  if (compactEach) {
    b.CompactEach();
  } else {
    b.Compact();
  }

  uint64_t t = env->NowMicros();
  double consumed_time = (double)(t - t_start) / CLOCKS_PER_SEC;
  std::cout << consumed_time << std::endl;
}

int main(int argc, char** argv) {
  //   simple_test(argc, argv);

  write_stress(argc, argv);
  return 0;
}