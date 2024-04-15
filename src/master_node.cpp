#include "master_node.h"
#include "common/messages.h"
#include "common/shared_locations.h"
#include "common/task.h"
#include "common/tcp_socket.h"
#include "common/util.h"
#include "master_scheduler.h"
#include "master_stub.h"
#include <algorithm>
#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
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

  // FIXME: Move to master_main.cpp
  std::string infile;
  int n_mappers;
  std::cout << "input filename> ";
  std::cin >> infile;
  std::cout << "# mappers> ";
  std::cin >> n_mappers;
  this->scheduler.Init(this->fs_root, infile, n_mappers, "bin/mapper", 1,
                       "bin/reducer");
  this->scheduler.MarkReady();

  auto acceptor = [this]() {
    // TODO: Lock? probably not needed as long as only this thread uses
    // FIXME: Accept() will block until a new client connects. Try select(2)
    std::unique_ptr<common::TcpSocket> in_sock;
    unsigned long tid = 0;
    while (!this->scheduler.IsComplete()) {
      if ((in_sock = this->server.Accept()) != nullptr) {
        this->coordinators.emplace_back(&MasterNode::CoordinatorThread, this,
                                        std::move(in_sock), tid++);
      } else {
        std::cout << "Listener says server was closed!\n";
      }
    }
  };
  auto t_accept = std::thread{acceptor};

  while (!this->scheduler.IsComplete()) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }
  std::cout << "MapReduce job complete\n";
  std::cout << "Result file at:"
            << common::util::NameReduceOutfile(this->fs_root, reducer::OUTDIR,
                                               common::util::Basename(infile),
                                               0)
            << "\n";
  this->server.Close();
  std::cout << "Server closed\n";
  t_accept.join();
  std::cout << "Listener joined\n";
  for (auto &thread : this->coordinators) {
    if (thread.joinable()) {
      thread.join();
    }
  }
  std::cout << "Coordinators joined\n";
}

// ---------------
// Private functions
// ---------------

void MasterNode::CoordinatorThread(std::unique_ptr<common::TcpSocket> sock,
                                   unsigned long tid) {
  auto stub = std::make_unique<MasterStub>();

  stub->Init(std::move(sock));
  auto node_type = stub->RecvRegistration();

  switch (node_type) {
  case common::rpc::NodeType::Master:
    std::cerr << "Received a connection from another master!\n";
    stub.reset(); // Close connection
    break;
  case common::rpc::NodeType::Worker:
    this->WorkerCoordinatorThread(std::move(stub), tid);
    break;
  case common::rpc::NodeType::Client:
    this->ClientCoordinatorThread(std::move(stub), tid);
    break;
  case common::rpc::NodeType::Invalid:
  default:
    std::cerr << "Received a connection with invalid registration type\n";
  }
}

// ---------------
// Private functions
// ---------------
void MasterNode::ClientCoordinatorThread(std::unique_ptr<MasterStub> stub,
                                         unsigned long tid) {
  std::cout << "[Thread-" + std::to_string(tid) +
                   "]Received a connection from a new client!\n";

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

void MasterNode::WorkerCoordinatorThread(std::unique_ptr<MasterStub> stub,
                                         unsigned long tid) {
  std::string name = "[Thread-" + std::to_string(tid) + "] ";
  std::cout << name + "Received a connection from a worker!\n";

  // NOTE: Track this worker's task as a local variable (not big DS).
  // When *this thread* notices that the worker crashed, flag this task as
  // inactive.
  common::Task scheduled;

  try {
    while (!this->scheduler.IsComplete()) {
      common::Task next;
      common::Task active;

      active = stub->WorkerTaskUpdate();
      switch (active.GetStatus()) {
      case common::Status::Idle:
        if (scheduled.GetStatus() == common::Status::InProgress) {
          // Worker's task failed
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
        break;
      default:
        // TODO: default case... do nothing?
        break;
      }
    }
  } catch (std::exception &e) {
    std::cerr
        << "MasterNode::WorkerCoordinatorThread: Error with connected socket\n";
    if (scheduled.GetStatus() == common::Status::InProgress) {
      this->scheduler.UpdateTask(scheduled, common::Status::InProgress,
                                 common::Status::Idle);
    }
  }
}
} // namespace master
