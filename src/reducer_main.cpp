/**
 * @file
 * @brief Dummy 'reducer' stage to count words in a text file
 *
 * Usage:
 * ./reducer  <reduce modulo>
 */

#include "common/shared_locations.h"
#include "common/util.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <utility>
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
  std::map<std::string, std::string> results;
  std::map<std::string, std::vector<std::string>> combine_results;
  std::string rootdir;
  std::string filename;
  std::string active_key;
  unsigned int n_mappers;
  std::vector<std::string> map_fnames;
  bool combining_;

  inline int Combine() {
    for (auto &fname : this->map_fnames) {
      std::ifstream file(fname);
      if (!file.is_open()) {
        std::cerr << "Unable to open file: " << fname << "\n";
        return 1;
      }

      // Each line is formatted:
      // <key>,v0,v1,...vN
      // where v0...vN are comma separated strings of the values for <key>
      for (std::string line; std::getline(file, line, '\n');) {
        std::istringstream linestream(line);
        std::vector<std::string> values;

        std::getline(linestream, this->active_key, ',');
        for (std::string v; std::getline(linestream, v, ',');) {
          values.push_back(v);
        }

        this->Reduce(values);
      }
    }

    return 0;
  }

  inline void Emit(std::string value) {
    // Save results to a different map during the combine stage
    if (this->combining_) {
      auto pos = this->combine_results.find(this->active_key);
      if (pos == this->combine_results.end()) {
        this->combine_results.emplace(std::make_pair(
            this->active_key, std::vector<std::string>({value})));
      } else {
        pos->second.push_back(value);
      }
    } else {
      this->results.emplace(std::make_pair(this->active_key, value));
    }
  }

  inline int Persist() {
    // TODO: Update reducer ID to support >1 reducer
    auto fname = common::util::NameReduceOutfile(
        this->rootdir, std::string(OUTDIR), this->filename, 0);
    std::ofstream file(fname, std::ios::trunc);
    if (!file.is_open()) {
      std::cerr << "Unable to open output file " << fname << "\n";
      return 1;
    }

    for (auto &kv_pair : this->results) {
      file << kv_pair.first << "," << kv_pair.second << "\n";
    }
    return 0;
  };

public:
  AllReducer() = delete;
  inline AllReducer(std::string rootdir, std::string filename,
                    unsigned int n_mappers)
      : rootdir(std::move(rootdir)), filename(std::move(filename)),
        n_mappers(n_mappers), combining_(false) {
    // Make sure root dir path ends in a "/"
    if (this->rootdir.size() > 0 &&
        this->rootdir.substr(this->rootdir.size() - 1, 1) != "/") {
      this->rootdir.append("/");
    }

    for (int x = 0; x < n_mappers; x++) {
      this->map_fnames.push_back(common::util::NameIntermediateFile(
          this->rootdir, mapper::OUTDIR,
          this->filename + "_map_" + std::to_string(x), 0));
    }
  }

  virtual void Reduce(std::vector<std::string> &values) = 0;

  inline int Run() {
    // Aggregate values for each unique key
    this->combining_ = true;
    if (this->Combine() != 0) {
      return 1;
    }

    // Actually reduce
    this->combining_ = false;
    for (auto &kv : this->combine_results) {
      this->active_key = kv.first;
      this->Reduce(kv.second);
    }

    // Save results
    return this->Persist();
  }
};

class Adder : public AllReducer {
public:
  inline Adder(std::string rootdir, std::string filename,
               unsigned int n_mappers)
      : AllReducer(std::move(rootdir), std::move(filename), n_mappers){};

  inline void Reduce(std::vector<std::string> &values) override {
    std::vector<unsigned long> parsed_values(values.size());
    std::transform(values.begin(), values.end(), parsed_values.begin(),
                   [](std::string v) { return atoi(v.c_str()); });

    this->Emit(std::to_string(
        std::accumulate(parsed_values.begin(), parsed_values.end(), 0)));
  };
};

} // namespace reducer

int main(int argc, char *argv[]) {

  if (argc != 4) {
    std::cerr << reducer::cmd::USAGE << std::endl;
    /* std::cerr << "reducer Received: "; */
    /* for (int x = 0; x < argc; x++) { */
    /*   std::cerr << argv[x] << " "; */
    /* } */
    /* std::cerr << std::endl; */
    return 1;
  }

  auto adder = reducer::Adder(std::string(argv[1]), // root directory
                              std::string(argv[2]), // Original input file name
                              atoi(argv[3]));       // # of map tasks

  return adder.Run();
}
