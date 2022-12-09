
#ifndef SIMPLEHTTPSERVER_SRC_HTTP_SERVERR_H_
#define SIMPLEHTTPSERVER_SRC_HTTP_SERVERR_H_

#include <memory>
#include <networking/socket.h>
#include <networking/connection_manager.h>

class HttpServer {
 public:
  HttpServer(std::unique_ptr<Socket> &&socket);
  void Start(const std::string &host, int port);

 private:
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<ConnectionManager> cm_;
};

#endif//SIMPLEHTTPSERVER_SRC_HTTP_SERVERR_H_
