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
#include "../include/demo_app_log.h"

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
    printf("[DemoApp DKM] Use shell commands:\n");
    printf("[DemoApp DKM]   demoAppConnect()        - Connect to Agent\n");
    printf("[DemoApp DKM]   demoAppReset()           - Full reset (cleanup, Idle)\n");
    printf("[DemoApp DKM]   demoAppCreateEntities() - Create DDS entities\n");
    printf("[DemoApp DKM]   demoAppStartScenario()  - Start simulation\n");
    printf("[DemoApp DKM]   demoAppStopScenario()   - Stop simulation\n");
    printf("[DemoApp DKM]   demoAppTestWrite(\"topic\") - 1-shot write test\n");
    printf("[DemoApp DKM]   demoAppLogMode(\"mode\")   - Set log output mode\n");
    printf("[DemoApp DKM]   demoAppStatus()         - Show status\n");
    printf("[DemoApp DKM]   demoAppStop()           - Shutdown\n");
    
    return OK;
}

/**
 * Connect to Agent (send hello + clear)
 * 
 * @return OK on success, ERROR on failure
 */
STATUS demoAppConnect(void) {
    if (!g_demo_ctx) {
        printf("[DemoApp DKM] Not initialized. Run demoAppStart() first\n");
        return ERROR;
    }
    
    // Connect to Agent
    if (demo_app_start(g_demo_ctx, g_agent_ip, 23000) != 0) {
        printf("[DemoApp DKM] Failed to connect to Agent\n");
        return ERROR;
    }
    
    printf("[DemoApp DKM] Connected to Agent successfully\n");
    return OK;
}

/**
 * Disconnect from Agent and reset state to Idle
 * 
 * @return OK on success, ERROR on failure
 */
STATUS demoAppDisconnect(void) {
    printf("[DemoApp DKM] demoAppDisconnect() is removed. Use demoAppReset() instead.\n");
    return ERROR;
}

/**
 * Create DDS entities
 * 
 * @return OK on success, ERROR on failure
 */
STATUS demoAppCreateEntities(void) {
    if (!g_demo_ctx) {
        printf("[DemoApp DKM] Not initialized. Run demoAppStart() first\n");
        return ERROR;
    }
    
    if (demo_app_create_entities(g_demo_ctx) != 0) {
        printf("[DemoApp DKM] Failed to create DDS entities\n");
        return ERROR;
    }
    
    printf("[DemoApp DKM] DDS entities created successfully\n");
    return OK;
}

/**
 * Start scenario (PBIT + periodic publishing)
 * 
 * @return OK on success, ERROR on failure
 */
STATUS demoAppStartScenario(void) {
    if (!g_demo_ctx) {
        printf("[DemoApp DKM] Not initialized. Run demoAppStart() first\n");
        return ERROR;
    }
    
    if (demo_app_start_scenario(g_demo_ctx) != 0) {
        printf("[DemoApp DKM] Failed to start scenario\n");
        return ERROR;
    }
    
    printf("[DemoApp DKM] Scenario started successfully\n");
    return OK;
}

/**
 * Stop scenario (stop periodic publishing)
 * 
 * @return OK on success, ERROR on failure
 */
STATUS demoAppStopScenario(void) {
    if (!g_demo_ctx) {
        printf("[DemoApp DKM] Not initialized\n");
        return ERROR;
    }
    
    demo_app_stop(g_demo_ctx);
    printf("[DemoApp DKM] Scenario paused (state=PEND)\n");
    return OK;
}

/**
 * Full reset (cleanup entities and set Idle)
 */
STATUS demoAppReset(void) {
    if (!g_demo_ctx) {
        printf("[DemoApp DKM] Not initialized\n");
        return ERROR;
    }

    if (demo_app_reset(g_demo_ctx) == 0) {
        printf("[DemoApp DKM] Reset complete, state=Idle\n");
        return OK;
    }
    printf("[DemoApp DKM] Reset failed\n");
    return ERROR;
}

/**
 * Test write - send a single message (no scenario needed)
 * 
 * @param topic  Topic name: "pbit", "cbit", "result_bit", "signal", or "all"
 * @return OK on success, ERROR on failure
 */
