#pragma once

namespace reducer {
constexpr const char INPUT_DIR[] = "reduceInputs/";
constexpr const char OUTDIR[] = "mapReduceOutputs/";
} // namespace reducer

namespace mapper {
constexpr const char INPUT_DIR[] = "mapInputs/";
constexpr const char *OUTDIR = reducer::INPUT_DIR;
} // namespace mapper
