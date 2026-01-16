# Makefile for LegacyLib (Legacy Agent Library)
#
# Usage:
#   make                # Build for VxWorks DKM (Default)
#   make MODE=native    # Build for Native/Host (Windows MinGW or Linux)
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

# Build directories (platform-specific to avoid cross-compilation conflicts)
BUILD_DIR_VXWORKS = build_vxworks
BUILD_DIR_NATIVE  = build_native

# Library Sources (C++)
LIB_SRC_CPP = src/internal/DkmRtpIpc.cpp \
              src/internal/IpcJsonClient.cpp \
              src/legacy_agent.cpp

# -----------------------------------------------------------------------------
# Data Transport 어댑터 (Control=UDP, Data=UDP 또는 SHM)
# -----------------------------------------------------------------------------
# 설계 문서: DevGuide/03_LegacyLib_SHM_Integration_Design.md
DATA_TRANSPORT_DIR = src/internal/data_transport
DATA_TRANSPORT_SRC_CPP = $(DATA_TRANSPORT_DIR)/data_transport_udp.cpp \
                         $(DATA_TRANSPORT_DIR)/data_transport_factory.cpp

# SHM 어댑터는 VxWorks에서만 추가 (아래 VxWorks 섹션에서 처리)

# -----------------------------------------------------------------------------
# TransportSHM 공용모듈 (VxWorks 전용)
# -----------------------------------------------------------------------------
# 설계 문서: DevGuide/01_TransportSHM_Design.md
#            DevGuide/03_LegacyLib_SHM_Integration_Design.md
#
# VxWorks에서만 활성화되며, 타 플랫폼에서는 제외된다.
SHM_DIR = common/transport_shm
SHM_SRC_CPP = $(SHM_DIR)/src/shm_transport.cpp \
              $(SHM_DIR)/src/shm_region_posix.cpp \
              $(SHM_DIR)/src/shm_notify_posix_sem.cpp \
              $(SHM_DIR)/src/shm_ring_spsc.cpp

# Demo Sources (relative to project root)
DEMO_SRC_C_NATIVE = ../examples/main.c
DEMO_SRC_C_DKM    = ../examples/demo_dkm.c

# --- VxWorks DKM Settings ---
ifeq ($(MODE),vxworks)
    BUILD_DIR = $(BUILD_DIR_VXWORKS)
    
    # Data Transport 어댑터 추가 (UDP + SHM)
    LIB_SRC_CPP += $(DATA_TRANSPORT_SRC_CPP)
    LIB_SRC_CPP += $(DATA_TRANSPORT_DIR)/data_transport_shm.cpp
    
    # TransportSHM 공용모듈 소스 추가 (VxWorks 전용)
    LIB_SRC_CPP += $(SHM_SRC_CPP)
    
    # Objects in build directory
    LIB_OBJ_CPP = $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(filter src/%.cpp,$(LIB_SRC_CPP))) \
                  $(patsubst $(SHM_DIR)/src/%.cpp,$(BUILD_DIR)/shm/%.o,$(filter $(SHM_DIR)/src/%.cpp,$(LIB_SRC_CPP)))
    DEMO_OBJ_DKM = $(patsubst ../examples/%.c,$(BUILD_DIR)/%.o,$(DEMO_SRC_C_DKM))
    
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
                   -DDDSFW_ENABLE_SHM=1 \
                   -DDDSFW_TRANSPORT_SHM_VXWORKS=1 \
                   -Iinclude \
                   -Isrc/internal \
                   -Igenerated \
                   -I$(SHM_DIR)/include \
                   -I$(SHM_DIR)/src \
                   -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter -O2 
				   
#				   -DDEMO_PERF_INSTRUMENTATION
    
    CFLAGS    = $(COMMON_FLAGS)
    CXXFLAGS  = $(COMMON_FLAGS) -std=c++17
    
    # Discovery of generated headers
    # Ensure generated code can find original structs
    CFLAGS    += -Itools/struct
    CXXFLAGS  += -Itools/struct

    # Two targets for VxWorks
    TARGET_LIB  = liblegacy_agent_dkm.out
    TARGET_DEMO = demo_tcp_cli_dkm.out
    
    TARGETS = $(TARGET_LIB) $(TARGET_DEMO)
