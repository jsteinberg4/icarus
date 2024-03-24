#!/usr/bin/make -f

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
# TODO: Direct compiler output
BUILD_DIR := build/

CMN_HDRS := $(wildcard $(CMN_DIR)/*.h)
CMN_SRCS := $(wildcard $(CMN_DIR)/*.cpp)
CMN_OBJS := $(CMN_SRCS:.cpp=.o)

# TODO: Objs for master, worker, mapper, reducer

TARGET := master worker mapper reducer
#-------------------------------------------------------------------------------
# Build Rules
#-------------------------------------------------------------------------------
all: $(TARGET)

debug: DFLAGS := -ggdb -DDEBUG
debug: $(TARGET)

master: $(CMN_OBJS)
	@echo "TODO: Master build rule"

worker: $(CMN_OBJS)
	@echo "TODO: Worker build rule"

mapper: $(CMN_OBJS)
	@echo "TODO: Mapper build rule"

reducer: $(CMN_OBJS)
	@echo "TODO: Reducer build rule"

# Build any shared dependencies
$(CMN_OBJS): $(CMN_SRCS) $(CMN_HDRS)
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(CMN_SRCS)

clean:
	rm -rf *.o $(TARGET)

test:
	@echo "TODO: Test build rule"

.PHONY: clean
