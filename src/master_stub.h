#pragma once

#include "common/task.h"
#include "common/tcp_socket.h"
#include <memory>
namespace master {

/**
 * @class MasterStub
 * @brief RPC stub for MapReduce master to coordinate with different types of
 * clients
 *
 */
class MasterStub {
public:
  MasterStub();
  ~MasterStub();

  void Init(std::unique_ptr<common::TcpSocket> socket);

  // -------------------
  // Client coordination
  // -------------------
  void ClientAccept(/* TODO: */);

  // -------------------
  // Worker coordination
  // -------------------

  /*
   * Accepts an incoming request
   */
  void RecvRequest();

  /**
   * Tell worker to complete a task
   *
   * @param t map or reduce task configuration
   */
  void AssignTask(common::Task &t /* TODO: what else? */);

private:
  std::unique_ptr<common::TcpSocket> socket;
};
} // namespace master
