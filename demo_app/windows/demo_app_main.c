/*
 * demo_app_main.c - Windows Console Entry Point
 * 
 * TCP CLI-based Windows executable for DemoApp
 * 
 * Usage:
 *   demo_app.exe [-cli_port 23000] [-log_port 24000] [-agent_host 127.0.0.1] [-agent_port 25000]
 *   
 *   Then connect with:
 *     telnet localhost 23000  (for CLI commands)
 *     telnet localhost 24000  (for log output)
 */

#include "../include/demo_app.h"
#include "../include/demo_app_log.h"
#include "../include/demo_app_tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

/* ========================================================================
 * Global Context
 * ======================================================================== */

DemoAppContext* g_demo_ctx = NULL;
static volatile int g_running = 1;
int g_cli_port = 23000;
static int g_log_port = 24000;
char g_agent_ip[64] = "127.0.0.1";
static int g_agent_port = 25000;

/* ========================================================================
 * Signal Handler
 * ======================================================================== */

#ifdef _WIN32
BOOL WINAPI consoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
        LOG_INFO("\n[DemoApp Main] Shutting down...\n");
        g_running = 0;
        return TRUE;
    }
    return FALSE;
}
#else
void signalHandler(int sig) {
    (void)sig;
    LOG_INFO("\n[DemoApp Main] Shutting down...\n");
    g_running = 0;
}
#endif

/* ========================================================================
 * Help
 * ======================================================================== */

void print_usage(const char* prog) {
    LOG_INFO("Usage: %s [options]\n", prog);
    LOG_INFO("Options:\n");
    LOG_INFO("  -cli_port <port>    TCP CLI server port (default: 23000)\n");
    LOG_INFO("  -log_port <port>    TCP Log server port (default: 24000)\n");
    LOG_INFO("  -agent_host <host>  DDS Agent host (default: 127.0.0.1)\n");
    LOG_INFO("  -agent_port <port>  DDS Agent port (default: 25000)\n");
    LOG_INFO("  -log_mode <mode>    Log output mode: console|redirect|both (default: both)\n");
    LOG_INFO("  -help               Show this help\n");
    LOG_INFO("\nExample:\n");
    LOG_INFO("  %s -cli_port 23000 -log_port 24000 -agent_host 127.0.0.1\n", prog);
    LOG_INFO("\nConnect to CLI:\n");
    LOG_INFO("  telnet localhost 23000\n");
    LOG_INFO("\nConnect to Log:\n");
    LOG_INFO("  telnet localhost 24000\n");
}

/* ========================================================================
 * Main Entry Point
 * ======================================================================== */

