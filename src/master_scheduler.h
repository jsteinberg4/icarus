#pragma once

#include "common/task.h"
#include <atomic>
#include <deque>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

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
   * @param n_mappers # partitions to break input_file into
   * @param n_reducers # of result files
   *
   * TODO: What error type??
   */
  void Init(std::string fsroot, std::string input_file, int n_mappers,
            std::string mapper, int n_reducers, std::string reducer);

  void UpdateTask(const common::Task &t, common::Status old,
                  common::Status new_) noexcept;
  common::Task GetTask();

  bool IsComplete() noexcept;
  bool MarkReady() noexcept;

private:
  struct lock_conf {
    bool map_tasks_lk = false;
    bool reduce_tasks_lk = false;
    bool idle_lk = false;
    bool active_lk = false;
    bool done_lk = false;

    inline void EnableType(common::TaskType t) {
      switch (t) {
      case common::TaskType::Map:
        this->map_tasks_lk = true;
        break;
      case common::TaskType::Reduce:
        this->reduce_tasks_lk = true;
        break;
      default:
        break;
      }
    }

    inline void EnableStatus(common::Status s) {
      switch (s) {
      case common::Status::Idle:
        this->idle_lk = true;
        break;
      case common::Status::InProgress:
        this->active_lk = true;
        break;
      case common::Status::Done:
        this->done_lk = true;
        break;
      case common::Status::Invalid:
      default:
        break;
      }
    }
  };

  std::atomic_bool is_initialized;
  std::atomic_bool ready; // Don't assign tasks if false. ready is true iff
                          // is_initialized is true.
  std::string fs_root;    // Root path for all files
  std::atomic_bool map_done;
  std::atomic_bool reduce_done;

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
    this->LockSome({
        .map_tasks_lk = true,
        .reduce_tasks_lk = true,
        .idle_lk = true,
        .active_lk = true,
        .done_lk = true,
    });
  }
  inline void LockSome(struct lock_conf lks) {
    if (lks.map_tasks_lk) {
      this->map_tasks_lk.lock();
    }
    if (lks.reduce_tasks_lk) {
      this->reduce_tasks_lk.lock();
    }
    if (lks.idle_lk) {
      this->idle_lk.lock();
    }
    if (lks.active_lk) {
      this->active_lk.lock();
    }
    if (lks.done_lk) {
      this->done_lk.lock();
    }
  }

  inline void UnlockAll() noexcept {
    this->UnlockSome({
        .map_tasks_lk = true,
        .reduce_tasks_lk = true,
        .idle_lk = true,
        .active_lk = true,
        .done_lk = true,
    });
  }
  inline void UnlockSome(struct lock_conf lks) {
    if (lks.done_lk) {
      this->done_lk.unlock();
    }
    if (lks.active_lk) {
      this->active_lk.unlock();
    }
    if (lks.idle_lk) {
      this->idle_lk.unlock();
    }
    if (lks.reduce_tasks_lk) {
      this->reduce_tasks_lk.unlock();
    }
    if (lks.map_tasks_lk) {
      this->map_tasks_lk.unlock();
    }
  }

  /**
   * @brief Clear all internal state
   */
  void Reset() noexcept;

  std::vector<std::string> PartitionInput(std::string fsroot, std::string input,
                                          int n_mappers);
  inline std::deque<common::Task> &GetStatusQueue(common::Status s) {
    if (s == common::Status::Idle) {
      std::cout << "Getting idle queue\n";
      return this->idle;
    }
    if (s == common::Status::InProgress) {
      std::cout << "Getting active queue\n";
      return this->active;
    }

    return this->done;
  }
};
} // namespace master
