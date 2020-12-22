// Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#include <string>
#include "iostream"

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

using namespace ROCKSDB_NAMESPACE;

std::string kDBPath = "/tmp/rocksdb_simple_example";

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

void DefaultConfig(Options& options) {
  std::cout << "Using default configs" << std::endl;
  options.nodes.push_back(new ClusterNode(std::string("127.0.0.1"), 3241));
  options.this_node = options.nodes.front();
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
  kDBPath = root_node.get<std::string>("db_path");
  bool is_compaction_leader = root_node.get<bool>("is_compaction_leader");
  std::cout << "The node " << (is_compaction_leader ? "is" : "is not") << " a compaction leader" << std::endl;
  options.disable_auto_compactions = !is_compaction_leader;

  options.this_node = new ClusterNode();
  ParseNode(local_node, options.this_node);
  ParseNodes(all_nodes, options.nodes);
}

int main(int argc, char** argv) {
  std::cout << "Main started" << std::endl;

  Options options;
  // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
  options.IncreaseParallelism(12);
  options.OptimizeLevelStyleCompaction(64 * 1024 * 1024);
  if (argc > 1) {
    LoadConfig(argv[1], options);
  } else {
    DefaultConfig(options);
  }

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
  assert(s.ok());

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
