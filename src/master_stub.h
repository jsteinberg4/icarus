#pragma once

#include "common/messages.h"
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
  MasterStub() = default;
  ~MasterStub() = default;

  // -------------------
  // Initialization
  // -------------------
  /**
   * @brief Set the socket for this stub
   *
   * @param socket active TCP socket connected to another node
   */
  void Init(std::unique_ptr<common::TcpSocket> sock);

  /**
   * @brief Receive a node's identification message
   */
  common::rpc::NodeType RecvRegistration() noexcept;
  /* void AckRegistration();                   // TODO: */

  // -------------------
  // Client coordination
  // -------------------
  // void ClientAccept(/* TODO: */);

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
   * TODO: Assigned tasks should be sent with status InProgress
   */
  void AssignTask(common::Task &t /* TODO: what else? */);

private:
  std::unique_ptr<common::TcpSocket> socket;
};
} // namespace master
