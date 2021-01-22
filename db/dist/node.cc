#include "rocksdb/node.h"
#include "../../include/rocksdb/node.h"


namespace ROCKSDB_NAMESPACE {
const std::string& ClusterNode::getIp() const { return ip; }
void ClusterNode::setIp(const std::string& _ip) { ClusterNode::ip = _ip; }
uint32_t ClusterNode::getPort() const { return port; }
void ClusterNode::setPort(int _port) { ClusterNode::port = _port; }
bool ClusterNode::operator==(ClusterNode& another) {
  return ip == another.ip && port == another.port;
}
ClusterNode::ClusterNode() = default;
ClusterNode::ClusterNode(std::string  _ip, uint32_t _port)
    : ip(std::move(_ip)), port(_port) {}
std::string ClusterNode::ToString() { return ip + ":" + std::to_string(port) + "[" + std::to_string(compacted_bytes) + "]"; }
bool ClusterNode::operator!=(ClusterNode& another) { return ip != another.ip || port != another.port; }
uint64_t ClusterNode::getCompactedBytes() const { return compacted_bytes; }
void ClusterNode::setCompactedBytes(uint64_t compactedBytes) {
  compacted_bytes = compactedBytes;
}
uint64_t ClusterNode::getTempLoad() const { return temp_load; }
void ClusterNode::setTempLoad(uint64_t tempLoad) { temp_load = tempLoad; }
}  // namespace ROCKSDB_NAMESPACE
