// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "ThriftService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace  ::rocksdb;

class ThriftServiceHandler : virtual public ThriftServiceIf {
 public:
  ThriftServiceHandler() {
    // Your initialization goes here
  }

  void CompactFiles(TCompactionResult& _return, const TCompactFilesRequest& request) {
    // Your implementation goes here
    printf("CompactFiles\n");
  }

  void DownLoadFile(std::string& _return, const std::string& file_name, const int64_t offset, const int32_t size) {
    // Your implementation goes here
    printf("DownLoadFile\n");
  }

  void PushFiles(const TCompactionResult& output_files, const std::string& source_ip, const int32_t source_port) {
    // Your implementation goes here
    printf("PushFiles\n");
  }

  void SetFileNumber(const int64_t new_file_num) {
    // Your implementation goes here
    printf("SetFileNumber\n");
  }

  void InstallCompaction(TStatus& _return, const TInstallCompactionRequest& request) {
    // Your implementation goes here
    printf("InstallCompaction\n");
  }

  void Put(TStatus& _return, const std::string& key, const std::string& value) {
    // Your implementation goes here
    printf("Put\n");
  }

  void Get(GetResult& _return, const std::string& key) {
    // Your implementation goes here
    printf("Get\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  ::std::shared_ptr<ThriftServiceHandler> handler(new ThriftServiceHandler());
  ::std::shared_ptr<TProcessor> processor(new ThriftServiceProcessor(handler));
  ::std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  ::std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  ::std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

