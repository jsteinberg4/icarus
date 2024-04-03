#include "worker_node.h"
#include "common/task.h"
#include <chrono>
#include <iostream>
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

    if (t.Valid()) {
      auto status = this->ExecTask(t); // Handle fork+exec
      this->stub.SubmitTask(t, status);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
  }
}

//------------------
// Private functions
//------------------

common::Status WorkerNode::ExecTask(common::Task &t) {
  pid_t pid = fork();

  if (pid < 0) {
    std::cerr << "WorkerNode::ExecTask: failed to fork task\n";
    return common::Status::Idle;
  } else if (pid == 0) { // Child
    // TODO: non-dummy task from request
    execl("/Users/jesse/Course-Repos/mapreduce/hello.sh", "hello.sh", NULL);
    // Should be unreachable; just an extra safeguard for my sanity
    exit(1);
  }

  // Parent
  int exit_status;
  auto self = getpid();
  std::cout << "Worker parent pid=" << self << " blocking till task finishes\n";
  if (waitpid(pid, &exit_status, 0) == -1) {
    // TODO: Handle errors on wait
    std::cerr << "Parent pid=" << pid << ". Error occurred on waitpid\n";
  }

  // FIXME: Any program that doesn't just crash (e.g. calls exit) will be true
  if (WIFEXITED(exit_status)) {
    std::cout << "Worker parent pid=" << self << " task completed\n";
    return common::Status::Done;
  } else {
    std::cout << "Worker parent pid=" << self << " task failed\n";
    return common::Status::Idle;
  }
}

} // namespace worker
