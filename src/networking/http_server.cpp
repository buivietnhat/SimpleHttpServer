
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
void HttpServer::Register(const std::string &path, HttpMethod method,
                          std::function<HttpResponse(void)> handler) {
  cm_->Register(path, method, handler);
}

void HttpServer::Router::Register(const std::string &path, HttpMethod method,
                                  std::function<HttpResponse(void)> handler) {
  route_[path][method] = handler;
}

auto HttpServer::Router::Serve(const std::string &path, const HttpRequest &request) -> HttpResponse {
  if (route_.count(path) == 0) {
    throw std::logic_error("not supported path's handler");
  }
  auto request_method = request.GetStartLine().method_;
  if (route_[path].count(request_method) == 0) {
    throw std::logic_error("not supported method handler");
  }

  auto handler = route_[path][request_method];
  auto respone = handler();
  return respone;
}
