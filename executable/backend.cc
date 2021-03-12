// Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include <monitoring/statistics.h>

#include <atomic>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <string>

#include "db/thrift/RpcUtils.h"
#include "iostream"
#include "rocksdb/compaction_filter.h"
#include "rocksdb/comparator.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"

using namespace ROCKSDB_NAMESPACE;

std::string kDBPath = "/tmp/rocksdb_simple_example";

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

void DefaultConfig(Options& options) {
  std::cout << "Using default configs" << std::endl;
  options.nodes.push_back(new ClusterNode(std::string("127.0.0.1"), 3241));
  options.this_node = options.nodes.front();
  options.external_node = new ClusterNode(std::string("127.0.0.1"), 3242);
}

void LoadConfig(char* config_path, Options& options) {
  if (!boost::filesystem::exists(config_path)) {
    std::cout << config_path << " not exists." << std::endl;
    DefaultConfig(options);
    return;
  }
  boost::property_tree::ptree root_node;
  boost::property_tree::ini_parser::read_ini(config_path, root_node);
  std::string local_node = root_node.get<std::string>("local_node");
  std::string all_nodes = root_node.get<std::string>("all_nodes");
  std::string external_node = root_node.get<std::string>("external_node");
  options.enable_dist_compaction = root_node.get<bool>("enable_dist_comp");
  options.IncreaseParallelism(root_node.get<uint32_t>("parallelism"));
  kDBPath = root_node.get<std::string>("db_path");
  bool is_compaction_leader = root_node.get<bool>("is_compaction_leader");
  std::cout << "The node " << (is_compaction_leader ? "is" : "is not")
            << " a compaction leader" << std::endl;
  options.disable_auto_compactions =
      !is_compaction_leader && options.enable_dist_compaction;
  options.OptimizeLevelStyleCompaction(root_node.get<uint64_t>("mem_budget"));
  options.max_write_buffer_number = 8;
  options.target_file_size_base = root_node.get<uint64_t>("l0_file_size");
  options.level0_file_num_compaction_trigger =
      root_node.get<uint64_t>("l0_trigger");
  options.level0_slowdown_writes_trigger =
      options.level0_file_num_compaction_trigger * 5;
  options.level0_stop_writes_trigger =
      options.level0_file_num_compaction_trigger * 8;

  options.this_node = new ClusterNode();
  options.external_node = new ClusterNode();
  ParseNode(local_node, options.this_node);
  ParseNode(external_node, options.external_node);
  ParseNodes(all_nodes, options.nodes);
  RpcUtils::local_addr = new sockaddr;
  memset(RpcUtils::local_addr, 0, sizeof(sockaddr));
  strncpy(RpcUtils::local_addr->sa_data, options.this_node->getIp().c_str(),
          options.this_node->getIp().size());
}

class MyCompactionFilter : public CompactionFilter {
  static std::atomic<uint64_t> entry_cnt;
  uint64_t stall_interval;

  void stall() const {
    clock_t t_start = clock();
    while (clock() - t_start < 1000) {
      // stall 1ms
    }
  }

 public:
  MyCompactionFilter(uint64_t stallInterval) : stall_interval(stallInterval) {}
  const char* Name() const override { return "MyCompactionFilter"; }

  bool Filter(int i, const Slice& slice, const Slice& slice1,
              std::string* string, bool* pBoolean) const override {
    if ((entry_cnt.fetch_add(1) + 1) % stall_interval == 0) {
      stall();
    }
    if (i > 0 && slice.empty() && slice1.empty() && string && pBoolean) {
      // remove unused warning
    }
    return false;
  }
};

class IntComparator : public Comparator {
 public:
  int Compare(const Slice& a, const Slice& b) const override {
    int64_t diff = *reinterpret_cast<const uint64_t*>(a.data()) -
                   *reinterpret_cast<const uint64_t*>(b.data());
    if (diff < 0) {
      return -1;
    } else if (diff == 0) {
      return 0;
    } else {
      return 1;
    }
  }

  const char* Name() const override { return "IntComparator"; }

  void FindShortestSeparator(std::string* /*start*/,
                             const Slice& /*limit*/) const override {}

  void FindShortSuccessor(std::string* /*key*/) const override {}
};

std::atomic<uint64_t> MyCompactionFilter::entry_cnt;

int main(int argc, char** argv) {
  std::cout << "Main started" << std::endl;

  Options options;
  // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
  options.statistics = CreateDBStatistics();

  options.compression = kSnappyCompression;
  options.bottommost_compression = kSnappyCompression;
  options.stats_dump_period_sec = 180;
  options.comparator = new IntComparator();
  // options.compaction_filter = new MyCompactionFilter(100);
  for (auto& i : options.compression_per_level) {
    i = kSnappyCompression;
  }
  if (argc > 1) {
    LoadConfig(argv[1], options);
  } else {
    DefaultConfig(options);
  }
  options.max_subcompactions = options.nodes.size() * 8;

  std::cout << "Local node: " << options.this_node->ToString() << std::endl;
  std::cout << "All nodes:" << std::endl;
  for (auto* node : options.nodes) {
    std::cout << node->ToString() << std::endl;
  }

  // create the DB if it's not already present
  options.create_if_missing = true;

  // open DB
  DB* db;
  Status s = DB::Open(options, kDBPath, &db);
  if (!s.ok()) {
    std::cout << s.ToString() << std::endl;
    return -1;
  }

  std::cout << "DB is up" << std::endl;

  while (true) {
    sleep(1);
  }

  std::cout << "Before closing DB" << std::endl;

  db->Close();
  delete db;

  std::cout << "DB closed" << std::endl;

  return 0;
}
