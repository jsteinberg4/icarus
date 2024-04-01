#include "worker_node.h"
#include "common/task.h"
#include <iostream>
#include <sys/wait.h>
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
    common::Task t = this->stub.RequestTask();

    if (t.Valid()) {
      auto status = this->ExecTask(t); // Handle fork+exec
      this->stub.SubmitTask(t, status);
    }
  }
}

//------------------
// Private functions
//------------------

common::Status WorkerNode::ExecTask(common::Task &t) {
  pid_t pid = fork();

  if (pid < 0) {
    std::cerr << "WorkerNode::ExecTask: failed to fork task\n";
    return common::Status::Failed;
  } else if (pid > 0) { // Child
    // TODO: Exec the task
  }

  // Parent
  int exit_status;
  if (waitpid(pid, &exit_status, 0) == -1) {
    // TODO: Handle errors on wait
    /* exex */
  }

  // FIXME: Any program that doesn't just crash (e.g. calls exit) will be true
  if (WIFEXITED(exit_status)) {
    return common::Status::Done;
  } else {
    return common::Status::Failed;
  }

  return common::Status::Done;
}

} // namespace worker
