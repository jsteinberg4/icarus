#include "master_scheduler.h"
#include "common/task.h"
#include <iostream>
#include <mutex>

namespace master {

void TaskScheduler::Init(std::string input_file, int chunksize, int n_mappers,
                         int n_reducers) {
  this->Reset();
  this->LockAll();

  // TODO: partition the input file
  // TODO: Create a map task for each
  // TODO: Create n_reducers reduce tasks

  std::cout << "WARNING: Putting a dummy task (hello.sh) onto queue\n";
  this->idle.emplace_front("/Users/jesse/Course-Repos/icarus/hello.sh", "", "",
                           common::Status::Idle);

  this->is_initialized = true;
  this->ready = false;
  this->status = common::Status::Idle;
  this->UnlockAll();
}

void TaskScheduler::UpdateTask(const common::Task &t, common::Status old,
                               common::Status new_) noexcept {
  // TODO: Find t in the appropriate task queue
}

common::Task TaskScheduler::GetTask() {
  if (!this->ready || this->idle.empty()) {
    // Default status is invalid
    return common::Task{};
  }
  std::unique_lock<std::mutex> lk_outer{this->idle_lk};
  std::unique_lock<std::mutex> lk_inner{this->active_lk};

  common::Task t = this->idle.front();
  this->idle.pop_front();
  t.SetStatus(common::Status::InProgress);
  this->active.push_back(t);

  // TODO: Scheduler metrics (inside task?)
  return t;
}

bool TaskScheduler::MarkReady() noexcept {
  if (!this->is_initialized) {
    this->ready = false;
    return false;
  }

  this->ready = true;
  this->status = common::Status::InProgress;
  return true;
}

void TaskScheduler::Reset() noexcept {
  if (!this->is_initialized) {
    // Nothing to reset
    return;
  }

  // Drop tasks
  this->LockAll();
  this->map_tasks.clear();
  this->reduce_tasks.clear();
  this->idle.clear();
  this->active.clear();
  this->done.clear();

  this->status = common::Status::Idle;
  this->ready = false;
  this->is_initialized = false;
  this->UnlockAll();
}
} // namespace master
