/**
 * @file
 * @brief Dummy 'reducer' stage to count words in a text file
 *
 * Usage:
 * ./reducer  <reduce modulo>
 */

#include <cstdlib>
#include <iostream>
#include <string>

namespace reducer {
namespace cmd {
constexpr const char USAGE[] = "./reducer <TODO>";
}

class Reducer {
protected:
  void Emit(std::string value);

public:
  virtual void Reduce(std::string key, std::string value);
};

} // namespace reducer

int main(int argc, char *argv[]) {
  std::string input_file;
  int reducer_modulo;

  if (argc != 3) {
    std::cerr << reducer::cmd::USAGE << std::endl;
    return 1;
  }
  input_file = argv[1];
  reducer_modulo = atoi(argv[2]);

  return 0;
}
