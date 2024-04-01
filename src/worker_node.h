#pragma once

#include "common/task.h"
#include "worker_stub.h"
namespace worker {

/**
 * @class Worker
 * @brief A MapReduce worker node
 *
 */
class WorkerNode {
public:
  /**
   * @brief Endlessly attempt to execute tasks
   *
   * TODO: signal handler to exit gracefully
   */
  void Run(std::string master_ip, int port);

private:
  WorkerStub stub;

  /**
   * @brief Execute a MapReduce task
   *
   * @param t map or reduce task configuration
   * @return the result status of t
   */
  common::Status ExecTask(common::Task &t);
};
} // namespace worker
