#include "master_node.h"
#include "common/messages.h"
#include "common/task.h"
#include "common/tcp_socket.h"
#include "master_scheduler.h"
#include "master_stub.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

namespace master {

MasterNode::MasterNode() noexcept
    : client_active(false), scheduler(), coordinators(),
      task_size_default(DEFAULT_TASK_SIZE), fs_root("/"), server() {}

/* void MasterNode::SetScheduler(TaskScheduler ts) { this->scheduler = ts; } */

void MasterNode::SetFSMount(std::string rootpath) { this->fs_root = rootpath; }

void MasterNode::SetDefaultTaskSize(int size) {
  this->task_size_default = size;
}

void MasterNode::ServeRequests(int port) {
  std::unique_ptr<common::TcpSocket> in_sock;

  if (!this->server.Bind(port)) {
    std::cerr << "Unable to bind to port=" << port << std::endl;
    return;
  }

  // FIXME: Remove once clients exist
  this->scheduler.Init(this->fs_root, "essay.txt", 2, "hello.sh", 1,
                       "hello.sh");
  this->scheduler.MarkReady();

  // TODO: Lock? probably not needed as long as only this thread uses
  while ((in_sock = this->server.Accept())) {
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
  // inactive!
  common::Task active;

  try {
    while (true) {
      common::Task next;
      // NOTE: Pseudocode
      // 1) Get task update
      // 2)
      //  a) Mark completed task as completed (or failed as idle)
      //  b) Catch error when socket is closed. Mark task as idle
      // 3) Assign a new task
      active = stub->WorkerTaskUpdate();

      switch (active.GetStatus()) {
      case common::Status::Idle:
        if (active.GetStatus() == common::Status::InProgress) {
          // Worker's task failed
          this->scheduler.UpdateTask(active, common::Status::InProgress,
                                     common::Status::Idle);
        }
        next = this->scheduler.GetTask();
        std::cout << "Worker wants a task!\n";
        stub->AssignTask(next);
        active = next;
        break;
      case common::Status::InProgress:
        // TODO: Do nothing
        break;
      case common::Status::Done:
        this->scheduler.UpdateTask(active, common::Status::InProgress,
                                   common::Status::Done);
        active = common::Task{};
        active.SetStatus(common::Status::Idle);
        std::cout << "Worker finished the task!\n";
        break;
      default:
        // TODO: default case... do nothing?
        break;
      }
    }
  } catch (std::runtime_error &e) {
    std::cerr
        << "MasterNode::WorkerCoordinatorThread: Error with connected socket\n";
    if (active.GetStatus() == common::Status::InProgress) {
      this->scheduler.UpdateTask(active, common::Status::InProgress,
                                 common::Status::Idle);
    }
  }
}
} // namespace master
