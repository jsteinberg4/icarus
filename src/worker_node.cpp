#include "worker_node.h"
#include "common/task.h"
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

namespace worker {

//------------------
// Public functions
//------------------

void WorkerNode::Run(std::string master_ip, int port) {
  if (!this->stub.Register(master_ip, port)) {
    std::cerr << "WorkerNode::run: failed to register with master\n";
    return;
  }

  // TODO: Catch socket errors if master disconnects
  while (true) {
    std::cout << "Requesting a new task!\n";
    common::Task t = this->stub.RequestTask();

    if (t.GetStatus() == common::Status::InProgress) {
      auto status = this->ExecTask(t);
      this->stub.SubmitTask(t, status);
    }
  }

  std::cout << "\033[1;33mend worker run\033[0m\n";
}

//------------------
// Private functions
//------------------

common::Status WorkerNode::ExecTask(common::Task t) {
  int exit_status;
  pid_t parent;
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
  parent = getpid();
  if (waitpid(child, &exit_status, 0) == -1) {
    // TODO: Handle errors on wait
    std::cerr << "Parent pid=" << child << ". Error occurred on waitpid\n";
  }

  // Consider any program that doesnt exit(0) as a failure.
  if (WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == EXIT_SUCCESS) {
    std::cout << "Worker parent pid=" << parent << " task completed\n";
    return common::Status::Done;
  }

  std::cout << "Worker parent pid=" << parent << " task failed\n";
  return common::Status::Idle;
}

} // namespace worker
