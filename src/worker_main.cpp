// Usage:
// ./worker <master IP> <# workers>

#include <cerrno>
#include <chrono>
#include <csignal> /// for kill
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h> // For fork
#include <vector>

#define USAGE "./worker <master IP> <master port> <# workers>\n"

volatile static std::sig_atomic_t quit;

namespace worker {} // namespace worker

void sigint_handler(int sig) {
  std::cout << "sigint_handler called\n";
  quit = 1;
}

int main(int argc, char *argv[]) {
  std::string ip;
  int port;
  int n_workers;
  std::vector<pid_t> workers;
  struct sigaction sa;

  if (argc != 4) {
    std::cout << USAGE;
    return 1;
  }

  // Configure SIGINT handler for CTRL+C
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sigint_handler;
  sigfillset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);

  ip = std::string(argv[1]);
  port = atoi(argv[2]);
  n_workers = atoi(argv[3]);

  while (!quit) {
    if (workers.size() < n_workers) {
      pid_t c_pid = fork();

      if (c_pid == 0) {
        pid_t self = getpid();
        while (1) {
          std::cout << "Child: pid=" << self << "\n";

          if (std::rand() % 3 == 0) {
            std::cout << "Random proc death pid=" << self << "\n";
          }
          std::this_thread::sleep_for(std::chrono::seconds(2));
        }
      } else {
        workers.push_back(c_pid);
      }
    }
  }

  for (pid_t child : workers) {
    if (kill(child, SIGKILL) != 0) {
      std::cerr << "exit: problem killing child pid=" << child << "\n\t"
                << std::strerror(errno) << "\n";
    }
  }

  /**/
  /* for (int worker = 0; worker < n_workers; worker++) { */
  /*   int pid = fork(); */
  /**/
  /*   if (pid == 0) { // Child */
  /*     std::cout << "Child process :)\n"; */
  /*     while (1) */
  /*       std::cout << "."; */
  /**/
  /*   } else { */
  /*     std::cout << "Child spawned with pid=" + std::to_string(pid) + "\n"; */
  /*     worker_pids[worker] = pid; */
  /*   } */
  /* } */
  /**/
  /* for (int worker = 0; worker < n_workers; worker++) { */
  /*   int pid = worker_pids[worker]; */
  /*   if (kill(pid, 0) == 0) { */
  /*     std::cout << "Worker pid=" + std::to_string(pid) + */
  /*                      "still alive. Killing it.\n"; */
  /*     kill(pid, SIGKILL); */
  /*   } */
  /* } */

  return 0;
}
