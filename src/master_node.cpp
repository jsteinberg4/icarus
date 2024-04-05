#include "master_node.h"
#include "common/messages.h"
#include "common/tcp_socket.h"
#include "master_stub.h"
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

namespace master {

// ---------------
// ctor/dtors
// TODO: dtor
// ---------------
MasterNode::MasterNode() {
  this->client_active = false;
  /* this->n_coordinators = DEFAULT_WORKER_POOL; */
  this->task_size_default = DEFAULT_TASK_SIZE;
}
MasterNode::~MasterNode() {}

// ---------------
// Configuration functions
// ---------------

void MasterNode::SetScheduler(TaskScheduler ts) { this->scheduler = ts; }
/* void MasterNode::SetWorkers(int n_workers) { this->n_coordinators =
 * n_workers; } */
void MasterNode::SetDefaultTaskSize(int size) {
  this->task_size_default = size;
}
void MasterNode::SetFSMount(std::string rootpath) { this->fs_root = rootpath; }

// ---------------
// Normal runtime functions
// ---------------
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

  // Thread pool to handle worker coordination
  /* for (auto i = 0; i < this->n_coordinators; i++) { */
  /*   coordinators.emplace_back(&MasterNode::CoordinatorThread, this); */
  /* } */

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
    // FIXME: This is unnecessarily complex
    /* this->incoming_sockets.push(std::move(*(in_sock.release()))); */
    this->coordinators.emplace_back(&MasterNode::CoordinatorThread, this,
                                    std::move(in_sock));
    std::cout << "ConnectionListenerThread: new thread spawned\n";
  }
}

void MasterNode::CoordinatorThread(std::unique_ptr<common::TcpSocket> sock) {
  std::cout << "CoordinatorThread start\n";
  auto stub = std::make_unique<MasterStub>();

  stub->Init(std::move(sock));
  std::cout << "MasterNode::CoordinatorThread stub is initialized\n";
  auto node_type = stub->RecvRegistration();

  // TODO: Do some work to register & ack registration
  switch (node_type) {
  case common::rpc::NodeType::Master:
    std::cerr << "Received a connection from another master!\n";
    break;
  case common::rpc::NodeType::Worker:
    std::cout << "Received a connection from a worker!\n";
    break;
  case common::rpc::NodeType::Client:
    std::cout << "Received a connection from a new client!\n";
    break;
  case common::rpc::NodeType::Invalid:
  default:
    std::cerr << "Received a connection with invalid registration type\n";
  }

  while (true) {
    // TODO: Start handling requests
    /* common::rpc::Request req = stub->RecvRequest(); */
    common::rpc::Request req;
    /* stub->RecvRequest(req); */
    switch (req.GetType()) {
    case common::rpc::RequestType::Register:
      break;
    case common::rpc::RequestType::TaskUpdate:
      break;
    case common::rpc::RequestType::Invalid:
    default:
      break;
    }
  }
}
} // namespace master
