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

  inline void update_str(std::string s2) { this->result_path = s2; }

private:
  std::string obj_path;
  std::string input_path;
  std::string result_path;
  Status status;
};

} // namespace common
