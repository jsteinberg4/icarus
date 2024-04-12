/**
 * @file
 * @brief Dummy 'mapper' stage to count words in a text file
 *
 * Usage:
 * ./mapper <input file> <reduce modulo>
 */

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
namespace cmd {
constexpr const char USAGE[] = "./mapper <input file> <reduce modulo>";
}

class Mapper {
protected:
  std::map<std::string, std::vector<std::string>> intermediate_vals;
  std::string filename;
  int R; // reducer modulo
  std::map<std::string, std::ofstream> r_fstreams;

  virtual ~Mapper() {
    for (auto &fname_stream : this->r_fstreams) {
      if (fname_stream.second.is_open()) {
        fname_stream.second.close();
      }
    }
  }

  void Emit(std::string key, std::string value) {
    auto iter = this->intermediate_vals.find(key);

    if (iter == this->intermediate_vals.end()) {
      this->intermediate_vals.emplace(key, std::vector<std::string>({value}));
    } else {
      iter->second.push_back(value);
    }
  };

  void Persist() {
    for (auto &kv_pair : this->intermediate_vals) {
      std::string key = kv_pair.first;
      std::vector<std::string> values = kv_pair.second;
      std::ofstream &r_fstream = this->GetOutputFile(key);

      // Write each key as a single line
      r_fstream << key + ",";
      for (auto it = values.begin(); it != values.end(); it++) {
        if (std::next(it) == values.end()) {
          r_fstream << *it + ",";
        } else {
          r_fstream << *it + "\n";
        }
      }
    }
  }

  std::ofstream &GetOutputFile(std::string key) {
    auto hash = std::hash<std::string>{}(key) % this->R;
    // TODO: Get correct output file name using hash
    std::string r_filename = "reducer_file.txt";
    auto iter = this->r_fstreams.find(r_filename);

    if (iter == this->r_fstreams.end()) {
      std::ofstream stream(r_filename, std::ios::app);
      this->r_fstreams.emplace(r_filename, std::move(stream));
      iter = this->r_fstreams.find(r_filename);
    }

    return iter->second;
  }

public:
  Mapper(std::string filename, int R) : filename(std::move(filename)), R(R) {}
  virtual void Map(std::string key, std::string value);
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
    this->Map(this->filename, contents.str());

    // Dump output
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
  WordCounter(std::string filename, int R) : Mapper(filename, R){};
  void Map(std::string filename, std::string contents) override {
    int i = 0;
    while (i < contents.size()) {
      // Skip past leading white space
      while ((i < contents.size()) && std::isspace(contents[i])) {
        i++;
      }
      // Find word end
      int word_start = i;
      while ((i < contents.size()) && !std::isspace(contents[i])) {
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
  std::string input_file;
  int reducer_modulo;

  if (argc != 3) {
    std::cerr << mapper::cmd::USAGE << std::endl;
    return 1;
  }
  input_file = argv[1];
  reducer_modulo = atoi(argv[2]);
  auto wc = mapper::WordCounter(input_file, reducer_modulo);
  return wc.Run();
}
