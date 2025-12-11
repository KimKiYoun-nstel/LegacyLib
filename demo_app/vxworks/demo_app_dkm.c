/*
 * demo_app_dkm.c - VxWorks DKM Entry Point
 * 
 * VxWorks Shell Commands:
 *   -> ld < demo_app_dkm.out
 *   -> demoAppStart(23000, "127.0.0.1")
 *   -> demoAppStatus()
 *   -> demoAppStop()
 *   -> unld "demo_app_dkm.out"
 */

#include <vxWorks.h>
#include <taskLib.h>
#include <sysLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/demo_app.h"

/* ========================================================================
 * External Functions
 * ======================================================================== */

// From demo_app_cli.c
extern int demo_cli_start(int port);
extern void demo_cli_stop(void);
extern void demo_cli_print(const char* fmt, ...);

/* ========================================================================
 * Global Context
 * ======================================================================== */

DemoAppContext* g_demo_ctx = NULL;
static int g_cli_port = 23000;
static char g_agent_ip[64] = "127.0.0.1";

/* ========================================================================
 * VxWorks Shell Commands
 * ======================================================================== */

/**
 * Start DemoApp
 * 
 * @param cli_port    TCP CLI server port (default: 23000)
 * @param agent_ip    DDS Agent IP address (default: "127.0.0.1")
 * @return OK on success, ERROR on failure
 */
STATUS demoAppStart(int cli_port, const char* agent_ip) {
    if (g_demo_ctx) {
        printf("[DemoApp DKM] Already running\n");
        return ERROR;
    }
    
    printf("[DemoApp DKM] Starting...\n");
    
    // Save configuration
    g_cli_port = cli_port;
    if (agent_ip && *agent_ip) {
        strncpy(g_agent_ip, agent_ip, sizeof(g_agent_ip) - 1);
        g_agent_ip[sizeof(g_agent_ip) - 1] = '\0';
    }
    
    // Allocate context
    g_demo_ctx = (DemoAppContext*)malloc(sizeof(DemoAppContext));
    if (!g_demo_ctx) {
        printf("[DemoApp DKM] Failed to allocate context\n");
        return ERROR;
    }
    
    // Initialize context
    demo_app_context_init(g_demo_ctx);
    
    // Start CLI server
    if (demo_cli_start(g_cli_port) != 0) {
        printf("[DemoApp DKM] Failed to start CLI server\n");
        free(g_demo_ctx);
        g_demo_ctx = NULL;
        return ERROR;
    }
    
    printf("[DemoApp DKM] Started successfully\n");
    printf("[DemoApp DKM] CLI Port: %d\n", g_cli_port);
    printf("[DemoApp DKM] Agent IP: %s\n", g_agent_ip);
    printf("[DemoApp DKM] Connect with: telnet <target_ip> %d\n", g_cli_port);
    printf("[DemoApp DKM] Use 'demo_init' command to start demo\n");
    
    return OK;
}

/**
 * Stop DemoApp
 * 
 * @return OK on success, ERROR on failure
 */
STATUS demoAppStop(void) {
    if (!g_demo_ctx) {
        printf("[DemoApp DKM] Not running\n");
        return ERROR;
    }
    
    printf("[DemoApp DKM] Stopping...\n");
    
    // Stop demo application
    demo_app_stop(g_demo_ctx);
    
    // Stop CLI server
    demo_cli_stop();
    
    // Free context
    free(g_demo_ctx);
    g_demo_ctx = NULL;
    
    printf("[DemoApp DKM] Stopped\n");
    return OK;
}

/**
 * Show DemoApp status
 */
void demoAppStatus(void) {
    if (!g_demo_ctx) {
        printf("[DemoApp DKM] Not running\n");
        printf("Use: demoAppStart(23000, \"127.0.0.1\")\n");
        return;
    }
    
    printf("\n=== DemoApp Status ===\n");
    printf("State: %s\n", demo_state_name(g_demo_ctx->current_state));
    printf("CLI Port: %d\n", g_cli_port);
    printf("Agent IP: %s\n", g_agent_ip);
    printf("Tick Count: %llu\n", g_demo_ctx->tick_count);
    printf("\nStatistics:\n");
    printf("  Signal Published: %u\n", g_demo_ctx->signal_pub_count);
    printf("  CBIT Published: %u\n", g_demo_ctx->cbit_pub_count);
    printf("  Control Received: %u\n", g_demo_ctx->control_rx_count);
    printf("  Speed Received: %u\n", g_demo_ctx->speed_rx_count);
    printf("\nBIT State:\n");
    printf("  PBIT Completed: %s\n", g_demo_ctx->bit_state.pbit_completed ? "Yes" : "No");
    printf("  CBIT Active: %s\n", g_demo_ctx->bit_state.cbit_active ? "Yes" : "No");
    printf("  IBIT Running: %s\n", g_demo_ctx->bit_state.ibit_running ? "Yes" : "No");
    printf("\nComponent Status:\n");
    printf("  Round Motor: %s\n", g_demo_ctx->bit_state.pbit_components.roundMotor ? "OK" : "FAULT");
    printf("  UpDown Motor: %s\n", g_demo_ctx->bit_state.pbit_components.upDownMotor ? "OK" : "FAULT");
    printf("  Base Gyro: %s\n", g_demo_ctx->bit_state.pbit_components.baseGyro ? "OK" : "FAULT");
    printf("  Power Controller: %s\n", g_demo_ctx->bit_state.pbit_components.powerController ? "OK" : "FAULT");
    printf("======================\n");
}
