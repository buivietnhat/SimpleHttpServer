#include <iostream>
#include <memory>
#include <networking/http_server.h>
#include "api/root.h"

int main() {
  int num_worker = 5;
  const std::string host = "0.0.0.0";
  const int port = 8080;
  auto socket = std::make_unique<Socket>();
  HttpServer server{num_worker, std::move(socket)};

  server.Register("/", HttpMethod::GET, Greeting);

  server.Start(host, port);

  while (true) {

    }
  return 0;
}