STATUS demoAppTestWrite(const char* topic) {
    if (!g_demo_ctx) {
        printf("[DemoApp DKM] Not initialized\n");
        return ERROR;
    }
    
    if (g_demo_ctx->current_state != DEMO_STATE_RUN && 
        g_demo_ctx->current_state != DEMO_STATE_POWERON_BIT) {
        printf("[DemoApp DKM] ERROR: Must create entities first\n");
        return ERROR;
    }
    
    if (!topic || !*topic) {
        printf("[DemoApp DKM] Usage: demoAppTestWrite(\"pbit|cbit|result_bit|signal|all\")\n");
        return ERROR;
    }
    
    int result = 0;
    if (strcmp(topic, "pbit") == 0) {
        result = demo_msg_test_write_pbit(g_demo_ctx);
        printf("[DemoApp DKM] Test write: PBIT %s\n", result == 0 ? "sent" : "failed");
    } else if (strcmp(topic, "cbit") == 0) {
        result = demo_msg_test_write_cbit(g_demo_ctx);
        printf("[DemoApp DKM] Test write: CBIT %s\n", result == 0 ? "sent" : "failed");
    } else if (strcmp(topic, "result_bit") == 0) {
        result = demo_msg_test_write_result_bit(g_demo_ctx);
        printf("[DemoApp DKM] Test write: resultBIT %s\n", result == 0 ? "sent" : "failed");
    } else if (strcmp(topic, "signal") == 0) {
        result = demo_msg_test_write_signal(g_demo_ctx);
        printf("[DemoApp DKM] Test write: Actuator Signal %s\n", result == 0 ? "sent" : "failed");
    } else if (strcmp(topic, "all") == 0) {
        int r1 = demo_msg_test_write_pbit(g_demo_ctx);
        taskDelay(sysClkRateGet() / 10);  // 100ms delay
        int r2 = demo_msg_test_write_cbit(g_demo_ctx);
        taskDelay(sysClkRateGet() / 10);
        int r3 = demo_msg_test_write_result_bit(g_demo_ctx);
        taskDelay(sysClkRateGet() / 10);
        int r4 = demo_msg_test_write_signal(g_demo_ctx);
        result = (r1 | r2 | r3 | r4);
        printf("[DemoApp DKM] Test write: All 4 messages %s\n", result == 0 ? "sent" : "failed");
    } else {
        printf("[DemoApp DKM] ERROR: Unknown topic '%s'\n", topic);
        printf("[DemoApp DKM] Valid topics: pbit, cbit, result_bit, signal, all\n");
        return ERROR;
    }
    
    return (result == 0) ? OK : ERROR;
}

/**
 * Set log output mode
 * 
 * @param mode  "console", "redirect", or "both"
 * @return OK on success, ERROR on failure
 */
STATUS demoAppLogMode(const char* mode) {
    if (!mode || !*mode) {
        printf("[DemoApp DKM] Usage: demoAppLogMode(\"console|redirect|both\")\n");
        return ERROR;
    }
    
    LogOutputMode log_mode;
    if (strcmp(mode, "console") == 0) {
        log_mode = LOG_MODE_CONSOLE;
    } else if (strcmp(mode, "redirect") == 0) {
        log_mode = LOG_MODE_REDIRECT;
    } else if (strcmp(mode, "both") == 0) {
        log_mode = LOG_MODE_BOTH;
    } else {
        printf("[DemoApp DKM] ERROR: Invalid mode '%s'\n", mode);
        printf("[DemoApp DKM] Valid modes: console, redirect, both\n");
        return ERROR;
    }
    
    demo_log_set_mode(log_mode);
    printf("[DemoApp DKM] Log mode set to: %s\n", mode);
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
    
    // Full reset/demo shutdown
    demo_app_reset(g_demo_ctx);
    
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
    printf("  Round Motor: %s\n", g_demo_ctx->bit_state.pbit_components.roundMotor == L_BITResultType_NORMAL ? "OK" : "FAULT");
    printf("  UpDown Motor: %s\n", g_demo_ctx->bit_state.pbit_components.upDownMotor == L_BITResultType_NORMAL ? "OK" : "FAULT");
    printf("  Base Giro: %s\n", g_demo_ctx->bit_state.pbit_components.baseGiro == L_BITResultType_NORMAL ? "OK" : "FAULT");
    printf("  Power Controller: %s\n", g_demo_ctx->bit_state.pbit_components.powerController == L_BITResultType_NORMAL ? "OK" : "FAULT");
    printf("======================\n");
}
