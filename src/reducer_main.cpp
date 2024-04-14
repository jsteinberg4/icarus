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
#include <vector>

namespace reducer {
namespace cmd {
// Example:
// Assume 3 mappers ran with: `./mapper /home/icarus/ input/file1_map_{0...2} 1`
// Reducer should run as:
// `./reducer /home/icarus file1 3`
constexpr const char USAGE[] =
    "./reducer <root directory> <map task name> <# mappers>";
} // namespace cmd

class AllReducer {
protected:
  void Emit(std::string value);

public:
  virtual void Reduce(std::string key, std::vector<std::string> values) = 0;
};

class Adder : public AllReducer {
public:
  inline virtual void Reduce(std::string key,
                             std::vector<std::string> values) override {
    return;
  };
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
