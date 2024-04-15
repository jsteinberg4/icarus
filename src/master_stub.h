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

  /**
   * @brief Set the socket for this stub
   *
   * @param socket active TCP socket connected to another node
   */
  void Init(std::unique_ptr<common::TcpSocket> sock);

  /**
   * @brief Receive a node's identification message
   */
  common::rpc::NodeType RecvRegistration();

  // -------------------
  // Client coordination
  // -------------------
  // TODO: Set client configuration
  void ClientAcceptConfig();

  // -------------------
  // Worker coordination
  // -------------------
  common::Task WorkerTaskUpdate();
  bool WorkerIsAlive();

  /**
   * Tell worker to complete a task
   *
   * @param t map or reduce task configuration
   * TODO: Assigned tasks should be sent with status InProgress
   */
  void AssignTask(common::Task &t /* TODO: what else? */);

protected:
  std::unique_ptr<common::TcpSocket> socket;

  /*
   * Accepts an incoming request
   *
   * Receives a request and unmarshalls it into req
   */
  int RecvRequest(common::rpc::Request &req) const noexcept;

  /**
   * @brief Send an RPC request
   *
   * @param type request type identifier
   * @param data request payload
   * @param data_len length of data in bytes
   * @return bytes written (same as send(2) syscall)
   */
  int SendRequest(common::rpc::RequestType type, std::unique_ptr<char> data,
                  int data_len) const noexcept;
};
} // namespace master
