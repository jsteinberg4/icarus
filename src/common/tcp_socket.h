#pragma once

#include "socket.h"
#include <memory>
#include <string>

namespace common {

/**
 * @class TcpSocket
 * @brief A wrapper around basic TCP socket functionality
 *
 */
class TcpSocket : public Socket {
public:
  TcpSocket();
  TcpSocket(int fd, bool nagle_on);
  ~TcpSocket();

  inline TcpSocket(common::TcpSocket &&other) {}

  /**
   * @brief Initialize a TCP server and start listening
   *
   * @param port port to listen on
   * @return true if the socket can be initialized successfully, false otherwise
   */
  bool Bind(int port);
  bool Connect(std::string ip, int port);
  std::unique_ptr<TcpSocket> Accept();

private:
  bool AllocSocket();

  /**
   * @brief Configure an AF_INET socket address
   *
   * @param addr reference to address struct for ipv4
   * @param s_addr value for addr.sin_addr.s_addr
   * @param port port number in host byte order
   */
  void ConfigSockaddr(struct sockaddr_in &addr, int s_addr, int port);
};
} // namespace common
