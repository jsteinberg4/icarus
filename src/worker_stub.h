#pragma once

#include "common/task.h"
#include "common/tcp_socket.h"
#include <string>

namespace worker {
class WorkerStub {
public:
  // TODO: Change return type to master's registration response
  bool Register(std::string ip, int port /* TODO: Worker's metadata */);
  common::Task RequestTask();
  void SubmitTask(common::Task &t,
                  common::Status status /* TODO: Task status */);

private:
  common::TcpSocket socket;
};
} // namespace worker
