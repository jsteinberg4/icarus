#include "task.h"
namespace common {

bool Task::Valid() const { return true; }
int Task::Marshall(std::unique_ptr<char> buffer, int bufsize) const {
  // TODO: Implement
  return 0;
}
void Task::Unmarshall(std::unique_ptr<char> buffer, int bufsize) const {
  // TODO: Implement
}
} // namespace common
