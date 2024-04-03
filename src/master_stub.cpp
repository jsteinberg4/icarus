#include "master_stub.h"
#include "common/messages.h"
#include "common/tcp_socket.h"

#include <iostream>
#include <memory>
#include <utility>

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
  char buf[common::rpc::REQUEST_BUF_MAX];
  int insize = 0;

  std::cout << "MasterStub::RecvRegistration\n";
  /* if (!(insize = this->socket->Recv(buf, common::rpc::REQUEST_BUF_MAX))) { */
  if (!(insize = this->socket->Recv(buf, req.Size()))) {
    return common::rpc::NodeType::Invalid;
  }
  std::cout << "MasterStub::RecvRegistration read from buffer size=" << insize
            << "\n";

  req.Unmarshall(buf, insize);
  if (req.GetType() != common::rpc::RequestType::Register) {
    return common::rpc::NodeType::Invalid;
  }

  return req.GetSender();
}

void MasterStub::RecvRequest() { std::cout << "MasterStub::RecvRequest\n"; }
void MasterStub::AssignTask(common::Task &t /* TODO: what else? */) {
  std::cout << "MasterStub::AssignTask\n";
}
} // namespace master
