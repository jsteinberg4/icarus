// Usage:
// ./worker <master IP> <# workers>

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

#define USAGE "./worker <master IP> <master port> <# workers>\n"

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

}; // namespace coordinator

int main(int argc, char *argv[]) {
  std::string ip;
  int port;
  int n_workers;
  std::vector<pid_t> workers;

  if (argc != 4) {
    std::cout << USAGE;
    return 1;
  }

  ip = std::string(argv[1]);
  port = atoi(argv[2]);
  n_workers = atoi(argv[3]);

  coordinator::signals_init();
  while (!coordinator::quit) {
    if (workers.size() < n_workers) {
      pid_t c_pid = fork();

      if (c_pid == 0) {
        pid_t self = getpid();
        while (1) {
          if (std::rand() % 2 == 0) {
            std::cout << "Random proc death pid=" << self << "\n";

            return 0;
            /* exit(0); */
          }

          std::cout << "Child: pid=" << self << "\n";
          std::this_thread::sleep_for(std::chrono::seconds(10));
        }
      } else {
        workers.push_back(c_pid);
      }
    }

    if (!workers.empty()) {

      auto isDead = [](pid_t p) { return kill(p, 0) != 0; };
      /* workers.erase(std::remove_if(workers.begin(), workers.end(), isDead));
       */
      /* std::this_thread::sleep_for(std::chrono::seconds(2)); */
      int estatus = 0;
      auto pid = waitpid(-1, &estatus, WNOHANG);
      std::cout << "Proc died: =" << pid << "\n";
      std::for_each(workers.begin(), workers.end(), [isDead](pid_t p) {
        std::cout << p << (isDead(p) ? "=dead\n" : "=alive\n");
      });
      workers.erase(std::remove_if(workers.begin(), workers.end(), isDead),
                    workers.end());
    }
    std::this_thread::sleep_for(std::chrono::seconds(rand() % 3));
  }

  for (pid_t child : workers) {
    if (kill(child, SIGKILL) != 0) {
      std::cerr << "exit: problem killing child pid=" << child << "\n\t"
                << std::strerror(errno) << "\n";
    }
  }

  return 0;
}
