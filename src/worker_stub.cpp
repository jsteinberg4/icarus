#include "worker_stub.h"
#include "common/task.h"
#include <iostream>
#include <thread>

namespace worker {

bool WorkerStub::Register(std::string ip, int port) {
  if (this->socket.Connect(ip, port)) {
    // TODO: Send an identity message to the server
    return true;
  }

  return false;
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
