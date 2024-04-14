#include "master_scheduler.h"
#include "common/task.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <vector>

namespace master {

void TaskScheduler::Init(std::string fsroot, std::string input_file,
                         int n_mappers, std::string mapper, int n_reducers,
                         std::string reducer) {
  assert(n_mappers > 0);
  assert(n_reducers > 0);
  this->Reset();
  this->LockAll();
  this->fs_root = fsroot;

  // Create map tasks
  for (std::string map_input :
       this->PartitionInput(fsroot, input_file, n_mappers)) {
    auto t = common::Task(common::TaskType::Map, fsroot, mapper, map_input,
                          std::to_string(n_reducers), common::Status::Idle);
    this->idle.push_back(t);
  }

  // TODO: Create n_reducers reduce tasks (push them to reducer queue!)
  for (int rid = 0; rid < n_reducers; rid++) {
    auto rt = common::Task(common::TaskType::Reduce, fsroot, reducer, "",
                           "reduceOutput_" + std::to_string(rid) + ".txt",
                           common::Status::Idle);
    this->reduce_tasks.push_back(rt);
  }

  this->is_initialized = true;
  this->ready = false;
  this->status = common::Status::Idle;
  this->map_done = false;
  this->reduce_done = false;
  this->UnlockAll();
}

void TaskScheduler::UpdateTask(const common::Task &t, common::Status old,
                               common::Status new_) noexcept {
  // Clunky, but lock the appropriate queues
  struct lock_conf lks;
  lks.EnableType(t.GetType());
  lks.EnableStatus(old);
  lks.EnableStatus(new_);
  this->LockSome(lks);

  std::deque<common::Task> q = this->GetStatusQueue(old);
  auto t_pos = std::find(q.begin(), q.end(), t);
  common::Task t_in_queue = std::move(*t_pos);
  q.erase(t_pos);

  std::deque<common::Task> new_q = this->GetStatusQueue(new_);
  new_q.push_back(std::move(t_in_queue));
  this->UnlockSome(lks);
}

common::Task TaskScheduler::GetTask() {
  // TODO: Scheduler metrics (inside task?)
  if (!this->ready || this->IsComplete()) {
    // Default status is invalid
    return common::Task{};
  }

  if (this->idle.empty()) {
    if (!this->map_done) {
      this->map_done = true;
      std::unique_lock<std::mutex> lk{this->idle_lk};
      this->idle = std::move(this->reduce_tasks);
    } else if (!this->reduce_done) {
      this->reduce_done = true;
      return common::Task{};
    }
  }

  struct lock_conf lks = {.idle_lk = true, .active_lk = true};
  this->LockSome(lks);

  common::Task t = this->idle.front();
  this->idle.pop_front();
  t.SetStatus(common::Status::InProgress);
  this->active.push_back(t);

  this->UnlockSome(lks);
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

bool TaskScheduler::IsComplete() noexcept {
  return this->status == common::Status::Done && this->map_done &&
         this->reduce_done;
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
  this->map_done = false;
  this->reduce_done = false;
  this->UnlockAll();
}

std::vector<std::string> TaskScheduler::PartitionInput(std::string fsroot,
                                                       std::string input,
                                                       int n_mappers) {
  struct stat fstat;
  int task_size;
  std::string input_fpath = fsroot + "/" + input;
  std::ifstream file{input_fpath};
  std::vector<std::string> paths;

  if (!file.is_open() || stat(input_fpath.c_str(), &fstat) == -1) {
    std::cerr << "TaskScheduler::Init: unable to load input file "
              << std::quoted(fsroot + input) << "\n";
    exit(1); // FIXME: Raise an error or something
  }

  task_size = fstat.st_size / n_mappers;
  bool reading = true;
  int task_id = 0;
  while (reading) {
    std::string buf(task_size, '\0');

    // FIXME: Input can be split in the middle of a word. Make content aware!
    if (file.read(&buf[0], task_size)) {
      std::string fname = fsroot + "/mapInput_" + std::to_string(task_id);

      if (std::ofstream pif{fname}) {
        pif << buf;
        paths.push_back(fname);
      }
      task_id++;
    } else {
      reading = false;
    }
  }

  return paths;
}
} // namespace master
