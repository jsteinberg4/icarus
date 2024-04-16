# mapreduce

Limited implementation of MapReduce

![build](https://github.com/jsteinberg4/icarus/actions/workflows/build.yml/badge.svg)

## Miscellaneous Design Choices

- Why C++14?
  - C++11 is a standard in distributed systems. C++14 is nearly identical, but adds a few quality of life changes. I mostly opted for C++14 so I could use `std::make_unique`.

# Original Authors

Jeffrey Dean and Sanjay Ghemawat. OSDI'04: Sixth Symposium on Operating System Design and Implementation, San Francisco, CA (2004), pp. 137-150. https://research.google/pubs/mapreduce-simplified-data-processing-on-large-clusters/
