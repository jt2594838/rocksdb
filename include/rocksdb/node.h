#ifndef ROCKSDB_NODE_H
#define ROCKSDB_NODE_H

#include <string>
#include "rocksdb/rocksdb_namespace.h"

namespace ROCKSDB_NAMESPACE {
class ClusterNode {
 private:
  std::string ip;
  uint32_t port = 0;
  uint64_t compacted_bytes = 0;
  uint64_t temp_load = 0;

 public:
  ClusterNode();
  ClusterNode(std::string  ip, uint32_t port);
  const std::string& getIp() const;
  void setIp(const std::string& _ip);
  uint32_t getPort() const;
  void setPort(int _port);
  bool operator== (ClusterNode& another);
  bool operator!= (ClusterNode& another);
  std::string ToString();
  uint64_t getCompactedBytes() const;
  void setCompactedBytes(uint64_t compactedBytes);
  uint64_t getTempLoad() const;
  void setTempLoad(uint64_t tempLoad);
};
}


#endif  // ROCKSDB_NODE_H
