#include "master_stub.h"
#include "common/messages.h"
#include "common/task.h"
#include "common/tcp_socket.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

namespace master {

void MasterStub::Init(std::unique_ptr<common::TcpSocket> sock) {
  this->socket.swap(sock);
}

common::rpc::NodeType MasterStub::RecvRegistration() {
  common::rpc::Request req;
  if (this->RecvRequest(req) < 1) {
    throw std::runtime_error("MasterStub::WorkerTaskUpdate: socket error");
  }

  if (req.GetType() == common::rpc::RequestType::Register) {
    return req.GetSender();
  }

  // Consider an invalid request to be from invalid sender
  return common::rpc::NodeType::Invalid;
}

void MasterStub::AssignTask(common::Task &t) {
  auto buf = std::unique_ptr<char>(new char[t.Size()]);
  t.Marshall(buf.get(), t.Size());

  if (this->SendRequest(common::rpc::RequestType::TaskUpdate, std::move(buf),
                        t.Size()) < 0) {
    throw std::runtime_error("MasterStub::AssignTask: socket error");
  }
}

common::Task MasterStub::WorkerTaskUpdate() {
  common::rpc::Request req;
  common::Task t;

  // Clear & retry
  if (this->RecvRequest(req) < 1) {
    throw std::runtime_error("MasterStub::WorkerTaskUpdate: socket error");
  }

  // Invalid update
  if (req.GetType() != common::rpc::RequestType::TaskUpdate) {
    return t;
  }

  t.Unmarshall(req.GetData(), req.DataSize());
  return t;
}

//------------------
// Protected functions
//------------------
int MasterStub::RecvRequest(common::rpc::Request &req) const noexcept {
  std::vector<char> buf(req.HeaderSize());
  int b_read = 0;

  // Read the size header
  b_read = this->socket->Recv(buf.data(), req.HeaderSize());
  if (b_read != req.HeaderSize()) {
    return -1;
  }

  // Read packet
  req.UnmarshallHeaders(buf.data(), b_read);
  buf.clear();
  buf.resize(req.DataSize());
  if ((b_read = this->socket->Recv(buf.data(), req.DataSize())) !=
      req.DataSize()) {
    return -1;
  }
  req.UnmarshallData(buf.data(), buf.size());

  return req.Size();
}

int MasterStub::SendRequest(common::rpc::RequestType type,
                            std::unique_ptr<char> data,
                            int data_len) const noexcept {
  common::rpc::Request req;

  req.SetType(type);
  req.SetSender(common::rpc::NodeType::Master);
  if (data_len > 0) {
    req.SetData(std::move(data), data_len);
  }

  // FIXME: for some reason this breaks the VSCode debugger on M1 mac. revert
  // auto buf = std::vector<char>(req.Size());
  std::unique_ptr<char> buf(new char[req.Size()]);
  req.Marshall(buf.get(), req.Size());

  return this->socket->Send(buf.get(), req.Size());
}

} // namespace master
