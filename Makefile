# Makefile for Legacy Agent
#
# Usage:
#   make                # Build for VxWorks DKM (Default)
#   make MODE=linux     # Build for Linux (Executable + Static Lib)
#
# Prerequisites for VxWorks:
#   - WIND_BASE, WIND_CC_SYSROOT set
#   - wr-cc, wr-c++ in PATH

MODE ?= vxworks

# --- Configuration ---

# Library Sources (C++)
LIB_SRC_CPP = src/internal/DkmRtpIpc.cpp \
              src/internal/IpcJsonClient.cpp \
              src/legacy_agent.cpp

# Demo Sources
DEMO_SRC_C_LINUX = examples/main.c
DEMO_SRC_C_DKM   = examples/demo_dkm.c

# Objects
LIB_OBJ_CPP = $(LIB_SRC_CPP:.cpp=.o)
DEMO_OBJ_LINUX = $(DEMO_SRC_C_LINUX:.c=.o)
DEMO_OBJ_DKM = $(DEMO_SRC_C_DKM:.c=.o)

# --- VxWorks DKM Settings ---
ifeq ($(MODE),vxworks)
    CC        = wr-cc
    CXX       = wr-c++
    
    # -dkm flag for Downloadable Kernel Module
    # -D_VXWORKS_ for code conditional compilation
    COMMON_FLAGS = -dkm -D_VXWORKS_ -Iinclude -Isrc/internal -Wall
    
    CFLAGS    = $(COMMON_FLAGS)
    CXXFLAGS  = $(COMMON_FLAGS) -std=c++11
    
    # Two targets for VxWorks
    TARGET_LIB  = liblegacy_agent_dkm.out
    TARGET_DEMO = demo_tcp_cli_dkm.out
    
    TARGETS = $(TARGET_LIB) $(TARGET_DEMO)
endif

# --- Linux/Generic Settings ---
ifeq ($(MODE),linux)
    CC        = gcc
    CXX       = g++
    AR        = ar
    
    CFLAGS    = -Iinclude -Isrc/internal -Wall -Wextra
    CXXFLAGS  = -Iinclude -Isrc/internal -Wall -Wextra -std=c++11
    
    TARGET_APP = example_app
    TARGET_LIB = liblegacy_agent.a
    
    TARGETS = $(TARGET_APP)
endif

# --- Rules ---

.PHONY: all clean

all: $(TARGETS)

ifeq ($(MODE),vxworks)
# VxWorks Library DKM (no main, relocatable)
$(TARGET_LIB): $(LIB_OBJ_CPP)
	@echo "Building VxWorks Library DKM: $@"
	$(CXX) $(CXXFLAGS) -r $(LIB_OBJ_CPP) -o $(TARGET_LIB)

# VxWorks Demo DKM (uses library object files)
$(TARGET_DEMO): $(DEMO_OBJ_DKM) $(LIB_OBJ_CPP)
	@echo "Building VxWorks Demo DKM: $@"
	$(CXX) $(CXXFLAGS) $(DEMO_OBJ_DKM) $(LIB_OBJ_CPP) -o $(TARGET_DEMO)
else
# Linux Static Library
$(TARGET_LIB): $(LIB_OBJ_CPP)
	@echo "Building Static Library: $@"
	$(AR) rcs $@ $^

# Linux Application
$(TARGET_APP): $(DEMO_OBJ_LINUX) $(TARGET_LIB)
	@echo "Building Linux App: $@"
	$(CXX) $(CXXFLAGS) -o $@ $^ -lpthread
endif

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning..."
	rm -f $(LIB_OBJ_CPP) $(DEMO_OBJ_LINUX) $(DEMO_OBJ_DKM)
	rm -f $(TARGET_APP) $(TARGET_LIB) liblegacy_agent_dkm.out demo_tcp_cli_dkm.out legacy_agent_dkm.out
	-del *.out 2>nul
