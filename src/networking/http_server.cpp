
#include "include/networking/http_server.h"
#include <iostream>
#include <memory>

HttpServer::HttpServer(int num_worker, std::unique_ptr<Socket> &&socket)
    : socket_(std::move(socket)) {
  cm_ = std::make_unique<ConnectionManager>(num_worker, std::make_unique<Router>());
}

void HttpServer::Start(const std::string &host, int port) {
  std::cout << "starting HTTP server on " << host << ":" << port << std::endl;
  socket_->Start(host, port);
  cm_->ListenAndProcess(socket_->GetFd());
}

void HttpServer::Router::Register(const std::string &path, std::function<HttpResponse(void)> handler) {
  route_[path] = handler;
}

auto HttpServer::Router::Serve(const std::string &path) -> HttpResponse {
  if (route_.count(path) == 0) {
    throw std::logic_error("not supported path's handler");
  }

  auto handler = route_[path];
  auto respone = handler();
  return respone;
}
