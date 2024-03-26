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

# Shared code
CMN_HDRS := $(wildcard $(CMN_DIR)/*.h)
CMN_SRCS := $(wildcard $(CMN_DIR)/*.cpp)
CMN_OBJS := $(patsubst $(CMN_DIR)/%.cpp,%.o,$(CMN_SRCS))

# TODO: make this better
MASTER_HDRS := $(wildcard $(BASEDIR)/Master*.h)
MASTER_SRCS := $(wildcard $(BASEDIR)/Master*.cpp)
MASTER_OBJS := $(notdir $(MASTER_SRCS:.cpp=.o))

WORKER_HDRS := $(wildcard $(BASEDIR)/Worker*.h)
WORKER_SRCS := $(wildcard $(BASEDIR)/Worker*.cpp)
WORKER_OBJS := $(notdir $(WORKER_SRCS:.cpp=.o))

MAPPER_HDRS := $(wildcard $(BASEDIR)/Mapper*.h)
MAPPER_SRCS := $(wildcard $(BASEDIR)/Mapper*.cpp)
MAPPER_OBJS := $(notdir $(MAPPER_SRCS:.cpp=.o))

REDUCER_HDRS := $(wildcard $(BASEDIR)/Reducer*.h)
REDUCER_SRCS := $(wildcard $(BASEDIR)/Reducer*.cpp)
REDUCER_OBJS := $(notdir $(REDUCER_SRCS:.cpp=.o))

# Flattens SRCS into the top level folder
# OBJS := $(patsubst $(BASEDIR)/%.cpp,%.o,$(SRCS)) 
# OBJS := $(notdir $(SRCS))

# MASTER_SRCS :=  $(filter )

TARGET := master worker mapper reducer
#-------------------------------------------------------------------------------
# Build Rules
#-------------------------------------------------------------------------------
all:  builddir $(TARGET)

builddir:
	mkdir -p $(BIN)

debug: DFLAGS := -ggdb -DDEBUG
debug: $(TARGET)

master: builddir $(BIN)/master
worker: builddir $(BIN)/worker
mapper: builddir $(BIN)/mapper 
reducer: builddir $(BIN)/reducer
	

$(BIN)/master: $(CMN_OBJS) $(MASTER_OBJS)
	@echo "$(YELLOW)Compile $@$(END)"
	$(CXX) $(CFLAGS) $(LFLAGS) -o $@ $^

$(BIN)/worker: $(CMN_OBJS) $(WORKER_OBJS)
	@echo "$(YELLOW)Compile $@$(END)"
	$(CXX) $(LFLAGS) -o $@ $^

$(BIN)/mapper: $(CMN_OBJS) $(MAPPER_OBJS)
	@echo "$(YELLOW)Compile $@$(END)"
	$(CXX) $(CFLAGS) $(LFLAGS) -o $@ $^

$(BIN)/reducer: $(CMN_OBJS) $(REDUCER_OBJS)
	@echo "$(YELLOW)Compile $@$(END)"
	$(CXX) $(CFLAGS) $(LFLAGS) -o $@ $^


$(WORKER_OBJS): $(WORKER_HDRS) $(CMN_OBJS)
	@echo "Compile $@"
	$(CXX) $(CFLAGS) $(LFLAGS) $(DFLAGS) -c $(WORKER_SRCS) $^

$(MASTER_OBJS): $(MASTER_HDRS) $(CMN_OBJS)
	@echo "Compile $@"
	$(CXX) $(CFLAGS) $(LFLAGS) $(DFLAGS) -c $(MASTER_SRCS) $^

$(MAPPER_OBJS): $(MAPPER_HDRS) $(CMN_OBJS)
	@echo "Compile $@"
	$(CXX) $(CFLAGS) $(LFLAGS) $(DFLAGS) -c $(MAPPER_SRCS) $^

$(REDUCER_OBJS): $(REDUCER_HDRS) $(CMN_OBJS)
	@echo "Compile $@"
	$(CXX) $(CFLAGS) $(LFLAGS) $(DFLAGS) -c $(REDUCER_SRCS) $^


# Build any shared dependencies
$(CMN_OBJS): $(CMN_SRCS) $(CMN_HDRS)
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(CMN_SRCS)

clean:
	rm -rf $(BIN) *.o

hello:
	@echo "hello"
	@echo "$(OBJS)"
	@echo "$(CMN_OBJS)"

.PHONY: clean hello
