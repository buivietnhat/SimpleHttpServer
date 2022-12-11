//
// Created by nhatbui on 09/12/2022.
//

#ifndef SIMPLEHTTPSERVER_SRC_INCLUDE_NETWORKING_SOCKET_H_
#define SIMPLEHTTPSERVER_SRC_INCLUDE_NETWORKING_SOCKET_H_

#include <string>

class Socket {
  static constexpr int MAX_BACKLOG_SIZE = 1024;
 public:
  Socket() = default;
  void Start(const std::string &host, int port);
  int GetFd() const;
  virtual ~Socket();

 private:
  int fd_server_{0};

  void SetUp();
  void Bind(const std::string &host, int port);
  void Listen();
};

#endif//SIMPLEHTTPSERVER_SRC_INCLUDE_NETWORKING_SOCKET_H_
