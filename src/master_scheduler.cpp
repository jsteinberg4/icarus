#include "master_scheduler.h"
#include "common/task.h"
namespace master {
void TaskScheduler::Init(std::string input_file, int chunksize, int n_mappers,
                         int n_reducers) {
  this->Reset();

  // TODO: partition the input file
  // TODO: Create a map task for each
  // TODO: Create n_reducers reduce tasks

  this->is_initialized = true;
  this->ready = false;
  this->status = common::Status::Idle;
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
  this->all_tasks.clear();
  this->idle.clear();
  this->active.clear();
  this->done.clear();

  this->status = common::Status::Idle;
  this->ready = false;
  this->is_initialized = false;
}
} // namespace master
