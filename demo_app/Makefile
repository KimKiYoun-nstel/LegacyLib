# DemoApp Makefile for VxWorks
# 
# Build DemoApp DKM module
# Requires: Wind River VxWorks development environment
#
# Prerequisites:
#   - Run vx_env.bat to setup VxWorks environment
#   - WIND_BASE, WIND_CC_SYSROOT, VSB_DIR must be set
#   - wr-cc, wr-c++ in PATH
#
# Usage:
#   make          # Build DemoApp DKM
#   make clean    # Clean build artifacts
#   make config   # Show build configuration

# --- Environment Variables (from vx_env.bat) ---
# These should be set by vx_env.bat:
#   WIND_BASE         = D:\WindRiver\vxworks\23.03
#   WIND_CC_SYSROOT   = D:\WindRiver\workspace_test
#   VSB_DIR           = D:\WindRiver\workspace_test
#   NDDSHOME_CTL      = D:\rti_connext_dds-7.3.1
#   LLVM_ROOT         = D:\WindRiver\compilers\llvm-15.0.0.1\WIN64

# Check required environment variables
ifndef WIND_BASE
$(error WIND_BASE not set. Run vx_env.bat first)
endif

ifndef WIND_CC_SYSROOT
$(error WIND_CC_SYSROOT not set. Run vx_env.bat first)
endif

# VSB Directory (VxWorks Source Build)
VSB_DIR ?= $(WIND_CC_SYSROOT)

# Target Architecture (from default.conf)
# PowerPC e6500 as specified in config
ARCH = ppc
CPU = PPCE6500
CPU_VARIANT = _ppc_PPCE6500

# Compiler and Tools (Wind River toolchain)
CC = wr-cc
CXX = wr-c++
LD = wr-ld
AR = wr-ar

# Include Paths
INCLUDES = -I./include \
           -I../include \
           -I$(VSB_DIR)/krnl/h/published/UTILS_UNIX \
           -I$(VSB_DIR)/share/h \
           -I$(VSB_DIR)/krnl/h/public

# Compiler Flags (based on default.conf for DKM)
# -dkm: Downloadable Kernel Module mode
# -mcpu=e6500: PowerPC e6500 CPU
# -D_VX_CPU=_VX_PPCE6500: CPU definition
CFLAGS = -dkm \
         -mcpu=e6500 \
         -D_VX_CPU=_VX_PPCE6500 \
         -D_WRS_KERNEL \
         -D_VXWORKS_ \
         -Wall -O2
		 
#		 -DDEMO_PERF_INSTRUMENTATION

# Source Files
SRCS = src/demo_app_core.c \
	src/demo_app_msg.cpp \
	src/demo_app_timer.c \
	src/demo_app_publisher.c \
	src/demo_app_status.c \
       src/demo_app_enums.c \
       src/demo_app_log.c \
       src/demo_app_cli.c \
       vxworks/demo_app_tcp_vx.c \
       vxworks/demo_app_dkm.c

# Object Files
OBJS = $(patsubst %.c,%.o,$(SRCS))
OBJS := $(patsubst %.cpp,%.o,$(OBJS))

# Output
TARGET = demo_app_dkm.out

# LegacyLib dependency
# Link with LegacyLib object files directly for DKM
LEGACY_OBJS = ../src/legacy_agent.o \
              ../src/internal/IpcJsonClient.o \
              ../src/internal/DkmRtpIpc.o

# Linker Flags for DKM
# -r: Relocatable output (partial link)
LDFLAGS = -dkm

# ========================================================================
# Build Rules
# ========================================================================

all: check-env $(TARGET)

# Check environment before building
check-env:
	@echo "=== Checking VxWorks Environment ==="
	@if [ -z "$(WIND_BASE)" ]; then \
		echo "ERROR: WIND_BASE not set"; \
		echo "Please run vx_env.bat first"; \
		exit 1; \
	fi
	@if [ -z "$(WIND_CC_SYSROOT)" ]; then \
		echo "ERROR: WIND_CC_SYSROOT not set"; \
		echo "Please run vx_env.bat first"; \
		exit 1; \
	fi
	@echo "WIND_BASE: $(WIND_BASE)"
	@echo "VSB_DIR: $(VSB_DIR)"
	@echo "=== Environment OK ==="

# Build LegacyLib objects first if needed
$(LEGACY_OBJS):
	@echo "Building LegacyLib dependencies..."
	$(MAKE) -C .. MODE=vxworks

$(TARGET): $(OBJS) $(LEGACY_OBJS)
	@echo "Linking DKM: $@"
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LEGACY_OBJS)
	@echo "=== Build complete: $@ ==="
	@echo "Load in VxWorks shell:"
	@echo "  -> ld < demo_app_dkm.out"
	@echo "  -> demoAppStart(23000, \"127.0.0.1\")"

%.o: %.c
	@echo "Compiling C $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.cpp
	@echo "Compiling C++ $<..."
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "Cleaning..."
	rm -f $(OBJS) $(TARGET)
	@echo "Clean complete"

rebuild: clean all

# ========================================================================
# Development Helpers
# ========================================================================

# Show configuration
config:
	@echo "=== DemoApp Build Configuration ==="
	@echo "Environment:"
	@echo "  WIND_BASE: $(WIND_BASE)"
	@echo "  WIND_CC_SYSROOT: $(WIND_CC_SYSROOT)"
	@echo "  VSB_DIR: $(VSB_DIR)"
	@echo ""
	@echo "Target:"
	@echo "  ARCH: $(ARCH)"
	@echo "  CPU: $(CPU)"
	@echo "  OUTPUT: $(TARGET)"
	@echo ""
	@echo "Sources:"
	@echo "  $(SRCS)"
	@echo ""
	@echo "LegacyLib Objects:"
	@echo "  $(LEGACY_OBJS)"
	@echo ""
	@echo "Compiler:"
	@echo "  CC: $(CC)"
	@echo "  CXX: $(CXX)"
	@echo "  CFLAGS: $(CFLAGS)"
	@echo "================================"

# Quick syntax check (without full build)
check:
	@echo "Syntax checking..."
	@for src in $(SRCS); do \
		echo "  Checking $$src..."; \
		$(CC) $(CFLAGS) $(INCLUDES) -fsyntax-only $$src; \
	done
	@echo "Syntax check complete"

.PHONY: all clean rebuild config check check-env