int main(int argc, char* argv[]) {
    LogOutputMode log_mode = LOG_MODE_BOTH;  // Default: console + redirect
    
    // Initialize Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        LOG_ERROR("ERROR: WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }
    
    LOG_INFO("============================================\n");
    LOG_INFO("  DemoApp - Windows TCP CLI Version\n");
    LOG_INFO("============================================\n\n");
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-cli_port") == 0 && i + 1 < argc) {
            g_cli_port = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-log_port") == 0 && i + 1 < argc) {
            g_log_port = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-agent_host") == 0 && i + 1 < argc) {
            strncpy(g_agent_ip, argv[++i], sizeof(g_agent_ip) - 1);
            g_agent_ip[sizeof(g_agent_ip) - 1] = '\0';
        }
        else if (strcmp(argv[i], "-agent_port") == 0 && i + 1 < argc) {
            g_agent_port = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-log_mode") == 0 && i + 1 < argc) {
            const char* mode = argv[++i];
            if (strcmp(mode, "console") == 0) {
                log_mode = LOG_MODE_CONSOLE;
            } else if (strcmp(mode, "redirect") == 0) {
                log_mode = LOG_MODE_REDIRECT;
            } else if (strcmp(mode, "both") == 0) {
                log_mode = LOG_MODE_BOTH;
            } else {
                LOG_ERROR("ERROR: Invalid log_mode '%s'\n", mode);
                print_usage(argv[0]);
                return 1;
            }
        }
        else if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        else {
            LOG_ERROR("ERROR: Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Setup signal handler
#ifdef _WIN32
    SetConsoleCtrlHandler(consoleHandler, TRUE);
#else
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
#endif
    
    // Initialize log system
    LOG_INFO("Initializing log system...\n");
    if (demo_log_init(log_mode) != 0) {
        LOG_ERROR("ERROR: Failed to initialize log system\n");
        return 1;
    }
    LOG_INFO("Log system initialized (mode: %s)\n",
           log_mode == LOG_MODE_CONSOLE ? "console" :
           log_mode == LOG_MODE_REDIRECT ? "redirect" : "both");
    
    // Start TCP Log Server (port 24000)
    LOG_INFO("Starting TCP Log Server on port %d...\n", g_log_port);
    if (demo_tcp_log_start(g_log_port) != 0) {
        LOG_ERROR("ERROR: Failed to start TCP Log Server\n");
        demo_log_cleanup();
        return 1;
    }
    LOG_INFO("TCP Log Server started on port %d\n", g_log_port);
    
    // Start TCP CLI Server (port 23000)
    LOG_INFO("Starting TCP CLI Server on port %d...\n", g_cli_port);
    if (demo_tcp_cli_start(g_cli_port) != 0) {
        LOG_ERROR("ERROR: Failed to start TCP CLI Server\n");
        demo_tcp_log_stop();
        demo_log_cleanup();
        return 1;
    }
    LOG_INFO("TCP CLI Server started on port %d\n", g_cli_port);
    
    // Allocate context
    g_demo_ctx = (DemoAppContext*)malloc(sizeof(DemoAppContext));
    if (!g_demo_ctx) {
        LOG_ERROR("ERROR: Failed to allocate context\n");
        demo_tcp_cli_stop();
        demo_tcp_log_stop();
        demo_log_cleanup();
        return 1;
    }
    
    // Initialize context
    demo_app_context_init(g_demo_ctx);
    
    LOG_INFO("\n========================================\n");
    LOG_INFO("DemoApp started successfully!\n");
    LOG_INFO("----------------------------------------\n");
    LOG_INFO("CLI Port:   %d\n", g_cli_port);
    LOG_INFO("Log Port:   %d\n", g_log_port);
    LOG_INFO("Agent:      %s:%d\n", g_agent_ip, g_agent_port);
    LOG_INFO("Log Mode:   %s\n",
           log_mode == LOG_MODE_CONSOLE ? "console" :
           log_mode == LOG_MODE_REDIRECT ? "redirect" : "both");
    LOG_INFO("----------------------------------------\n");
    LOG_INFO("Connect to CLI: telnet localhost %d\n", g_cli_port);
    LOG_INFO("Connect to Log: telnet localhost %d\n", g_log_port);
    printf("Press Ctrl+C to exit\n");
    printf("========================================\n\n");
    
    LOG_INFO("DemoApp Windows TCP CLI version started\n");
    LOG_INFO("Waiting for CLI commands...\n");
    
    // Main loop - just wait for Ctrl+C
    while (g_running) {
#ifdef _WIN32
        Sleep(1000);  // 1 second
#else
        sleep(1);
#endif
    }
    
    // Cleanup
    printf("\n[Main] Shutting down...\n");
    
    if (g_demo_ctx) {
        if (g_demo_ctx->current_state != DEMO_STATE_IDLE) {
            demo_app_reset(g_demo_ctx);
        }
        free(g_demo_ctx);
        g_demo_ctx = NULL;
    }
    
    demo_tcp_cli_stop();
    demo_tcp_log_stop();
    demo_log_cleanup();
    
    WSACleanup();
    printf("[Main] Shutdown complete\n");
    return 0;
}
