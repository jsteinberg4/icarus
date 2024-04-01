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

  bool Bind(int port);
  bool Connect(std::string ip, int port);
  std::unique_ptr<TcpSocket> Accept();
};

} // namespace common
