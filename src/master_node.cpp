#include "master_node.h"
#include <iostream>

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
  /* this->server.Accept(); */

  // TODO: Initialize a few threads
  // - T0: Listen for incoming clients (user clients or workers). Assign a
  //       thread to handle each.
  // - T1: Client connection handler
  // - T2-n workers: Pool of threads for worker comms
}

} // namespace master
