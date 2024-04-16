#pragma once

#include <string>

namespace common {

/* The status of a Map or Reduce task. */
enum class Status {
  Invalid = -1, // Invalid: Used as an error flag
  Idle = 0, // Idle: The task is not currently running/scheduled. Can also be
            // used to mark tasks as failed
  InProgress = 1, // InProgress: The task is currently scheduled to run
  Done = 2,       // Done: The task has successfully completed on a worker
};

enum class TaskType {
  Map = 1,
  Reduce = 2,
};

class Task {
  // TODO: Make input and output paths List[str]. Map tasks will only use one
  // input. Reduce will be many in, many out
  // NOTE: encoding that will suck. Devise scheme to keep single string.
public:
  inline Task() : Task(TaskType::Map, "", "", "", "", Status::Invalid){};

  inline Task(TaskType t, std::string root, std::string obj_path,
              std::string input_path, std::string result_path,
              Status status = Status::Invalid)
      : type(t), root(root), obj_path(std::move(obj_path)),
        input_path(std::move(input_path)), result_path(std::move(result_path)),
        status(status) {}

  int Size() const;
  TaskType GetType() const noexcept;
  std::string GetRoot() const noexcept;
  std::string GetObjPath() const noexcept;
  std::string GetInputPath() const noexcept;
  std::string GetOutPath() const noexcept;
  Status GetStatus() const noexcept;
  void SetType(TaskType t) noexcept;
  void SetRoot(std::string path) noexcept;
  void SetObjPath(std::string path) noexcept;
  void SetInputPath(std::string path) noexcept;
  void SetOutPath(std::string path) noexcept;
  void SetStatus(Status s) noexcept;
  std::string str() const;

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
   * - They describe the same type of task
   *
   * @param other
   * @return true IFF the tasks are equal
   */
  bool operator==(const Task &other) {
    return this->type == other.type && this->root == other.root &&
           this->obj_path == other.obj_path &&
           this->input_path == other.input_path;
  }

private:
  TaskType type;
  std::string root; // Base directory for all other paths
  std::string obj_path;
  std::string input_path;
  std::string result_path;
  Status status;
};

} // namespace common
