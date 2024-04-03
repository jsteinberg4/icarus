#include "task.h"
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
namespace common {

int Task::Size() const {
  return (sizeof(int) * 3) // 3 ints to describe length of each path
         + this->obj_path.size() + this->input_path.size() +
         this->result_path.size() + sizeof(this->status);
}
std::string Task::GetObjPath() const noexcept { return this->obj_path; }
std::string Task::GetInputPath() const noexcept { return this->input_path; }
std::string Task::GetOutPath() const noexcept { return this->result_path; }
Status Task::GetStatus() const noexcept { return this->status; }
void Task::SetObjPath(std::string path) noexcept { this->obj_path = path; }
void Task::SetInputPath(std::string path) noexcept { this->input_path = path; }
void Task::SetOutPath(std::string path) noexcept { this->result_path = path; }
void Task::SetStatus(Status s) noexcept { this->status = s; }

int Task::Marshall(char *buffer, int bufsize) const {
  assert(this->Size() <= bufsize);
  int offset = 0;
  int n_obj_path_size = htonl(this->obj_path.size());
  int n_input_path_size = htonl(this->input_path.size());
  int n_result_path_size = htonl(this->result_path.size());
  int n_status = htonl((int)this->status);
  memset(buffer, 0, bufsize);

  // Object path
  memcpy(buffer, &n_obj_path_size, sizeof(n_obj_path_size));
  offset += sizeof(n_obj_path_size);
  memcpy(buffer, this->obj_path.c_str(), this->obj_path.size());
  offset += this->obj_path.size();

  // Input path
  memcpy(buffer, &n_input_path_size, sizeof(n_input_path_size));
  offset += sizeof(n_input_path_size);
  memcpy(buffer, this->input_path.c_str(), this->input_path.size());
  offset += this->input_path.size();

  // Results path
  memcpy(buffer, &n_result_path_size, sizeof(n_result_path_size));
  offset += sizeof(n_result_path_size);
  memcpy(buffer, this->result_path.c_str(), this->result_path.size());
  offset += this->result_path.size();

  // Task status
  memcpy(buffer, &n_status, sizeof(n_status));
  offset += sizeof(n_status);

  return offset;
}

void Task::Unmarshall(const char *buffer, int bufsize) {
  int offset = 0;
  int n_obj_path_size, obj_path_size;
  int n_input_path_size, input_path_size;
  int n_result_path_size, result_path_size;
  int n_status;
  /* char pathbuf[bufsize]; */
  std::vector<char> pathbuf(bufsize);
  /* memset(pathbuf, 0, bufsize); */

  // Object path
  memcpy(&n_obj_path_size, buffer, sizeof(n_obj_path_size));
  obj_path_size = ntohl(n_obj_path_size);
  offset += sizeof(n_obj_path_size);
  memcpy(pathbuf.data(), buffer + offset, obj_path_size);
  /* this->obj_path = std::string(pathbuf); */
  this->obj_path.assign(pathbuf.begin(), pathbuf.end());
  offset += this->obj_path.size();
  pathbuf.clear();

  // Input path
  memcpy(&n_input_path_size, buffer + offset, sizeof(n_input_path_size));
  input_path_size = ntohl(n_input_path_size);
  offset += sizeof(n_input_path_size);
  memcpy(pathbuf.data(), buffer + offset, input_path_size);
  this->input_path.assign(pathbuf.begin(), pathbuf.end());
  offset += this->input_path.size();
  pathbuf.clear();

  // Result path
  memcpy(&n_result_path_size, buffer + offset, sizeof(n_obj_path_size));
  result_path_size = ntohl(n_obj_path_size);
  offset += sizeof(n_result_path_size);
  memcpy(pathbuf.data(), buffer + offset, result_path_size);
  this->result_path.assign(pathbuf.begin(), pathbuf.end());
  offset += this->result_path.size();
  pathbuf.clear();

  // Status
  memcpy(&n_status, buffer + offset, sizeof(n_status));
  this->status = (Status)ntohl(n_status);
}
} // namespace common
