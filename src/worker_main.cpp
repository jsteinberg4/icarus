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
#include <unistd.h>
#include <vector>

namespace coordinator {
namespace signals {
volatile static std::sig_atomic_t quit;
// Set quit flag on SIGINT
static void sigint_handler(int sig) { quit = 1; }

// Configure SIGINT handler for CTRL+C
static void init(void) {
  struct sigaction sa;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sigint_handler;
  sigfillset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
}
} // namespace signals

constexpr const char USAGE[] =
    "./worker <master IP> <master port> <# workers> [optional fail chance]";

static void kill_worker(pid_t worker) {
  if (kill(worker, SIGKILL) != 0) {
    std::cerr << "exit: problem killing child pid=" << worker << "\n\t"
              << std::strerror(errno) << "\n";
  }
}

static void simulate_failures(std::vector<pid_t> &workers,
                              unsigned int fail_chance) {
  if (fail_chance > 0) {
    std::for_each(workers.begin(), workers.end(), [fail_chance](pid_t pid) {
      if (rand() % fail_chance == 0) {
        kill_worker(pid);
      }
    });
  }
}

static void run_pool(std::string ip, int port, unsigned int n_workers,
                     unsigned int fail_chance) {
  std::vector<pid_t> workers;

  // Maintain a pool of workers. Cleanup on SIGINT
  while (!signals::quit) {
    int estatus = 0;
    pid_t dead = -1;

    if (workers.size() < n_workers) {
      pid_t c_pid = fork();

      if (c_pid == 0) {
        worker::WorkerNode child;
        child.Run(ip, port);
        exit(0);
      } else {
        // Track child pid
        workers.push_back(c_pid);
      }
    }

    if ((dead = waitpid(-1, &estatus, WNOHANG)) > 0) {
      workers.erase(std::remove(workers.begin(), workers.end(), dead),
                    workers.end());
    }

    /* std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 1000)); */

    // Don't start failing until >1 worker started
    if (workers.size() > 1) {
      simulate_failures(workers, fail_chance);
    }
  }

  // Cleanup child processes
  std::for_each(workers.begin(), workers.end(), coordinator::kill_worker);
}
}; // namespace coordinator

int main(int argc, char *argv[]) {
  std::string ip;
  int port;
  unsigned int n_workers;
  unsigned int fail_chance = 0;

  if (argc != 4 && argc != 5) {
    std::cout << coordinator::USAGE << std::endl;
    return 1;
  }

  ip = std::string(argv[1]);
  port = atoi(argv[2]);
  n_workers = static_cast<unsigned int>(atoi(argv[3]));

  if (argc == 5) {
    if (n_workers < 1) {
      std::cerr
          << "Random chance to kill workers is only supported with >1 worker\n";
      std::cerr << coordinator::USAGE << std::endl;
      return 1;
    }

    fail_chance = static_cast<unsigned int>(atoi(argv[4]));
  }

  coordinator::signals::init();
  if (n_workers == 0) {
    // Single proc for debugging
    worker::WorkerNode child;
    child.Run(ip, port);
  } else {
    coordinator::run_pool(ip, port, n_workers, fail_chance);
  }

  return 0;
}
