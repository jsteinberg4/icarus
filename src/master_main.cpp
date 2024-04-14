#include "common/messages.h"
#include "master_node.h"
#include <cstdlib>
#include <iostream>
#include <utility>

// Usage:
// ./master <port no>
// TODO: Optional CLI configuration
#define USAGE "./master <port> <path/to/root/dir>"

int main(int argc, char *argv[]) {
  int port;
  std::string fsmount;
  master::MasterNode node;
  master::TaskScheduler ts;

  if (argc != 3) {
    std::cerr << USAGE << std::endl;
    return 1;
  }
  port = atoi(argv[1]);
  fsmount = std::string(argv[2]);
  // FIXME: Mount path could be invalid

  // Initialization
  /* node.SetScheduler(ts); */
  /* node.SetWorkers(master::DEFAULT_WORKER_POOL); */
  node.SetDefaultTaskSize(master::DEFAULT_TASK_SIZE);
  node.SetFSMount(fsmount);

  // TODO: Cleanup?
  node.ServeRequests(port);
  std::cout << "Server returned!\n";

  return 0;
}
