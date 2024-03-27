#pragma once

#include "common/task.h"
#include <string>

namespace worker {
class WorkerStub {
  void Register(std::string ip, int port /* TODO: Worker's metadata */);
  common::Task RequestTask();
  void SubmitTask(/* TODO: Task status */);
};
} // namespace worker
