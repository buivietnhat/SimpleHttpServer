#include "include/networking/http_server.h"
#include <iostream>
#include <memory>

HttpServer::HttpServer(int num_worker, std::unique_ptr<Socket> &&socket)
    : socket_(std::move(socket)) {
  cm_ = std::make_unique<ConnectionManager>(num_worker);
}

void HttpServer::Start(const std::string &host, int port) {
  std::cout << "starting HTTP server on " << host << ":" << port << std::endl;
  socket_->Start(host, port);
  cm_->ListenAndProcess(socket_->GetFd());
}
