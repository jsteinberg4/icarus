#!/usr/bin/make -f

GREEN = \033[1;32m
YELLOW = \033[1;33m
CYAN = \033[1;36m
END = \033[0m

#-------------------------------------------------------------------------------
# Build variables
#-------------------------------------------------------------------------------
# Compiler
CXX := g++

# Flags
CFLAGS := -Wall -std=c++14

# For std::thread
LFLAGS := -pthread

# Files
BASEDIR := src
CMN_DIR := $(BASEDIR)/common
BIN := bin
MAP_INDIR := mapInputs
REDUCE_INDIR := reduceInputs
OUTDIR := mapReduceOutputs

# Shared code
CMN_HDRS := $(wildcard $(CMN_DIR)/*.h)
CMN_SRCS := $(wildcard $(CMN_DIR)/*.cpp)
CMN_OBJS := $(patsubst $(CMN_DIR)/%.cpp,%.o,$(CMN_SRCS))

# TODO: make this better
MASTER_HDRS := $(wildcard $(BASEDIR)/master*.h)
MASTER_SRCS := $(wildcard $(BASEDIR)/master*.cpp)
MASTER_OBJS := $(notdir $(MASTER_SRCS:.cpp=.o))

WORKER_HDRS := $(wildcard $(BASEDIR)/worker*.h)
WORKER_SRCS := $(wildcard $(BASEDIR)/worker*.cpp)
WORKER_OBJS := $(notdir $(WORKER_SRCS:.cpp=.o))

MAPPER_HDRS := $(wildcard $(BASEDIR)/mapper*.h)
MAPPER_SRCS := $(wildcard $(BASEDIR)/mapper*.cpp)
MAPPER_OBJS := $(notdir $(MAPPER_SRCS:.cpp=.o))

REDUCER_HDRS := $(wildcard $(BASEDIR)/reducer*.h)
REDUCER_SRCS := $(wildcard $(BASEDIR)/reducer*.cpp)
REDUCER_OBJS := $(notdir $(REDUCER_SRCS:.cpp=.o))

TARGET := master worker mapper reducer
#-------------------------------------------------------------------------------
# Build Rules
#-------------------------------------------------------------------------------
all:  builddir $(TARGET)

builddir:
	@echo "$(YELLOW)Create build directories$(END)"
	mkdir -p $(BIN)
	mkdir -p $(MAP_INDIR)
	mkdir -p $(REDUCE_INDIR)
	mkdir -p $(OUTDIR)

debug: DFLAGS := -ggdb -DDEBUG
debug: $(TARGET)

master: builddir $(BIN)/master
worker: builddir $(BIN)/worker
mapper: builddir $(BIN)/mapper 
reducer: builddir $(BIN)/reducer
	

$(BIN)/master: $(CMN_OBJS) $(MASTER_OBJS) 
	@echo "$(YELLOW)Compile $@$(END)"
	$(CXX) $(LFLAGS) -o $@ $^

$(BIN)/worker: $(CMN_OBJS) $(WORKER_OBJS)
	@echo "$(YELLOW)Compile $@$(END)"
	$(CXX) $(LFLAGS) -o $@ $^

$(BIN)/mapper: $(CMN_OBJS) $(MAPPER_OBJS)
	@echo "$(YELLOW)Compile $@$(END)"
	$(CXX) $(LFLAGS) -o $@ $^

$(BIN)/reducer: $(CMN_OBJS) $(REDUCER_OBJS)
	@echo "$(YELLOW)Compile $@$(END)"
	$(CXX) $(LFLAGS) -o $@ $^


$(WORKER_OBJS): $(WORKER_HDRS) $(WORKER_SRCS)
	@echo "Compile $@"
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(WORKER_SRCS)

$(MASTER_OBJS): $(MASTER_HDRS) $(MASTER_SRCS)
	@echo "Compile $@"
	$(CXX) $(CFLAGS)  $(DFLAGS) -c $(MASTER_SRCS)

$(MAPPER_OBJS): $(MAPPER_HDRS) $(MAPPER_SRCS)
	@echo "Compile $@"
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(MAPPER_SRCS)

$(REDUCER_OBJS): $(REDUCER_HDRS) $(REDUCER_SRCS)
	@echo "Compile $@"
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(REDUCER_SRCS)


# Build any shared dependencies
$(CMN_OBJS): $(CMN_SRCS) $(CMN_HDRS)
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(CMN_SRCS)

clean: reset-inputs
	rm -rf $(BIN)  *.o

reset-inputs:
	rm -rf $(MAP_INDIR) $(REDUCE_INDIR)
	mkdir -p $(MAP_INDIR)
	mkdir -p $(REDUCE_INDIR)



.PHONY: clean
