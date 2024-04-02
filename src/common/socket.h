#pragma once

#include <string>

#include <sys/poll.h>
#include <sys/select.h>

#define NAGLE_ON 0
#define NAGLE_OFF 1

namespace common {
/**
 * @class Socket
 * @brief Abstraction of a raw unix socket
 *
 *
 * Original Author: Ji-Yong Shin (j.shin@northeastern.edu)
 * Adapted by: Jesse Steinberg (jessesteinberg01@gmail.com)
 */
class Socket {
public:
  Socket();
  virtual ~Socket();

  int Send(char *buffer, int size, int flags = 0);
  int Recv(char *buffer, int size, int flags = 0);

  int NagleOn(bool on_off);
  bool IsNagleOn();

  void Close();

protected:
  int fd_;
  bool is_initialized_;

private:
  int nagle_;
};
} // namespace common
