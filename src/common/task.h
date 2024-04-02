#pragma once

#include <memory>
#include <string>
namespace common {

/* The status of a Map or Reduce task. */
enum class Status {
  Idle = 0, // Idle: The task is not currently running/scheduled. Can also be
            // used to mark tasks as failed
  InProgress = 1, // InProgress: The task is currently scheduled to run
  Done = 2,       // Done: The task has successfully completed on a worker
};

class Task {
public:
  Task() : obj_path(), input_path(), result_path(), status(Status::Idle){};
  /**
   * @brief Checks if the task has been initialized
   *
   * @return true if properly initialized, false otherwise
   */
  bool Valid() const;

  /**
   * @brief Write this object as a bytestring
   *
   * @param buffer bytestring destination
   * @param bufsize length of the bytestring buffer. Should be large enough to
   * contain object serialization.
   * @return n bytes written
   */
  int Marshall(std::unique_ptr<char> buffer, int bufsize) const;

  /**
   * @brief Initialize this object from a bytestring
   *
   * @param buffer bytestring of object data
   * @param bufsize length of the bytestring
   */
  void Unmarshall(std::unique_ptr<char> buffer, int bufsize) const;

private:
  std::string obj_path;
  std::string input_path;
  std::string result_path;
  Status status;
};

} // namespace common
