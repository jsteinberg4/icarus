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
	mkdir -p $(BIN)

debug: DFLAGS := -ggdb -DDEBUG
debug: $(TARGET)

master: builddir $(BIN)/master
worker: builddir $(BIN)/worker
mapper: builddir $(BIN)/mapper 
reducer: builddir $(BIN)/reducer
	

$(BIN)/master: $(MASTER_OBJS) $(CMN_OBJS)
	@echo "$(YELLOW)Compile $@$(END)"
	$(CXX) $(LFLAGS) -o $@ $^

$(BIN)/worker: $(WORKER_OBJS) $(CMN_OBJS)
	@echo "$(YELLOW)Compile $@$(END)"
	$(CXX) $(LFLAGS) -o $@ $^

$(BIN)/mapper: $(MAPPER_OBJS) $(CMN_OBJS)
	@echo "$(YELLOW)Compile $@$(END)"
	$(CXX) $(LFLAGS) -o $@ $^

$(BIN)/reducer: $(REDUCER_OBJS) $(CMN_OBJS)
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

clean:
	rm -rf $(BIN) *.o

.PHONY: clean
