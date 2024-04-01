#include "master_stub.h"
#include "common/tcp_socket.h"

#include <memory>
#include <utility>

namespace master {

//------------------
// Public functions
//------------------
MasterStub::MasterStub() {}
MasterStub::~MasterStub() {}

void MasterStub::Init(std::unique_ptr<common::TcpSocket> socket) {
  this->socket = std::move(socket);
}
} // namespace master
