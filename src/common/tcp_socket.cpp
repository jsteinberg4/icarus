#include <arpa/inet.h>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#include "tcp_socket.h"

namespace common {
//------------------
// Public functions
//------------------
TcpSocket::TcpSocket() {}

TcpSocket::TcpSocket(int fd, bool nagle_on) {
  this->fd_ = fd;
  this->is_initialized_ = true;
  this->NagleOn(nagle_on);
}

TcpSocket::~TcpSocket() {}

bool TcpSocket::Bind(int port) {
  if (this->is_initialized_) {
    return true;
  }

  struct sockaddr_in addr;
  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (fd_ < 0) {
    perror("ERROR: failed to create a socket");
    return false;
  }

  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if ((bind(fd_, (struct sockaddr *)&addr, sizeof(addr))) < 0) {
    perror("ERROR: failed to bind");
    return false;
  }

  listen(fd_, 8);

  this->is_initialized_ = true;
  return true;
}

bool TcpSocket::Connect(std::string ip, int port) {
  if (this->is_initialized_) {
    return 0;
  }

  struct sockaddr_in addr;
  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (fd_ < 0) {
    perror("ERROR: failed to create a socket");
    return 0;
  }

  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip.c_str());
  addr.sin_port = htons(port);

  if ((connect(fd_, (struct sockaddr *)&addr, sizeof(addr))) < 0) {
    perror("ERROR: failed to connect");
    return 0;
  }
  this->is_initialized_ = true;
  return 1;
}

std::unique_ptr<TcpSocket> TcpSocket::Accept() {
  int accepted_fd;
  struct sockaddr_in addr;
  unsigned int addr_size = sizeof(addr);
  accepted_fd = accept(fd_, (struct sockaddr *)&addr, &addr_size);
  if (accepted_fd < 0) {
    perror("ERROR: failed to accept connection");
    return nullptr;
  }

  return std::make_unique<TcpSocket>(accepted_fd, this->IsNagleOn());
}

} // namespace common
