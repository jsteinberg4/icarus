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

  if (!this->socket.Connect(ip, port)) {
    std::cerr << "WorkerStub::Register: Unable to connect to master: " + ip +
                     ":" + std::to_string(port) + "\n";
    return false;
  }

  // TODO: data for worker registration?
  auto sent = this->SendRequest(common::rpc::RequestType::Register, nullptr, 0);

  std::cout << "WorkerStub::Register msg_size=" << id.Size()
            << " bytes written=" << sent << "\n";

  return sent > 0;
}

common::Task WorkerStub::RequestTask() {
  common::rpc::Request resp;
  common::Task t{};

  // TODO: setup the payload
  this->SendRequest(common::rpc::RequestType::TaskUpdate, nullptr, 0);

  // Invalid task on error
  this->RecvRequest(resp); // Error handling
  if (resp.GetType() != common::rpc::RequestType::TaskUpdate) {
    return common::Task{};
  }

  // TODO: Unmarshall task from response
  // t.Unmarshall(/* TODO */, int bufsize);
  return t;
}

// TODO: Reimplement
// common::Task WorkerStub::RequestTask() {
//   common::rpc::Request req;
//   common::rpc::Request resp;
//   common::Task t;
//   common::Task new_task;
//   char task_buf[common::rpc::REQUEST_BUF_MAX];
//   char req_buf[common::rpc::REQUEST_BUF_MAX];
//   char resp_buf[common::rpc::REQUEST_BUF_MAX];
//
//   req.SetType(common::rpc::RequestType::TaskUpdate);
//   req.SetSender(common::rpc::NodeType::Worker);
//   /* req.SetData(std::unique_ptr<char> data, int size) */
//   // TODO: Serialize the task
//
//   // Silent failure -- worker will resubmit request
//   if (!this->socket.Send(req_buf, 0 /* TODO: */)) {
//     return common::Task{};
//   }
//
//   // Silent failure -- worker will resubmit request
//   if (!this->socket.Recv(resp_buf, common::rpc::REQUEST_BUF_MAX)) {
//     return common::Task{};
//   }
//
//   // TODO: Unmarshall Request and then task from resp_buf
//   /* resp.Unmarshall(resp_buf, common::rpc::REQUEST_BUF_MAX); */
//
//   // Bad response
//   if (resp.GetType() != common::rpc::RequestType::TaskUpdate) {
//     return common::Task{};
//   }
//   new_task.Unmarshall(resp.GetData().get(), resp.DataSize());
//
//   return new_task;
// }

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
  std::vector<char> buf(req.HeaderSize());
  int b_read = 0;

  // Read the header
  if ((b_read = this->socket.Recv(buf.data(), req.HeaderSize())) !=
      req.HeaderSize()) {
    std::cerr << "WorkerStub::RecvRequest: size header too small\n";
    exit(1); // FIXME: Remove this. just a brief panic test
  }

  // Read packet
  req.UnmarshallHeaders(buf.data(), b_read);
  buf.clear();
  buf.reserve(req.DataSize());
  if ((b_read = this->socket.Recv(buf.data(), req.DataSize())) !=
      req.DataSize()) {
    // TODO: Set request as invalid and return
    std::cerr << "WorkerStub::RecvRequest: packet size incorrect\n";
    exit(1); // FIXME: Remove this. just a brief panic test
  }
  req.UnmarshallData(buf.data(), buf.size());

  return req.Size();
}

int WorkerStub::SendRequest(common::rpc::RequestType type,
                            std::unique_ptr<char> data, int data_len) noexcept {
  common::rpc::Request req;

  req.SetType(type);
  req.SetSender(common::rpc::NodeType::Worker);

  if (data_len > 0) {
    req.SetData(std::move(data), data_len);
  }

  auto buf = std::vector<char>(req.Size());
  req.Marshall(buf.data(), req.Size());

  return this->socket.Send(buf.data(), req.Size());
}

} // namespace worker
