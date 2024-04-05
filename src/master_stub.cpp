#include "master_stub.h"
#include "common/messages.h"
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

// common::rpc::NodeType MasterStub::RecvRegistration() noexcept {
//   common::rpc::Request req;
//   char buf[common::rpc::REQUEST_BUF_MAX];
//   int insize = 0;
//
//   std::cout << "MasterStub::RecvRegistration\n";
//   // TODO: Use TCP peek option. Assume there's an int for size at head.
//   /* if (!(insize = this->socket->Recv(buf, common::rpc::REQUEST_BUF_MAX))) {
//   */ if (!(insize = this->socket->Recv(buf, req.Size()))) {
//     return common::rpc::NodeType::Invalid;
//   }
//   std::cout << "MasterStub::RecvRegistration read from buffer size=" <<
//   insize
//             << "\n";
//
//   req.Unmarshall(buf, insize);
//   if (req.GetType() != common::rpc::RequestType::Register) {
//     return common::rpc::NodeType::Invalid;
//   }
//
//   return req.GetSender();
// }

void MasterStub::AssignTask(common::Task &t /* TODO: what else? */) {
  std::cout << "MasterStub::AssignTask\n";
}

//------------------
// Private functions
//------------------
int MasterStub::RecvRequest(common::rpc::Request &req) const noexcept {
  std::cout << "MasterStub::RecvRequest\n";
  std::vector<char> buf;
  int size = 0;
  int b_read = 0;

  // Read the size header
  b_read = this->socket->Recv((char *)&size, sizeof(size));
  std::cout << "MasterStub::RecvRequest: header size=" +
                   std::to_string(b_read) + "\n";
  if (b_read != sizeof(size)) {
    std::cerr << "MasterStub::RecvRequest: size header too small\n";
    exit(1); // FIXME: Remove this. just a brief panic test
  }

  // Read packet
  size = ntohl(size);
  buf.reserve(size);
  std::cout << "MasterStub::RecvRequest: header says packet size=" << size
            << "\n";
  if ((b_read = this->socket->Recv(buf.data(), size)) != size) {
    std::cerr << "MasterStub::RecvRequest: packet size incorrect\n";
    exit(1); // FIXME: Remove this. just a brief panic test
  }
  req.Unmarshall(buf.data(), buf.size());

  return size;
}

int MasterStub::SendRequest(common::rpc::RequestType type,
                            std::unique_ptr<char> data,
                            int data_len) const noexcept {
  common::rpc::Request req;
  std::vector<char> buf;

  req.SetType(type);
  req.SetSender(common::rpc::NodeType::Master);
  req.SetData(std::move(data), data_len);

  buf.reserve(req.Size());
  req.Marshall(buf.data(), buf.capacity());

  return this->socket->Send(buf.data(), buf.size());
}

} // namespace master
