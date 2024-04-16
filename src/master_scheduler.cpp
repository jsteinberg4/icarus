#include "master_scheduler.h"
#include "common/shared_locations.h"
#include "common/task.h"
#include "common/util.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>

namespace master {

void TaskScheduler::Init(std::string fsroot, std::string input_file,
                         int n_mappers, std::string mapper, int n_reducers,
                         std::string reducer) {
  assert(n_mappers > 0);
  assert(n_reducers == 1);
  this->Reset();
  this->LockAll();
  this->fs_root = fsroot;
  std::string input_bname = common::util::Basename(input_file);

  // Create map tasks
  for (std::string map_input :
       this->PartitionInput(fsroot, input_file, n_mappers)) {
    auto t = common::Task(common::TaskType::Map, fsroot, mapper, map_input,
                          std::to_string(n_reducers), common::Status::Idle);
    this->idle.push_back(t);
  }

  // TODO: Support >1 reducer
  for (int rid = 0; rid < n_reducers; rid++) {
    auto rt =
        common::Task(common::TaskType::Reduce, fsroot, reducer, input_bname,
                     std::to_string(n_mappers), common::Status::Idle);
    this->reduce_tasks.push_back(rt);
  }

  this->is_initialized = true;
  this->ready = false;
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

  std::deque<common::Task> &q = this->GetStatusQueue(old);
  auto t_pos = std::find(q.begin(), q.end(), t);

  if (t_pos != q.end()) {
    common::Task t_in_queue = std::move(*t_pos);
    q.erase(t_pos);

    std::deque<common::Task> &new_q = this->GetStatusQueue(new_);
    new_q.push_back(std::move(t_in_queue));
  }

  this->UnlockSome(lks);
}

common::Task TaskScheduler::GetTask() {
  /*
   * Scheduling Algorithm:
   *
   * 1) Are there idle tasks?
   *  YES - Pop front
   *  NO - Continue
   * 2) Are there active tasks?
   *  YES - A phase is active. Return an empty task
   *  NO - A phase ended. Continue
   * 3) Is map phase done yet?
   *  NO - Map just finished. Trigger reduce phase.
   *  YES - Reduce might be done, continue
   * 4) Is the reduce phase done?
   *   NO -- It just finished. Return an empty task, master should start cleanup
   *   YES -- Return empty task
   */
  // TODO: Scheduler metrics (inside task?)
  if (!this->ready || this->IsComplete()) {
    // Default status is invalid
    return common::Task{};
  }

  // Transition between map and reduce phases
  if (this->idle.empty()) {
    if (!this->active.empty() || this->reduce_done) {
      return common::Task{};
    }
    if (!this->map_done) {
      // Map phase ended, trigger the reduce task(s)
      struct lock_conf lks = {.reduce_tasks_lk = true, .idle_lk = true};
      this->LockSome(lks);
      this->map_done = true;
      for (auto &t : this->reduce_tasks) {
        this->idle.push_back(t);
      }
      this->reduce_tasks.clear();
      std::cout << "Map phase complete\n";
      this->UnlockSome(lks);
    } else if (!this->reduce_done) {
      // Job just completed
      this->reduce_done = true;
      std::cout << "Reduce phase complete\n";
      return common::Task{};
    }
  }

  // Get the next idle task
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
  this->map_done = false;
  this->reduce_done = false;
  return true;
}

bool TaskScheduler::IsComplete() noexcept {
  return this->map_done && this->reduce_done;
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

  this->ready = false;
  this->is_initialized = false;
  this->map_done = false;
  this->reduce_done = false;
  this->UnlockAll();
}

std::vector<std::string> TaskScheduler::PartitionInput(std::string fsroot,
                                                       std::string input,
                                                       int n_mappers) {
  std::string partition_base = common::util::Basename(input) + "_map_";
  std::string input_fpath = fsroot + input;
  std::vector<std::string> paths;
  struct stat fstat;
  bool reading;
  int task_size;
  int task_id;

  std::ifstream file{input_fpath};
  if (!file.is_open() || stat(input_fpath.c_str(), &fstat) == -1) {
    std::cerr << "TaskScheduler::Init: unable to load input file "
              << std::quoted(input_fpath) << "\n";
    exit(1); // FIXME: Raise an error or something
  }

  // Note: ctor asserts that n_mappers > 0
  task_size = fstat.st_size / n_mappers;
  reading = true;
  task_id = 0;
  while (reading) {
    std::string buf(task_size, '\0');

    // FIXME: Input can be split in the middle of a word. Make content aware!
    if (file.read(&buf[0], task_size)) {
      std::string fname =
          mapper::INPUT_DIR + partition_base + std::to_string(task_id);

      if (std::ofstream pif{fsroot + fname}) {
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
