#include "util.h"

#include <iostream>
#include <libgen.h>
#include <sstream>
#include <string>

namespace common {
namespace util {

std::string NameIntermediateFile(std::string base, std::string output_dir,
                                 std::string map_input, int reduce_id) {
  std::stringstream fname;
  fname << base << output_dir << map_input << "_reduce_" << reduce_id;
  return fname.str();
}

std::string NameReduceOutfile(std::string base, std::string output_dir,
                              std::string input, int reducer_id) {
  std::stringstream fname;
  fname << base << output_dir << input << "_mapreduce_" << reducer_id;
  return fname.str();
}

std::string Basename(std::string path, bool keep_extension) {
  std::string base(basename(&path[0]));

  if (!keep_extension) {
    return base.substr(0, base.find_last_of('.'));
  }

  return base;
}
} // namespace util

} // namespace common
