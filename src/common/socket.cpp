#include "socket.h"

#include <assert.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/types.h>

namespace common {

Socket::Socket() : is_initialized_(false), nagle_(NAGLE_ON) {}

Socket::~Socket() {
  if (is_initialized_) {
    Close();
  }
}

int Socket::Send(char *buffer, int size, int flags) {
  int bytes_written = 0;
  int offset = 0;
  while (size > 0) {
    bytes_written = send(fd_, buffer + offset, size, flags);
    if (bytes_written < 0) {
      /*
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
              perror("ERROR: send retry");
              continue;
      }
      */
      perror("ERROR: failed to send");
      Close();
      return 0;
    }
    size -= bytes_written;
    offset += bytes_written;
    assert(size >= 0);
  }
  return bytes_written;
}

int Socket::Recv(char *buffer, int size, int flags) {
  int bytes_read = 0;
  int offset = 0;
  while (size > 0) {
    std::stringstream s;
    s << "Socket::Recv: reading bytes (" << bytes_read << "B read, " << size
      << "B remain)\n";
    std::cout << s.str();
    bytes_read = recv(this->fd_, buffer + offset, size, flags);
    if (bytes_read <= 0) {
      /*
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
              //perror("ERROR: recv retry");
              continue;
      }
      */
      // perror("ERROR: failed to recv");
      Close();
      return 0;
    }
    assert(bytes_read != 0);

    size -= bytes_read;
    offset += bytes_read;
    assert(size >= 0);
  }
  return bytes_read;
}

int Socket::NagleOn(bool on_off) {
  nagle_ = (on_off ? NAGLE_ON : NAGLE_OFF);
  int result =
      setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, (void *)&nagle_, sizeof(int));
  if (result < 0) {
    perror("ERROR: setsockopt failed");
    return 0;
  }
  return 1;
}

bool Socket::IsNagleOn() { return (nagle_ == NAGLE_ON) ? true : false; }

void Socket::Close() {
  shutdown(fd_, SHUT_RDWR);
  close(fd_);
  // perror("Socket closed");
  is_initialized_ = false;
}
} // namespace common
