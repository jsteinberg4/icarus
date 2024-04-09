#pragma once

#include "common/task.h"
#include <atomic>
#include <deque>
#include <mutex>
#include <string>

namespace master {

/**
 * @class TaskScheduler
 * @brief simple round robin scheduler for map and reduce tasks
 *
 * Thread safe.
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
  void Init(std::string fsroot, std::string input_file, int chunksize,
            int n_mappers, int n_reducers);

  void UpdateTask(const common::Task &t, common::Status old,
                  common::Status new_) noexcept;
  common::Task GetTask();

  bool IsComplete() noexcept;
  bool MarkReady() noexcept;

private:
  std::atomic_bool is_initialized;
  std::atomic_bool ready; // Don't assign tasks if false. ready is true iff
                          // is_initialized is true.
  std::atomic<common::Status> status; // Overall job completion
  std::string fs_root;                // Root path for all files

  // NOTE: Locking order. ONLY lock in this order. Unlock in reverse.
  // 1) map_tasks_lk
  // 2) reduce_tasks_lk
  // 3) idle_lk
  // 4) active_lk
  // 5) done_lk
  std::deque<common::Task> map_tasks;
  std::mutex map_tasks_lk;
  std::deque<common::Task> reduce_tasks;
  std::mutex reduce_tasks_lk;
  std::deque<common::Task> idle;
  std::mutex idle_lk;
  std::deque<common::Task> active;
  std::mutex active_lk;
  std::deque<common::Task> done;
  std::mutex done_lk;

  inline void LockAll() noexcept {
    this->map_tasks_lk.lock();
    this->reduce_tasks_lk.lock();
    this->idle_lk.lock();
    this->active_lk.lock();
    this->done_lk.lock();
  }
  inline void UnlockAll() noexcept {
    this->done_lk.unlock();
    this->active_lk.unlock();
    this->idle_lk.unlock();
    this->reduce_tasks_lk.unlock();
    this->map_tasks_lk.unlock();
  }

  /**
   * @brief Clear all internal state
   */
  void Reset() noexcept;
};
} // namespace master
