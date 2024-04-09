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
  t.SetStatus(common::Status::Idle);
  auto buf = std::make_unique<char>(t.Size());
  t.Marshall(buf.get(), t.Size());

  // Retransmit until sent
  while (this->SendRequest(common::rpc::RequestType::TaskUpdate, std::move(buf),
                           t.Size()) < t.Size())
    ;

  // Clear & retry
  std::cout << "Waiting for new task...\n";
  while (this->RecvRequest(resp) < 1) {
    resp.Reset();
  }

  // Invalid task on error
  if (resp.GetType() != common::rpc::RequestType::TaskUpdate) {
    return common::Task{};
  }

  t.Unmarshall(resp.GetData().get(), resp.DataSize());
  std::cout << "Got a new task!\n";
  return t;
}

void WorkerStub::SubmitTask(common::Task &t, common::Status status) {
  common::rpc::Request req;
  t.SetStatus(status); // TODO: move this logic into the worker?

  auto buf = std::make_unique<char>(t.Size());
  t.Marshall(buf.get(), t.Size());

  // Retransmit until sent
  while (this->SendRequest(common::rpc::RequestType::TaskUpdate, std::move(buf),
                           t.Size()) < t.Size())
    ;
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
  buf.resize(req.DataSize());
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

  std::cout << "WorkerStub::SendRequest: bytes to send=" << req.Size() << "\n";
  auto written = this->socket.Send(buf.data(), req.Size());
  std::cout << "WorkerStub::SendRequest: bytes written=" << written << "\n";
  return written;
}

} // namespace worker
