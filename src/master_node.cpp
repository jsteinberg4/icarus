#include "master_node.h"
#include "common/tcp_socket.h"
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
  this->workers = DEFAULT_WORKER_POOL;
  this->task_size_default = DEFAULT_TASK_SIZE;
}
MasterNode::~MasterNode() {}

// ---------------
// Configuration functions
// ---------------

void MasterNode::SetScheduler(TaskScheduler ts) { this->scheduler = ts; }
void MasterNode::SetWorkers(int n_workers) { this->workers = n_workers; }
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
  std::thread client_handler;
  std::vector<std::thread> worker_channels; // size: this->workers

  // TODO: Spawn other threads first
  this->ConnectionListenerThread();
}

// ---------------
// Private functions
// ---------------
void MasterNode::ConnectionListenerThread() {
  std::unique_ptr<common::TcpSocket> in_sock;

  // TODO: Lock? probably not needed as long as only this thread uses
  while ((in_sock = this->server.Accept())) {
    // FIXME: This is unnecessarily complex
    this->incoming_sockets.push(std::move(*(in_sock.release())));
  }
}

void MasterNode::ClientConnHandler() {}
} // namespace master
