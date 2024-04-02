#pragma once

#include "common/tcp_socket.h"
#include <memory>
#include <string>
namespace master {

constexpr int DEFAULT_WORKER_POOL = 16;
constexpr int DEFAULT_TASK_SIZE = 16 << 20; // 16MB

class TaskScheduler {
private:
  // TODO: List(?) of tasks
  // TODO: Make thread safe
};

/**
 * @class MasterNode
 * @brief A MapReduce master node
 *
 */
class MasterNode {
public:
  MasterNode();
  ~MasterNode();

  /**
   * @brief Configure the task scheduling algorithm
   *
   * @param ts task scheduler instance
   */
  void SetScheduler(TaskScheduler ts);

  /**
   * @brief Set the number of threads communicating with worker nodes
   *
   * @param n_workers [TODO:parameter]
   */
  void SetWorkers(int n_workers);

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
  bool client_active;

  TaskScheduler scheduler;
  int workers; // TODO: better name. this is num threads talking to workers.
  int task_size_default;
  std::string fs_root;
  common::TcpSocket server;

  // TODO: Client connection data structs
  // TODO: Worker connection data structs

  // Thread to accept new clients. Could be main thread.
  void ConnectionListenerThread();
  // Thread to comm w/ client. Should only have one.
  void ClientConnHandler();
};
} // namespace master
