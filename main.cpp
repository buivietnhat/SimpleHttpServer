#include <iostream>
#include <memory>
#include <networking/http_server.h>

int main() {
  int num_worker = 4;
  const std::string host = "0.0.0.0";
  const int port = 8080;
  auto socket = std::make_unique<Socket>();
  HttpServer server{num_worker, std::move(socket)};
  server.Start(host, port);


  while (true) {

    }
  return 0;
}
