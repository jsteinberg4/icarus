#include "worker_stub.h"
#include "common/messages.h"
#include "common/task.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace worker {

bool WorkerStub::Register(std::string ip, int port) {
  common::rpc::Request id;
  char buf[common::rpc::REQUEST_BUF_MAX];
  int msg_size = 0;
  std::string master_name = ip + ":" + std::to_string(port);

  if (!this->socket.Connect(ip, port)) {
    std::cerr << "WorkerStub::Register: Unable to connect to master: " +
                     master_name + "\n";
    return false;
  }

  id.SetType(common::rpc::RequestType::Register);
  id.SetSender(common::rpc::NodeType::Worker);
  // TODO: data for worker registration

  msg_size = id.Marshall(buf, common::rpc::REQUEST_BUF_MAX);
  int written = 0;
  if (!(written = this->socket.Send(buf, msg_size))) {
    std::cerr << "WorkerStub::Register: Failed to send registration message to "
                 "master: " +
                     master_name + "\n";
    return false;
  }
  std::cout << "WorkerStub::Register msg_size=" << msg_size
            << " bytes written=" << written << "\n";

  return true;
}

// TODO: Reimplement
common::Task WorkerStub::RequestTask() {
  common::rpc::Request req;
  common::rpc::Request resp;
  common::Task t;
  common::Task new_task;
  char task_buf[common::rpc::REQUEST_BUF_MAX];
  char req_buf[common::rpc::REQUEST_BUF_MAX];
  char resp_buf[common::rpc::REQUEST_BUF_MAX];

  req.SetType(common::rpc::RequestType::TaskUpdate);
  req.SetSender(common::rpc::NodeType::Worker);
  /* req.SetData(std::unique_ptr<char> data, int size) */
  // TODO: Serialize the task

  // Silent failure -- worker will resubmit request
  if (!this->socket.Send(req_buf, 0 /* TODO: */)) {
    return common::Task{};
  }

  // Silent failure -- worker will resubmit request
  if (!this->socket.Recv(resp_buf, common::rpc::REQUEST_BUF_MAX)) {
    return common::Task{};
  }

  // TODO: Unmarshall Request and then task from resp_buf
  resp.Unmarshall(resp_buf, common::rpc::REQUEST_BUF_MAX);
  auto body = resp.GetData();

  // Bad response
  if (resp.GetType() != common::rpc::RequestType::TaskUpdate) {
    return common::Task{};
  }
  new_task.Unmarshall(body.first.get(), body.second);

  return new_task;
}

void WorkerStub::SubmitTask(common::Task &t,
                            common::Status status /* TODO: Task status */) {
  common::rpc::Request req;
  char buf[common::rpc::REQUEST_BUF_MAX];
  // TODO:
}

//------------------
// Private functions
//------------------
int WorkerStub::RecvRequest(common::rpc::Request &req) noexcept {
  std::cout << "WorkerStub::RecvRequest\n";
  std::vector<char> buf;
  int size = 0;
  int b_read = 0;

  // Read the size header
  if (this->socket.Recv((char *)&size, sizeof(size)) != sizeof(size)) {
    std::cerr << "WorkerStub::RecvRequest: size header too small\n";
    exit(1); // FIXME: Remove this. just a brief panic test
  }

  // Read packet
  size = ntohl(size);
  buf.reserve(size);
  if ((b_read = this->socket.Recv(buf.data(), size)) != size) {
    std::cerr << "WorkerStub::RecvRequest: packet size incorrect\n";
    exit(1); // FIXME: Remove this. just a brief panic test
  }
  req.Unmarshall(buf.data(), buf.size());

  return size;
}

int WorkerStub::SendRequest(common::rpc::RequestType type,
                            std::unique_ptr<char> data, int data_len) noexcept {
  common::rpc::Request req;
  std::vector<char> buf;

  req.SetType(type);
  req.SetSender(common::rpc::NodeType::Master);
  req.SetData(std::move(data), data_len);

  buf.reserve(req.Size());
  req.Marshall(buf.data(), buf.capacity());

  return this->socket.Send(buf.data(), buf.size());
}

} // namespace worker
