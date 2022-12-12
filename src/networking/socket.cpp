
#include "include/networking/socket.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void Socket::SetUp() {
  // create the listening socket
  fd_server_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (fd_server_ == -1) {
    throw std::runtime_error("failed to create socket");
  }

  // set socket option
  int arg = 1;
  if (setsockopt(fd_server_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &arg, sizeof(arg)) == -1) {
    throw std::runtime_error("failed to set socket option");
  }
}

void Socket::Bind(const std::string &host, int port) {
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  inet_pton(AF_INET, host.c_str(), &(address.sin_addr.s_addr));
  address.sin_port = htons(port);

  if (bind(fd_server_, (sockaddr *) &address, sizeof(address)) == -1) {
    throw std::runtime_error("failed to bind socket");
  }
}

void Socket::Listen() {
  if (listen(fd_server_, MAX_BACKLOG_SIZE) == -1) {
    throw std::runtime_error("cannot listen to socket");
  }
}

int Socket::GetFd() const {
  return fd_server_;
}

void Socket::Start(const std::string &host, int port) {
  SetUp();
  Bind(host, port);
  Listen();
}

Socket::~Socket() {
  close(fd_server_);
}
