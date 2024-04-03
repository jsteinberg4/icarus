#include "worker_stub.h"
#include "common/messages.h"
#include "common/task.h"
#include <iostream>
#include <memory>
#include <string>
#include <thread>

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

} // namespace worker
