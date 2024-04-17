# ICARUS ![build](https://github.com/jsteinberg4/icarus/actions/workflows/build.yml/badge.svg)

_I Could Actually Really Use Support (ICARUS)_: A MapReduce Implementation

## Getting Started

### Setup

Dependencies: `make`, a Unix platform which supports C++14

From the repository root, run the following:

```bash
# To compile the binaries and setup intermediate folders
$ make all

# To discard previous runs output & old binaries
$ make clean all
```

This will generate executables to `bin/`.

```sh
bin/
  master   # A MapReduce master node. Run on an isolated VM.
  worker   # MapReduce workers. Run one per virtual machine.
  mapper   # The Map binary. A word counting algorithm is implemented by default. Do not run directly.
  reducer  # The Reduce binary. A summation for the word counting algorithm is implemented by default. Do not run directly.
```

### Usage

All run instructions assume the repository root to be your working directory. Each of the usage messages will be printed by running `bin/master` or `bin/worker` with no arguments.

**Running the Master**:<br />

```
$ bin/master
Usage:
    bin/master [port] [root directory] [input path] [# mappers]
    port: Specify which TCP port to listen at
    root directory: Specify an absolute path as the working directory. All other filepaths internally will use this as a base. It will almost always be the repository root.
    input path: Specify the task's input file. Assumed relative to the root.
    mappers: Specify the number of map tasks to create from the input file
```

**Running the Workers**:<br />

```
$ bin/worker
Usage:
  bin/worker <master IP> <master port> <num workers> [(optional) failure chance]

  master ip: the IP address used by bin/master
  master port: the open port specified as <port> when running bin/master
  num workers: Specify the size of the worker pool. A value of 0 will run a single worker instance which exits the whole program on errors. Any value 1...N will maintain a pool of N child processes, each of which independently connects to the master.
  failure chance: If provided, enables simulated worker failures by killing child processes with probability 1 in <failure chance>. For example, a value of 5 means workers will be killed with probability 1 in 5 (20%). Num workers must be at least 1. If not provided, failure simulation is skipped.
```

#### Examples:

To run the Word Counter benchmark with the master node listening on port 80080 and 100 map partitions. Run 4 virtual nodes per bin/worker execution with a 20% chance of simulated failures.

```bash
# On one virtual machine (or terminal):
$ bin/master 80080 $(pwd) inputs/triple_large.txt 100

# On different virtual machines/terminals
bin/worker <master_ip> 80080 4 5
```

## Citations

[Data sources](/inputs/CITATIONS.md)

### Original Authors

> Jeffrey Dean and Sanjay Ghemawat. OSDI'04: Sixth Symposium on Operating System Design and Implementation, San Francisco, CA (2004), pp. 137-150. https://research.google/pubs/mapreduce-simplified-data-processing-on-large-clusters/
