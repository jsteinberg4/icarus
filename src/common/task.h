#pragma once

#include <memory>
#include <string>
namespace common {

// TODO: Migrate to separate header
class Marshallable {
public:
  Marshallable() = delete;
  virtual ~Marshallable();

  /**
   * @brief Write this object as a bytestring
   *
   * @param buffer bytestring destination
   * @param bufsize length of the bytestring buffer. Should be large enough to
   * contain object serialization.
   * @return n bytes written
   */
  virtual int Marshall(std::unique_ptr<char> buffer, int bufsize) const;

  /**
   * @brief Initialize this object from a bytestring
   *
   * @param buffer bytestring of object data
   * @param bufsize length of the bytestring
   */
  virtual void Unmarshall(std::unique_ptr<char> buffer, int bufsize) const;
};

/* The status of a Map or Reduce task.
 *
// InProgress: The task is currently scheduled to run
// Done: The task has successfully completed on a worker
 * */
enum class Status {
  Idle = 0, // Idle: The task is not currently running/scheduled. Can also be
            // used to mark tasks as failed
  InProgress = 1, // InProgress: The task is currently scheduled to run
  Done = 2,       // Done: The task has successfully completed on a worker
};

class Task : virtual public Marshallable {
public:
  /**
   * @brief Checks if the task has been initialized
   *
   * @return true if properly initialized, false otherwise
   */
  inline bool Valid() const { return true; }

  int Marshall(std::unique_ptr<char> buffer, int bufsize) const override;
  void Unmarshall(std::unique_ptr<char> buffer, int bufsize) const override;

private:
  std::string obj_path;
  std::string input_path;
  std::string result_path;
  Status status;
};

} // namespace common
