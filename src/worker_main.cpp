// Usage:
// ./worker <master IP> <# workers>

#include "worker_node.h"
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <csignal> /// for kill
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <thread>
#include <unistd.h> // For fork
#include <vector>

#define USAGE "./worker <master IP> <master port> <# workers>"

namespace coordinator {
volatile static std::sig_atomic_t quit;

// Set quit flag on SIGINT
static void sigint_handler(int sig) { quit = 1; }

// Configure SIGINT handler for CTRL+C
static void signals_init(void) {
  struct sigaction sa;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sigint_handler;
  sigfillset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
}

void kill_worker(pid_t worker) {
  if (kill(worker, SIGKILL) != 0) {
    std::cerr << "exit: problem killing child pid=" << worker << "\n\t"
              << std::strerror(errno) << "\n";
  }
}

}; // namespace coordinator

int main(int argc, char *argv[]) {
  std::string ip;
  int port;
  unsigned int n_workers;
  std::vector<pid_t> workers;

  if (argc != 4) {
    // TODO: Cmdline arg to randomly kill child procs
    std::cout << USAGE << std::endl;
    return 1;
  }
  ip = std::string(argv[1]);
  port = atoi(argv[2]);
  n_workers = atoi(argv[3]);
  coordinator::signals_init();

  // FIXME: Single proc for debugging
  if (n_workers == 0) {
    worker::WorkerNode child;
    child.Run(ip, port);
    return 0;
  }

  // Maintain a pool of workers. Cleanup on SIGINT
  while (!coordinator::quit) {
    int estatus = 0;
    pid_t dead = -1;

    if (workers.size() < n_workers) {
      pid_t c_pid = fork();

      if (c_pid == 0) {
        std::cout << "Starting new worker\n";
        worker::WorkerNode child;
        child.Run(ip, port);
        std::cout << "Return from worker run\n";
        return 1;

      } else {
        std::cout << "Inserting new child process: " << c_pid << "\n";
        workers.push_back(c_pid);

        std::cout << "Current pids: [ ";
        std::for_each(workers.begin(), workers.end(),
                      [](pid_t p) { std::cout << p << ", "; });
        std::cout << "] size=" << workers.size() << "\n";
      }
    }

    if ((dead = waitpid(-1, &estatus, WNOHANG)) > 0) {
      workers.erase(std::remove(workers.begin(), workers.end(), dead),
                    workers.end());
      std::cout << "Removed dead(?) process id=" << dead << "\n";
    }
    std::this_thread::sleep_for(std::chrono::seconds(rand() % 3));
  }

  // Cleanup child processes
  std::for_each(workers.begin(), workers.end(), coordinator::kill_worker);

  return 0;
}
