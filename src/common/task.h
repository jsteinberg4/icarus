#pragma once

#include <string>
namespace common {

/* The status of a Map or Reduce task.
 *
// InProgress: The task is currently scheduled to run
// Done: The task has successfully completed on a worker
 * */
enum class Status {
  Failed = -1,    // Failed: Assigned worker did not successfully complete the
                  // task. Should only be sent from worker->master
  Idle = 0,       // Idle: The task is not currently running/scheduled
  InProgress = 1, // InProgress: The task is currently scheduled to run
  Done = 2,       // Done: The task has successfully completed on a worker
};

class Task {
public:
  /**
   * @brief Checks if the task has been initialized
   *
   * @return true if properly initialized, false otherwise
   */
  inline bool Valid() const { return true; }

private:
  std::string obj_path;
  std::string input_path;
};

} // namespace common
