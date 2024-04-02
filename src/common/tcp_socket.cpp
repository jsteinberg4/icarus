#include <arpa/inet.h>
#include <functional>
#include <iostream>
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
  struct sockaddr_in addr;

  if (this->is_initialized_) {
    return true;
  }

  if (!this->AllocSocket()) {
    perror("ERROR: failed to create a socket");
    return false;
  }

  this->ConfigSockaddr(addr, INADDR_ANY, port);
  if ((bind(this->fd_, (struct sockaddr *)&addr, sizeof(addr))) < 0) {
    perror("ERROR: failed to bind");
    return false;
  }

  listen(this->fd_, 8);

  this->is_initialized_ = true;
  std::cout << "bound socket on port=" << port << "\n";
  return true;
}

bool TcpSocket::Connect(std::string ip, int port) {
  struct sockaddr_in addr;
  if (this->is_initialized_) {
    return 0;
  }

  if (!this->AllocSocket()) {
    perror("ERROR: failed to create a socket");
    return false;
  }

  this->ConfigSockaddr(addr, inet_addr(ip.c_str()), port);
  if ((connect(this->fd_, (struct sockaddr *)&addr, sizeof(addr))) < 0) {
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
  accepted_fd = accept(this->fd_, (struct sockaddr *)&addr, &addr_size);
  if (accepted_fd < 0) {
    perror("ERROR: failed to accept connection");
    return nullptr;
  }

  return std::make_unique<TcpSocket>(accepted_fd, this->IsNagleOn());
}

// ---------------
// Private functions
// ---------------
bool TcpSocket::AllocSocket() {
  this->fd_ = socket(AF_INET, SOCK_STREAM, 0);
  return this->fd_ >= 0;
}

void TcpSocket::ConfigSockaddr(struct sockaddr_in &addr, int s_addr, int port) {
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = s_addr;
  addr.sin_port = htons(port);
}

} // namespace common