endif

# --- Native/Host Settings (Windows MinGW / Linux) ---
ifeq ($(MODE),native)
    BUILD_DIR = $(BUILD_DIR_NATIVE)
    
    # Data Transport 어댑터 추가 (UDP만, SHM 제외)
    LIB_SRC_CPP += $(DATA_TRANSPORT_SRC_CPP)
    
    # Objects in build directory
    LIB_OBJ_CPP = $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(LIB_SRC_CPP))
    DEMO_OBJ_NATIVE = $(patsubst ../examples/%.c,$(BUILD_DIR)/%.o,$(DEMO_SRC_C_NATIVE))
    
    CC        = gcc
    CXX       = g++
    AR        = ar
    
    CFLAGS    = -Iinclude -Isrc/internal -Igenerated -Itools/struct -Wall -Wextra -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter -DDDSFW_ENABLE_SHM=0
    CXXFLAGS  = -Iinclude -Isrc/internal -Igenerated -Itools/struct -Wall -Wextra -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter -std=c++11 -DDDSFW_ENABLE_SHM=0
    
    TARGET_APP = example_app
    TARGET_LIB = liblegacy_agent.a
    
    TARGETS = $(TARGET_APP)
endif

# --- Rules ---

.PHONY: all clean config

all: check-env $(BUILD_DIR) $(TARGETS)

# Create build directory
$(BUILD_DIR):
	@echo "Creating build directory: $(BUILD_DIR)"
ifeq ($(MODE),vxworks)
	@mkdir -p $(BUILD_DIR)/internal
	@mkdir -p $(BUILD_DIR)/internal/data_transport
	@mkdir -p $(BUILD_DIR)/shm
else
  ifeq ($(OS),Windows_NT)
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR) 2>nul
	@if not exist $(BUILD_DIR)\internal mkdir $(BUILD_DIR)\internal 2>nul
	@if not exist $(BUILD_DIR)\internal\data_transport mkdir $(BUILD_DIR)\internal\data_transport 2>nul
  else
	@mkdir -p $(BUILD_DIR)/internal
	@mkdir -p $(BUILD_DIR)/internal/data_transport
  endif
endif

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
$(TARGET_APP): $(DEMO_OBJ_NATIVE) $(TARGET_LIB)
	@echo "Building Native App: $@"
ifeq ($(OS),Windows_NT)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lpthread -lws2_32
else
	$(CXX) $(CXXFLAGS) -o $@ $^ -lpthread
endif
endif

# Compilation rules with build directory
$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/internal/%.o: src/internal/%.cpp | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Data Transport 어댑터 컴파일 규칙
$(BUILD_DIR)/internal/data_transport/%.o: src/internal/data_transport/%.cpp | $(BUILD_DIR)
	@echo "Compiling DataTransport: $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# TransportSHM 소스 컴파일 규칙 (VxWorks 전용)
$(BUILD_DIR)/shm/%.o: $(SHM_DIR)/src/%.cpp | $(BUILD_DIR)
	@echo "Compiling SHM: $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: ../examples/%.c | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning LegacyLib (MODE=$(MODE))..."
ifeq ($(MODE),vxworks)
	rm -rf $(BUILD_DIR)
	rm -f liblegacy_agent_dkm.out demo_tcp_cli_dkm.out
else
  ifeq ($(OS),Windows_NT)
	-@if exist $(BUILD_DIR) rd /S /Q $(BUILD_DIR) 2>nul
	-@if exist example_app del /Q example_app 2>nul
	-@if exist liblegacy_agent.a del /Q liblegacy_agent.a 2>nul
	-@if exist liblegacy_agent_dkm.out del /Q liblegacy_agent_dkm.out 2>nul
	-@if exist demo_tcp_cli_dkm.out del /Q demo_tcp_cli_dkm.out 2>nul
  else
	rm -rf $(BUILD_DIR)
	rm -f example_app liblegacy_agent.a liblegacy_agent_dkm.out demo_tcp_cli_dkm.out
  endif
endif
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
