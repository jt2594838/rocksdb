#ifndef ROCKSDB_NODE_H
#define ROCKSDB_NODE_H

#include <string>
#include "rocksdb/rocksdb_namespace.h"

namespace ROCKSDB_NAMESPACE {
class ClusterNode {
 private:
  std::string ip;
  int port;

 public:
  ClusterNode(std::string  ip, int port);
  const std::string& getIp() const;
  void setIp(const std::string& _ip);
  int getPort() const;
  void setPort(int _port);
  bool operator== (ClusterNode& another);
  std::string ToString();
};
}


#endif  // ROCKSDB_NODE_H
