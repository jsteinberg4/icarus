#pragma once

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

  int Size() const;
  std::string GetObjPath() const noexcept;
  std::string GetInputPath() const noexcept;
  std::string GetOutPath() const noexcept;
  Status GetStatus() const noexcept;
  void SetObjPath(std::string path) noexcept;
  void SetInputPath(std::string path) noexcept;
  void SetOutPath(std::string path) noexcept;
  void SetStatus(Status s) noexcept;

  /**
   * @brief Write this object as a bytestring
   *
   * @param buffer bytestring destination
   * @param bufsize length of the bytestring buffer. Should be large enough to
   * contain object serialization.
   * @return n bytes written
   */
  int Marshall(char *buffer, int bufsize) const;

  /**
   * @brief Initialize this object from a bytestring
   *
   * @param buffer bytestring of object data
   * @param bufsize length of the bytestring
   */
  void Unmarshall(const char *buffer, int bufsize);

  /**
   * @brief Compare task equality
   *
   * Two tasks are equal iff:
   * - They describe the same executable
   * - They describe the same input file
   *
   * @param other
   * @return true IFF the tasks are equal
   */
  bool operator==(const Task &other) {
    return (this->obj_path == other.obj_path) &&
           (this->input_path == other.input_path);
  }

private:
  std::string obj_path;
  std::string input_path;
  std::string result_path; // TODO: Tasks likely to have *several* output
                           // paths... Make a filepath stem?
  Status status;
};

} // namespace common
