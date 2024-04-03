#include "worker_stub.h"
#include "common/messages.h"
#include "common/task.h"
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace worker {

bool WorkerStub::Register(std::string ip, int port) {
  common::rpc::Request id;
  char buf[common::rpc::REQUEST_BUF_MAX] = {};
  int msg_size = 0;
  std::string master_name = ip + ":" + std::to_string(port);

  if (!this->socket.Connect(ip, port)) {
    std::cerr << "WorkerStub::Register: Unable to connect to master: " +
                     master_name + "\n";
    return false;
  }

  id.SetType(common::rpc::RequestType::Register);
  id.SetSender(common::rpc::NodeType::Worker);
  // TODO: data for worker registration

  msg_size = id.Marshall(buf, common::rpc::REQUEST_BUF_MAX);
  if (!this->socket.Send(buf, msg_size)) {
    std::cerr << "WorkerStub::Register: Failed to send registration message to "
                 "master: " +
                     master_name + "\n";
    return false;
  }

  return true;
}
common::Task WorkerStub::RequestTask() {
  common::Task t;
  std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 2500));
  return t;
}
void WorkerStub::SubmitTask(common::Task &t,
                            common::Status status /* TODO: Task status */) {
  std::cout << "WorkerStub::SubmitTask\n";
}

} // namespace worker
