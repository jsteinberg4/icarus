#include "master_node.h"
#include "common/messages.h"
#include "common/task.h"
#include "common/tcp_socket.h"
#include "master_scheduler.h"
#include "master_stub.h"
#include <iostream>
#include <memory>
#include <thread>
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
  if (!this->server.Bind(port)) {
    std::cerr << "Unable to bind to port=" << port << std::endl;
    return;
  }
  // TODO: Initialize a few threads
  // - T0: Listen for incoming clients (user clients or workers). Assign a
  //       thread to handle each.
  //       DONE: Use current thread
  // - T1: Client connection handler
  // - T2-n workers: Pool of threads for worker comms
  /* std::thread main = std::thread{&MasterNode::ConnectionListenerThread,
   * this}; */
  std::vector<std::thread> coordinators; // size: this->workers

  // Dedicated client thread for better latency
  // TODO: change function to handle client specifically
  /* std::thread client_handler{&MasterNode::CoordinatorThread, this}; */

  this->ConnectionListenerThread();
}

// ---------------
// Private functions
// ---------------
void MasterNode::ConnectionListenerThread() {
  std::unique_ptr<common::TcpSocket> in_sock;

  // TODO: Lock? probably not needed as long as only this thread uses
  while ((in_sock = this->server.Accept())) {
    std::cout << "ConnectionListenerThread: accepted a new connection\n";
    this->coordinators.emplace_back(&MasterNode::CoordinatorThread, this,
                                    std::move(in_sock));
  }
}

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
      // TODO: 2 cases
      // 1) First request for a task
      // 2) Assigned task has failed
      // TODO: Use scheduler to get a new task
      /* stub->AssignTask(next); */
      break;
    case common::Status::InProgress:
      // TODO: Do nothing
      break;
    case common::Status::Done:
      // TODO: Mark task complete; assign new task
      /* this->scheduler.UpdateTask(active, common::Status::Done); */
      /* stub->AssignTask(next); */
      break;
    default:
      // TODO: default case... do nothing?
      break;
    }
  }
}
} // namespace master
