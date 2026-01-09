# Makefile for LegacyLib (Legacy Agent Library)
#
# Usage:
#   make                # Build for VxWorks DKM (Default)
#   make MODE=linux     # Build for Linux (Executable + Static Lib)
#   make config         # Show build configuration
#
# Prerequisites for VxWorks:
#   - Run vx_env.bat to setup environment
#   - WIND_BASE, WIND_CC_SYSROOT, VSB_DIR must be set
#   - wr-cc, wr-c++ in PATH
#
# Environment Variables (from vx_env.bat):
#   WIND_BASE         = D:\WindRiver\vxworks\23.03
#   WIND_CC_SYSROOT   = D:\WindRiver\workspace_test
#   VSB_DIR           = D:\WindRiver\workspace_test

MODE ?= vxworks

# --- Configuration ---

# Library Sources (C++)
LIB_SRC_CPP = src/internal/DkmRtpIpc.cpp \
              src/internal/IpcJsonClient.cpp \
              src/legacy_agent.cpp

# Demo Sources (relative to project root)
DEMO_SRC_C_LINUX = ../examples/main.c
DEMO_SRC_C_DKM   = ../examples/demo_dkm.c

# Objects
LIB_OBJ_CPP = $(LIB_SRC_CPP:.cpp=.o)
DEMO_OBJ_LINUX = $(DEMO_SRC_C_LINUX:.c=.o)
DEMO_OBJ_DKM = $(DEMO_SRC_C_DKM:.c=.o)

# --- VxWorks DKM Settings ---
ifeq ($(MODE),vxworks)
    # Check required environment variables
    ifndef WIND_BASE
        $(error WIND_BASE not set. Run vx_env.bat first)
    endif
    
    ifndef WIND_CC_SYSROOT
        $(error WIND_CC_SYSROOT not set. Run vx_env.bat first)
    endif
    
    VSB_DIR ?= $(WIND_CC_SYSROOT)
    
    CC        = wr-cc
    CXX       = wr-c++
    
    # -dkm flag for Downloadable Kernel Module
    # -mcpu=e6500: PowerPC e6500 (from default.conf)
    # -D_VX_CPU=_VX_PPCE6500: CPU definition
    # -D_WRS_KERNEL: Kernel mode
    COMMON_FLAGS = -dkm \
                   -mcpu=e6500 \
                   -D_VX_CPU=_VX_PPCE6500 \
                   -D_WRS_KERNEL \
                   -D_VXWORKS_ \
                   -Iinclude \
                   -Isrc/internal \
                   -Igenerated \
                   -Wall -O2 
				   
#				   -DDEMO_PERF_INSTRUMENTATION
    
    CFLAGS    = $(COMMON_FLAGS)
    CXXFLAGS  = $(COMMON_FLAGS) -std=c++11
    
    # Discovery of generated headers
    # Ensure generated code can find original structs
    CFLAGS    += -Itools/struct
    CXXFLAGS  += -Itools/struct

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
    
    CFLAGS    = -Iinclude -Isrc/internal -Igenerated -Wall -Wextra
    CXXFLAGS  = -Iinclude -Isrc/internal -Igenerated -Wall -Wextra -std=c++11
    
    TARGET_APP = example_app
    TARGET_LIB = liblegacy_agent.a
    
    TARGETS = $(TARGET_APP)
endif

# --- Rules ---

.PHONY: all clean config

all: check-env $(TARGETS)

# Check environment (VxWorks mode only)
check-env:
ifeq ($(MODE),vxworks)
	@echo "=== Checking VxWorks Environment ==="
	@if [ -z "$(WIND_BASE)" ]; then \
		echo "ERROR: WIND_BASE not set"; \
		echo "Please run vx_env.bat first"; \
		exit 1; \
	fi
	@echo "WIND_BASE: $(WIND_BASE)"
	@echo "VSB_DIR: $(VSB_DIR)"
	@echo "=== Environment OK ==="
endif

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
	@echo "Cleaning LegacyLib..."
	rm -f $(LIB_OBJ_CPP) $(DEMO_OBJ_LINUX) $(DEMO_OBJ_DKM)
	rm -f $(TARGET_APP) $(TARGET_LIB) liblegacy_agent_dkm.out demo_tcp_cli_dkm.out legacy_agent_dkm.out
	@echo "Clean complete"

# Show build configuration
config:
	@echo "=== LegacyLib Build Configuration ==="
	@echo "Mode: $(MODE)"
ifeq ($(MODE),vxworks)
	@echo "Environment:"
	@echo "  WIND_BASE: $(WIND_BASE)"
	@echo "  WIND_CC_SYSROOT: $(WIND_CC_SYSROOT)"
	@echo "  VSB_DIR: $(VSB_DIR)"
	@echo "Compiler:"
	@echo "  CC: $(CC)"
	@echo "  CXX: $(CXX)"
	@echo "Flags:"
	@echo "  CFLAGS: $(CFLAGS)"
	@echo "  CXXFLAGS: $(CXXFLAGS)"
	@echo "Targets:"
	@echo "  $(TARGETS)"
else
	@echo "Compiler: $(CC), $(CXX)"
	@echo "Targets: $(TARGETS)"
endif
	@echo "===================================="
	-del *.out 2>nul
