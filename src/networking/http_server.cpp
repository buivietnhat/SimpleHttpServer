
#include "include/networking/http_server.h"
#include <iostream>
#include <memory>

#include "include/networking/socket.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>

HttpServer::HttpServer(int num_worker, std::unique_ptr<Socket> &&socket)
    : socket_(std::move(socket)) {
  cm_ = std::make_unique<ConnectionManager>(num_worker, std::make_unique<Router>());
}

void HttpServer::Start(const std::string &host, int port) {
  std::cout << "starting HTTP server on " << host << ":" << port << std::endl;

  if ((sock_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
    throw std::runtime_error("Failed to create a TCP socket");
  }

  int opt = 1;
  sockaddr_in server_address;

  if (setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt)) < 0) {
    throw std::runtime_error("Failed to set socket options");
  }

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  inet_pton(AF_INET, host.c_str(), &(server_address.sin_addr.s_addr));
  server_address.sin_port = htons(port);

  if (bind(sock_fd_, (sockaddr *)&server_address, sizeof(server_address)) < 0) {
    throw std::runtime_error("Failed to bind to socket");
  }

  if (listen(sock_fd_, 1000) < 0) {
    std::ostringstream msg;
    msg << "Failed to listen on port " << port;
    throw std::runtime_error(msg.str());
  }

//  socket_->Start(host, port);
//  cm_->ListenAndProcess(socket_->GetFd());
  cm_->ListenAndProcess(sock_fd_);
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
