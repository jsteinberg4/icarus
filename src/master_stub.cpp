#include "master_stub.h"
#include "common/messages.h"
#include "common/task.h"
#include "common/tcp_socket.h"

#include <iostream>
#include <memory>
#include <vector>

namespace master {

//------------------
// Public functions
//------------------
/* MasterStub::MasterStub() {} */
/* MasterStub::~MasterStub() {} */

void MasterStub::Init(std::unique_ptr<common::TcpSocket> sock) {
  this->socket.swap(sock);
}

common::rpc::NodeType MasterStub::RecvRegistration() noexcept {
  common::rpc::Request req;
  this->RecvRequest(req);

  if (req.GetType() == common::rpc::RequestType::Register) {
    return req.GetSender();
  }

  // Consider an invalid request to be from invalid sender
  return common::rpc::NodeType::Invalid;
}

void MasterStub::AssignTask(common::Task &t) {
  std::cout << "MasterStub::AssignTask\n";
  auto buf = std::make_unique<char>(t.Size());
  t.Marshall(buf.get(), t.Size());

  // Retransmit until sent
  while (this->SendRequest(common::rpc::RequestType::TaskUpdate, std::move(buf),
                           t.Size()) < t.Size())
    ;
}

common::Task MasterStub::WorkerTaskUpdate() noexcept {
  std::cout << "MasterStub::WorkerTaskUpdate\n";
  common::rpc::Request req;
  common::Task t;

  // Clear & retry
  while (this->RecvRequest(req) < 1) {
    req.Reset();
  }

  // Invalid update
  if (req.GetType() != common::rpc::RequestType::TaskUpdate) {
    return t;
  }

  t.Unmarshall(req.GetData().get(), req.DataSize());
  return t;
}

//------------------
// Protected functions
//------------------
int MasterStub::RecvRequest(common::rpc::Request &req) const noexcept {
  std::cout << "MasterStub::RecvRequest\n";
  std::vector<char> buf(req.HeaderSize());
  int b_read = 0;

  // Read the size header
  b_read = this->socket->Recv(buf.data(), req.HeaderSize());
  std::cout << "MasterStub::RecvRequest: header size=" +
                   std::to_string(b_read) +
                   " expected=" + std::to_string(req.HeaderSize()) + "\n";
  if (b_read != req.HeaderSize()) {
    std::cerr << "MasterStub::RecvRequest: size header too small\n";
    exit(1); // FIXME: Remove this. just a brief panic test
  }

  // Read packet
  std::cout << "recvrequest buf size=" << buf.size()
            << " buf cap=" << buf.capacity() << "\n";
  req.UnmarshallHeaders(buf.data(), b_read);
  buf.clear();
  buf.resize(req.DataSize());
  std::cout << "MasterStub::RecvRequest: header says packet size="
            << req.DataSize() << "\n";
  if ((b_read = this->socket->Recv(buf.data(), req.DataSize())) !=
      req.DataSize()) {
    std::cerr << "MasterStub::RecvRequest: packet size incorrect\n";
    exit(1); // FIXME: Remove this. just a brief panic test
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

  auto buf = std::vector<char>(req.Size());
  req.Marshall(buf.data(), buf.capacity());

  return this->socket->Send(buf.data(), buf.size());
}

} // namespace master
