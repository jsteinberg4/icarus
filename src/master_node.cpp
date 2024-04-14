#include "master_node.h"
#include "common/messages.h"
#include "common/task.h"
#include "common/tcp_socket.h"
#include "master_scheduler.h"
#include "master_stub.h"
#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

namespace master {

MasterNode::MasterNode() noexcept
    : client_active(false), scheduler(), coordinators(),
      task_size_default(DEFAULT_TASK_SIZE), fs_root("/"), server() {}

/* void MasterNode::SetScheduler(TaskScheduler ts) { this->scheduler = ts; } */

void MasterNode::SetFSMount(std::string rootpath) {
  this->fs_root = rootpath;
  // Make sure root dir path ends in "/"
  if (this->fs_root.size() > 0 &&
      this->fs_root.substr(this->fs_root.size() - 1, 1) != "/") {
    this->fs_root.append("/");
  }
}

void MasterNode::SetDefaultTaskSize(int size) {
  this->task_size_default = size;
}

void MasterNode::ServeRequests(int port) {
  std::unique_ptr<common::TcpSocket> in_sock;

  if (!this->server.Bind(port)) {
    std::cerr << "Unable to bind to port=" << port << std::endl;
    return;
  }
  // Attempt at force closing server once tasks finish. Throws an un-catchable
  // error
  // auto gc = [this]() {
  //   while (!this->scheduler.IsComplete()) {
  //     std::cout << "server still active...\n";
  //     std::this_thread::sleep_for(std::chrono::seconds(1));
  //   }
  //   std::cout << "Janitor thread closing the server\n";
  //   this->server.Close();
  // };
  // std::thread gc_thread{gc};

  // FIXME: Remove once clients exist
  this->scheduler.Init(this->fs_root, "inputs/essay.txt", 1, "hello.sh", 1,
                       "hello.sh");
  this->scheduler.MarkReady();

  // TODO: Lock? probably not needed as long as only this thread uses
  // FIXME: Accept() will block until a new client connects. Try select(2)
  while (!this->scheduler.IsComplete() && (in_sock = this->server.Accept())) {
    std::cout << "ConnectionListenerThread: accepted a new connection\n";
    this->coordinators.emplace_back(&MasterNode::CoordinatorThread, this,
                                    std::move(in_sock));
  }
}

// ---------------
// Private functions
// ---------------

void MasterNode::CoordinatorThread(std::unique_ptr<common::TcpSocket> sock) {
  std::cout << "CoordinatorThread start\n";
  auto stub = std::make_unique<MasterStub>();

  stub->Init(std::move(sock));
  std::cout << "MasterNode::CoordinatorThread stub is initialized\n";
  auto node_type = stub->RecvRegistration();

  switch (node_type) {
  case common::rpc::NodeType::Master:
    std::cerr << "Received a connection from another master!\n";
    stub.reset(); // Close connection
    break;
  case common::rpc::NodeType::Worker:
    this->WorkerCoordinatorThread(std::move(stub));
    break;
  case common::rpc::NodeType::Client:
    this->ClientCoordinatorThread(std::move(stub));
    break;
  case common::rpc::NodeType::Invalid:
  default:
    std::cerr << "Received a connection with invalid registration type\n";
  }
}

// ---------------
// Private functions
// ---------------
void MasterNode::ClientCoordinatorThread(std::unique_ptr<MasterStub> stub) {
  // TODO: Do some work to register the client
  std::cout << "Received a connection from a new client!\n";

  // TODO: Pseudocode
  // 1) Register client (if none already active)
  //  1b) Parse client configuration from the "register" message
  // 2) Wait for client to trigger job
  // 3) Signal that scheduling should start
  //  3b) How? set some flag in scheduler?
  // 4) Await job complete signal (some promise?)
  // 5) Return the "Run" rpc

  // while (true) {
  //   // TODO: Start handling requests
  //   /* common::rpc::Request req = stub->RecvRequest(); */
  //   common::rpc::Request req;
  //   /* stub->RecvRequest(req); */
  //   switch (req.GetType()) {
  //   case common::rpc::RequestType::Register:
  //     break;
  //   case common::rpc::RequestType::TaskUpdate:
  //     break;
  //   case common::rpc::RequestType::Invalid:
  //   default:
  //     break;
  //   }
  // }
}

void MasterNode::WorkerCoordinatorThread(std::unique_ptr<MasterStub> stub) {
  // TODO: Do some work to register worker
  std::cout << "Received a connection from a worker!\n";

  // NOTE: Track this worker's task as a local variable (not big DS).
  // When *this thread* notices that the worker crashed, flag this task as
  // inactive.
  common::Task scheduled;

  try {
    while (!this->scheduler.IsComplete()) {
      common::Task next;
      common::Task active;

      active = stub->WorkerTaskUpdate();
      if (scheduled.GetStatus() == common::Status::Invalid) {
        active.SetStatus(common::Status::Idle);
      }
      switch (active.GetStatus()) {
      case common::Status::Idle:
        if (scheduled.GetStatus() == common::Status::InProgress) {
          // Worker's task failed
          std::cout << "Worker's task failed\n";
          this->scheduler.UpdateTask(scheduled, common::Status::InProgress,
                                     common::Status::Idle);
        }
        next = this->scheduler.GetTask();
        stub->AssignTask(next);
        scheduled = next;
        break;
      case common::Status::InProgress:
        // TODO: Track scheduler metrics? Do literally anything with this info
        break;
      case common::Status::Done:
        this->scheduler.UpdateTask(scheduled, common::Status::InProgress,
                                   common::Status::Done);
        scheduled = common::Task{};
        scheduled.SetStatus(common::Status::Idle);
        std::cout << "Worker finished the task!\n";
        break;
      default:
        // TODO: default case... do nothing?
        break;
      }
    }
    std::cout << "WorkerCoordinatorThread:: scheduler says done!\n";
  } catch (std::runtime_error &e) {
    std::cerr
        << "MasterNode::WorkerCoordinatorThread: Error with connected socket\n";
    if (scheduled.GetStatus() == common::Status::InProgress) {
      this->scheduler.UpdateTask(scheduled, common::Status::InProgress,
                                 common::Status::Idle);
    }
  }
}
} // namespace master
