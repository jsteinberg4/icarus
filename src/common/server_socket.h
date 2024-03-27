#pragma once

#include <memory>

#include "socket.h"

namespace common {
class ServerSocket : public Socket {
public:
  ServerSocket() {}
  ~ServerSocket() {}

  ServerSocket(int fd, bool nagle_on = NAGLE_ON);

  bool Init(int port);
  std::unique_ptr<ServerSocket> Accept();
};
} // namespace common
