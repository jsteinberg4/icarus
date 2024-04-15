#include "task.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
namespace common {

int Task::Size() const {
  return (sizeof(int) * 4) // 3 ints to describe length of each path
         + this->root.size() + this->obj_path.size() + this->input_path.size() +
         this->result_path.size() + sizeof(this->status);
}

TaskType Task::GetType() const noexcept { return this->type; }
std::string Task::GetRoot() const noexcept { return this->root; }
std::string Task::GetObjPath() const noexcept { return this->obj_path; }
std::string Task::GetInputPath() const noexcept { return this->input_path; }
std::string Task::GetOutPath() const noexcept { return this->result_path; }
Status Task::GetStatus() const noexcept { return this->status; }

void Task::SetType(TaskType t) noexcept { this->type = t; }
void Task::SetRoot(std::string path) noexcept { this->root = path; }
void Task::SetObjPath(std::string path) noexcept { this->obj_path = path; }
void Task::SetInputPath(std::string path) noexcept { this->input_path = path; }
void Task::SetOutPath(std::string path) noexcept { this->result_path = path; }
void Task::SetStatus(Status s) noexcept { this->status = s; }
std::string Task::str() const {
  std::stringstream ss;
  ss << "Task(root=" << std::quoted(this->root)
     << " obj_path=" << std::quoted(this->obj_path)
     << " input_path=" << std::quoted(this->input_path)
     << " result_path=" << std::quoted(this->result_path)
     << " status=" << (int)this->status << ")";
  return ss.str();
}

int Task::Marshall(char *buffer, int bufsize) const {
  assert(this->Size() <= bufsize);
  memset(buffer, 0, bufsize);

  int offset = 0;
  int n_root_path_size = htonl(this->root.size());
  int n_obj_path_size = htonl(this->obj_path.size());
  int n_input_path_size = htonl(this->input_path.size());
  int n_result_path_size = htonl(this->result_path.size());
  int n_status = htonl((int)this->status);

  // Root path
  memcpy(buffer + offset, &n_root_path_size, sizeof(n_root_path_size));
  offset += sizeof(n_root_path_size);
  memcpy(buffer + offset, this->root.c_str(), this->root.size());
  offset += this->root.size();

  // Object path
  memcpy(buffer + offset, &n_obj_path_size, sizeof(n_obj_path_size));
  offset += sizeof(n_obj_path_size);
  memcpy(buffer + offset, this->obj_path.c_str(), this->obj_path.size());
  offset += this->obj_path.size();

  // Input path
  memcpy(buffer + offset, &n_input_path_size, sizeof(n_input_path_size));
  offset += sizeof(n_input_path_size);
  memcpy(buffer + offset, this->input_path.c_str(), this->input_path.size());
  offset += this->input_path.size();

  // Results path
  memcpy(buffer + offset, &n_result_path_size, sizeof(n_result_path_size));
  offset += sizeof(n_result_path_size);
  memcpy(buffer + offset, this->result_path.c_str(), this->result_path.size());
  offset += this->result_path.size();

  // Task status
  memcpy(buffer + offset, &n_status, sizeof(n_status));
  offset += sizeof(n_status);

  return offset;
}

void Task::Unmarshall(const char *buffer, int bufsize) {
  std::vector<char> buf;
  int offset = 0;
  int n_root_size = 0, root_size = 0;
  int n_obj_path_size = 0, obj_path_size = 0;
  int n_input_path_size = 0, input_path_size = 0;
  int n_result_path_size = 0, result_path_size = 0;
  int n_status = 0;

  // Root path
  memcpy(&n_root_size, buffer + offset, sizeof(n_root_size));
  offset += sizeof(n_root_size);
  root_size = ntohl(n_root_size);

  buf.resize(root_size);
  memcpy(buf.data(), buffer + offset, root_size);
  this->root = std::string(buf.begin(), buf.end());
  offset += root_size;
  buf.clear();

  // Object path
  memcpy(&n_obj_path_size, buffer + offset, sizeof(n_obj_path_size));
  offset += sizeof(n_obj_path_size);
  obj_path_size = ntohl(n_obj_path_size);

  buf.resize(obj_path_size);
  memcpy(buf.data(), buffer + offset, obj_path_size);
  this->obj_path = std::string(buf.begin(), buf.end());
  offset += obj_path_size;
  buf.clear();

  // Input path
  memcpy(&n_input_path_size, buffer + offset, sizeof(n_input_path_size));
  offset += sizeof(n_input_path_size);
  input_path_size = ntohl(n_input_path_size);
  buf.resize(input_path_size);

  memcpy(buf.data(), buffer + offset, input_path_size);
  offset += input_path_size;
  this->input_path = std::string(buf.begin(), buf.end());
  buf.clear();

  // Result path
  memcpy(&n_result_path_size, buffer + offset, sizeof(n_result_path_size));
  offset += sizeof(n_result_path_size);
  result_path_size = ntohl(n_result_path_size);
  buf.resize(result_path_size);

  memcpy(buf.data(), buffer + offset, result_path_size);
  offset += result_path_size;
  this->result_path = std::string(buf.begin(), buf.end());

  // Status
  memcpy(&n_status, buffer + offset, sizeof(n_status));
  this->status = (Status)ntohl(n_status);
}
} // namespace common
