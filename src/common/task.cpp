#include "task.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <iostream>
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

int Task::Marshall(char *buffer, int bufsize) const {
  std::cout << "Task::Marshall \n\t"
            << "root: " + this->root + " len=" << this->root.size() << "\n\t"
            << "obj_path: " + this->obj_path + " len=" << this->obj_path.size()
            << "\n\t"
            << "input_path: " + this->input_path + " len="
            << this->input_path.size() << "\n\t"
            << "result_path: " + this->result_path + " len="
            << this->result_path.size() << "\n\t"
            << "status: " << (int)this->status << "\n";
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

  std::cout << "Task::Unmarshall\n\t"
            << "obj_path: " + this->obj_path + " len=" << this->obj_path.size()
            << "\n\t"
            << "input_path: " + this->input_path + " len="
            << this->input_path.size() << "\n\t"
            << "result_path: " + this->result_path + " len="
            << this->result_path.size() << "\n\t"
            << "status: " << (int)this->status << "\n";
}
} // namespace common
