#pragma once

#include "common/messages.h"
#include "common/task.h"
#include "common/tcp_socket.h"
#include <memory>
#include <string>

namespace worker {
class WorkerStub {
public:
  // TODO: Change return type to master's registration response
  bool Register(std::string ip, int port /* TODO: Worker's metadata */);
  common::Task RequestTask();
  void SubmitTask(common::Task &t,
                  common::Status status /* TODO: Task status */);

protected:
  int RecvRequest(common::rpc::Request &req) noexcept;
  int SendRequest(common::rpc::RequestType type, std::unique_ptr<char> data,
                  int data_len) noexcept;

private:
  common::TcpSocket socket;
};
} // namespace worker
