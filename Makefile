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
BIN := bin

# Shared code
CMN_HDRS := $(wildcard $(CMN_DIR)/*.h)
CMN_SRCS := $(wildcard $(CMN_DIR)/*.cpp)
CMN_OBJS := $(patsubst $(CMN_DIR)/%.cpp,%.o,$(CMN_SRCS))

# TODO: Objs for master, worker, mapper, reducer
HDRS := $(wildcard $(BASEDIR)/*.h)
SRCS := $(wildcard $(BASEDIR)/*.cpp)
# Flattens SRCS into the top level folder
OBJS := $(patsubst $(BASEDIR)/%.cpp,%.o,$(SRCS)) 

TARGET := master worker mapper reducer
#-------------------------------------------------------------------------------
# Build Rules
#-------------------------------------------------------------------------------
all:  builddir $(TARGET)

builddir:
	mkdir -p $(BIN)

debug: DFLAGS := -ggdb -DDEBUG
debug: $(TARGET)

master: $(CMN_OBJS)
	@echo "TODO: $@ build rule"

worker: $(BIN)/worker

$(BIN)/worker: $(CMN_OBJS) $(OBJS)
	@echo "Compile $@"
	$(CXX) $(LFLAGS) -o $@ $^

mapper: $(CMN_OBJS)
	@echo "TODO: $@ build rule"

reducer: $(CMN_OBJS)
	@echo "TODO: $@ build rule"

$(OBJS): $(SRCS) $(HDRS)
	@echo "Compile $@"
	$(CXX) $(CFLAGS) $(DFLAGS) -c $(SRCS)


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
