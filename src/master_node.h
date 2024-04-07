#pragma once

#include "common/tcp_socket.h"
#include "master_scheduler.h"
#include "master_stub.h"
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace master {

constexpr int DEFAULT_TASK_SIZE = 16 << 20; // 16MB

/**
 * @class MasterNode
 * @brief A MapReduce master node
 *
 */
class MasterNode {
public:
  MasterNode() noexcept;

  /**
   * @brief Configure the task scheduling algorithm
   * TODO: Add back if necessary
   *
   * @param ts task scheduler instance
   */
  /* void SetScheduler(TaskScheduler ts); */

  /**
   * @brief Default size to partition user's input file
   *
   * Each worker's Map task will have this file size by default.
   *
   * @param size file size in bytes
   */
  void SetDefaultTaskSize(int size);

  /**
   * @brief Base filepath for the MapReduce configuration. All mapper/reducer
   * objects, input files, and results filepaths will be relative to this point.
   *
   * @param rootpath unix-style filepath
   */
  void SetFSMount(std::string rootpath);

  /**
   * @brief Initialize and run the master server
   *
   * @param port TCP port to bind and listen on
   */
  void ServeRequests(int port);

private:
  // Track if there are active clients
  std::atomic_bool client_active;

  TaskScheduler scheduler;
  std::vector<std::thread> coordinators; // Threads talking to workers
  int task_size_default;
  std::string fs_root;
  common::TcpSocket server;

  // TODO: Client connection data structs
  // TODO: Worker connection data structs

  // Thread to accept new clients. Could be main thread.
  void ConnectionListenerThread();

  // Thread to comm w/ client. Should only have one.
  void CoordinatorThread(std::unique_ptr<common::TcpSocket> sock);
  // Thread body to communicate w/ a worker
  void WorkerCoordinatorThread(std::unique_ptr<MasterStub> stub);
  void ClientCoordinatorThread(std::unique_ptr<MasterStub> stub);
  // Thread body to communicate w/ a client
};
} // namespace master
