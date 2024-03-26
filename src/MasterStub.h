#ifndef __MASTER_STUB_H__
#define __MASTER_STUB_H__

class MasterStub {

  // -------------------
  // Client coordination
  // -------------------
  void ClientAccept(/* TODO: */);

  // -------------------
  // Worker coordination
  // -------------------
  /**
   * @brief Register a new worker
   */
  void WorkerAccept();
};

#endif // !__MASTER_STUB_H__
