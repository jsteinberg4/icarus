#include "master_node.h"
#include <cstdlib>
#include <iostream>

constexpr const char USAGE[] =
    "./master [port] [root directory] [input path] [mappers]\n\n"
    "port: Specify which TCP port to listen at\n"
    "root directory: specify an absolute path as your root directory. All "
    "other file paths will be relative to this. Most likely $(pwd)\n"
    "input path: Specify the task's input file. Assumed relative to the root.\n"
    "mappers: Specify the number of map tasks to create from the input file\n";

int main(int argc, char *argv[]) {
  int port;
  std::string fsmount;
  master::MasterNode node;
  master::TaskScheduler ts;

  if (argc != 5) {
    std::cerr << USAGE << std::endl;
    return 1;
  }
  port = atoi(argv[1]);
  fsmount = std::string(argv[2]);

  // Initialization
  // FIXME: Mount path could be invalid
  node.SetFSMount(fsmount);

  node.ServeRequests(port, std::string(argv[3]), atoi(argv[4]));

  return 0;
}
