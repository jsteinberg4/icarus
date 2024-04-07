#pragma once

#include "common/shared_queue.hpp"
#include "common/task.h"
#include <atomic>

namespace master {

/**
 * @class TaskScheduler
 * @brief [TODO:description]
 *
 * Example:
 * TaskScheduler t{};
 * t.Init(input, chunksize, mappers, reducers);
 * t.MarkReady();
 * while (true) {
 *    auto task = t.ScheduleTask();
 *    <assign task to a worker>
 *    <...task is completed>
 *    t.UpdateTask(task, common::Status::Done);
 * }
 */
class TaskScheduler {
public:
  /**
   * @brief Setup the map and reduce tasks for a given input.
   *
   * Note: This will overwrite any existing state
   *
   * @param input_file original input for the whole MapReduce job
   * @param chunksize desired size of each Mapper input in bytes
   * @param n_mappers # partitions to break input_file into
   * @param n_reducers # of result files
   *
   * TODO: What error type??
   */
  void Init(std::string input_file, int chunksize, int n_mappers,
            int n_reducers);

  void UpdateTask(const common::Task &t, common::Status s) noexcept;
  common::Task ScheduleTask();

  bool IsComplete() noexcept;
  bool MarkReady() noexcept;

private:
  std::atomic_bool is_initialized;
  std::atomic_bool ready; // Don't assign tasks if false. ready is true iff
                          // is_initialized is true.
  std::atomic<common::Status> status; // Overall job completion

  common::shared_queue<common::Task> all_tasks;
  common::shared_queue<common::Task> idle;
  common::shared_queue<common::Task> active;
  common::shared_queue<common::Task> done;

  /**
   * @brief Clear all internal state
   */
  void Reset() noexcept;
};
} // namespace master
