#include "worker_node.h"
#include "common/task.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

namespace worker {

void WorkerNode::Run(std::string master_ip, int port) {

  if (!this->stub.Register(master_ip, port)) {
    /* std::cerr << "WorkerNode::run: failed to register with master\n"; */
    return;
  }

  try {
    this->ForkLoop();
  } catch (std::runtime_error &e) {
    // Fail silently; worker will be restarted
    return;
  }
}

void WorkerNode::ForkLoop() {
  while (true) {
    common::Task t = this->stub.RequestTask();

    if (t.GetStatus() == common::Status::InProgress) {
      auto status = this->ExecTask(t);
      this->stub.SubmitTask(t, status);
    }
  }
}

common::Status WorkerNode::ExecTask(common::Task t) {
  int exit_status;
  pid_t child = fork();

  if (child < 0) {
    std::cerr << "WorkerNode::ExecTask: failed to fork task\n";
    return common::Status::Idle;
  } else if (child == 0) { // Child
    execl((t.GetRoot() + "/" + t.GetObjPath()).c_str(), t.GetObjPath().c_str(),
          t.GetRoot().c_str(), t.GetInputPath().c_str(), t.GetOutPath().c_str(),
          NULL);

    // Should be unreachable; just an extra safeguard if execl fails
    exit(1);
  }

  // Parent
  if (waitpid(child, &exit_status, 0) == -1) {
    // FIXME: Better handling of wait errors
    return common::Status::Idle;
  }

  // Consider any program that doesnt exit(0) as a failure.
  if (WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == EXIT_SUCCESS) {
    return common::Status::Done;
  }

  return common::Status::Idle;
}

} // namespace worker
