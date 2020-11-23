#include "rocksdb/node.h"

#include <utility>
namespace ROCKSDB_NAMESPACE {

const std::string& ClusterNode::getIp() const { return ip; }
void ClusterNode::setIp(const std::string& _ip) { ClusterNode::ip = _ip; }
int ClusterNode::getPort() const { return port; }
void ClusterNode::setPort(int _port) { ClusterNode::port = _port; }
bool ClusterNode::operator==(ClusterNode& another) {
  return ip == another.ip && port == another.port;
}
ClusterNode::ClusterNode(std::string  ip, int port)
    : ip(std::move(ip)), port(port) {}
std::string ClusterNode::ToString() { return ip + ":" + std::to_string(port); }
}  // namespace ROCKSDB_NAMESPACE
