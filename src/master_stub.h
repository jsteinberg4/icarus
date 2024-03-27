#pragma once

#include "common/task.h"
namespace master {

/**
 * @class MasterStub
 * @brief RPC stub for MapReduce master to coordinate with different types of
 * clients
 *
 */
class MasterStub {
  void Init(/* TODO: Socket? */);

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
};
} // namespace master
