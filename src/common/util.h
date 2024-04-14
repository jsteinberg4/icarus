#pragma once

#include <string>
namespace common {
namespace util {

/**
 * @brief Build the filename of a map output/reducer input
 *
 * @param base root directory to write to
 * @param output_dir relative path to the intermediate file directory
 * @param map_input filename of the map task input
 * @param reduce_id number of the reduce task. If MapReduce is run with 5
 * reduce tasks, this is a number [0, 5)
 *
 * @return the absolute file path
 */
std::string NameIntermediateFile(std::string base, std::string output_dir,
                                 std::string map_input, int reduce_id);

/**
 * @brief Extract basename of a path
 *
 * Example:
 * Given "/path/to/foo/bar/baz.txt"
 * if keep_extension is true, returns "baz.txt"
 * else returns "baz"
 *
 *
 * @param path a unix filepath
 */
std::string Basename(std::string path, bool keep_extension = false);

} // namespace util

} // namespace common
