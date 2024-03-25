// Usage:
// ./worker <master IP> <# workers>

#include <csignal> /// for kill
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h> // For fork

#define USAGE "./worker <master IP> <master port> <# workers>\n"

int main(int argc, char *argv[]) {
  std::string ip;
  int port;
  int n_workers;

  if (argc != 4) {
    std::cout << USAGE;
    return 1;
  }

  ip = std::string(argv[1]);
  port = atoi(argv[2]);
  n_workers = atoi(argv[3]);

  int worker_pids[n_workers];

  for (int worker = 0; worker < n_workers; worker++) {
    int pid = fork();

    if (pid == 0) { // Child
      std::cout << "Child process :)\n";
      while (1)
        std::cout << ".";

    } else {
      std::cout << "Child spawned with pid=" + std::to_string(pid) + "\n";
      worker_pids[worker] = pid;
    }
  }

  for (int worker = 0; worker < n_workers; worker++) {
    int pid = worker_pids[worker];
    if (kill(pid, 0) == 0) {
      std::cout << "Worker pid=" + std::to_string(pid) +
                       "still alive. Killing it.\n";
      kill(pid, SIGKILL);
    }
  }

  return 0;
}
