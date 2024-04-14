#include "worker_node.h"
#include "common/task.h"
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <system_error>
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
      auto status = this->ExecTask(t); // Handle fork+exec
      break;
      this->stub.SubmitTask(t, status);
    } else {
      std::cout << "Received task that is not in progress\n";
    }

    // FIXME: Remove
    std::this_thread::sleep_for(std::chrono::seconds(10));
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
    // TODO: Arguments/environment vars to pass
    /* execl(t.GetObjPath().c_str(), exe.c_str(), NULL); */
    std::cout << "Worker is exec the following: "
              << (t.GetRoot() + "/" + t.GetObjPath()).c_str() << " "
              << t.GetObjPath().c_str() << " " << t.GetRoot().c_str() << " "
              << t.GetInputPath().c_str() << " " << atoi(t.GetOutPath().c_str())
              << "\n";

    // execl("/Users/jesse/Course-Repos/icarus/hello.sh", "hello.sh",
    //       "/Users/jesse/Course-Repos/icarus",
    //       "/Users/jesse/Course-Repos/icarus/mapInput_1", "1", NULL);
    std::string full_exe = (t.GetRoot() + "/" + t.GetObjPath());
    execl(full_exe.c_str(), t.GetObjPath().c_str(), t.GetInputPath().c_str(),
          t.GetOutPath().c_str(), NULL);

    // Should be unreachable; just an extra safeguard for my sanity
    std::cout << "exec returned (" << errno << ") " << std::strerror(errno)
              << "\n";
    exit(1);
  }

  // Parent
  int exit_status;
  auto self = getpid();
  std::cout << "Worker parent pid=" << self << " blocking until child=" << pid
            << " exits/crashes\n";
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
