#include "include/networking/http_server.h"

HttpServer::HttpServer(std::unique_ptr<Socket> &&socket)
    : socket_(std::move(socket)) {
}

void HttpServer::Start(const std::string &host, int port) {
  socket_->Start(host, port);

}
