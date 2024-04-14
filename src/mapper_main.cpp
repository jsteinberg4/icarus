/**
 * @file
 * @brief Dummy 'mapper' stage to count words in a text file
 *
 */

#include "common/util.h"
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <ios>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace mapper {
constexpr const char OUTDIR[] = "reducerInputs/";
namespace cmd {
constexpr const char USAGE[] =
    "./mapper <root directory> <input file> <reduce modulo>";
}

class Mapper {
protected:
  std::map<std::string, std::vector<std::string>> intermediate_vals;
  std::string rootdir;
  std::string filename;
  int R; // reducer modulo

  void Emit(std::string key, std::string value) {
    auto iter = this->intermediate_vals.find(key);

    if (iter == this->intermediate_vals.end()) {
      this->intermediate_vals.emplace(key, std::vector<std::string>({value}));
    } else {
      iter->second.push_back(value);
    }
  };

  void Persist() {
    std::ofstream outputs[this->R];
    for (int rid = 0; rid < this->R; rid++) {
      std::string fname = common::util::NameIntermediateFile(
          this->rootdir, OUTDIR, common::util::Basename(this->filename, false),
          rid);
      // Clear file contents if already exists
      std::cout << "Creating output file: " << fname << "\n";
      outputs[rid] = std::ofstream(fname, std::ios::trunc);
    }

    for (auto &kv_pair : this->intermediate_vals) {
      std::string key = kv_pair.first;
      std::vector<std::string> values = kv_pair.second;

      auto hash = std::hash<std::string>{}(key) % this->R;
      std::ofstream &r_fstream = outputs[hash];

      // Write each key as a single line
      r_fstream << key + ",";
      for (auto it = values.begin(); it != values.end(); it++) {
        if (std::next(it) != values.end()) {
          r_fstream << *it + ",";
        } else {
          r_fstream << *it + "\n";
        }
      }
    }
  }

public:
  Mapper(std::string rootdir, std::string filename, int R)
      : rootdir(std::move(rootdir)), filename(std::move(filename)), R(R) {
    // Make sure root dir is a directory path
    if (this->rootdir.size() > 0 &&
        this->rootdir.substr(this->rootdir.size() - 1, 1) != "/") {
      this->rootdir.append("/");
    }
  }
  /**
   * @brief A 'map' function as defined by the MapReduce paradigm. MUST be
   * overridden by users.
   *
   * @param key map function input key
   * @param value map function input value
   */
  virtual void Map(std::string key, std::string value) = 0;

  int Run() {
    // Load file
    std::ifstream file(this->filename);
    std::ostringstream contents;
    if (!file.is_open()) {
      std::cerr << "Unable to open file: " << this->filename << "\n";
      return 1;
    }
    contents << file.rdbuf();

    // Apply user function
    std::cout << "Applying user map function\n";
    this->Map(this->filename, contents.str());

    // Dump output
    std::cout << "Persisting results\n";
    this->Persist();
    return 0;
  }
};

/**
 * @class WordCounter
 * @brief Adapted from MapReduce paper
 *
 * TODO: Citation
 *
 */
class WordCounter : public Mapper {
public:
  WordCounter(std::string rootdir, std::string filename, int R)
      : Mapper(rootdir, filename, R){};
  void Map(std::string filename, std::string contents) override {
    unsigned int i = 0;
    while (i < contents.size()) {
      // Skip past leading white space
      while ((i < contents.size()) &&
             (std::isspace(contents[i]) || std::ispunct(contents[i]))) {
        i++;
      }
      // Find word end
      unsigned int word_start = i;
      while ((i < contents.size()) && !std::isspace(contents[i]) &&
             !std::ispunct(contents[i])) {
        i++;
      }
      if (word_start < i) {
        this->Emit(contents.substr(word_start, i - word_start), "1");
      }
    }
  }
};

} // namespace mapper

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << mapper::cmd::USAGE << std::endl;
    return 1;
  }

  auto wc = mapper::WordCounter(std::string(argv[1]), std::string(argv[2]),
                                atoi(argv[3]));
  return wc.Run();
}
