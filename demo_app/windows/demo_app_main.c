/*
 * demo_app_main.c - Windows Console Entry Point
 * 
 * Standalone Windows executable for DemoApp testing
 */

#include "../include/demo_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>  // For _kbhit(), _getch()
#else
#include <unistd.h>
#include <signal.h>
#endif

/* ========================================================================
 * Global Context
 * ======================================================================== */

DemoAppContext* g_demo_ctx = NULL;
static volatile int g_running = 1;

/* ========================================================================
 * Signal Handler
 * ======================================================================== */

#ifdef _WIN32
BOOL WINAPI consoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
        printf("\n[DemoApp Main] Shutting down...\n");
        g_running = 0;
        return TRUE;
    }
    return FALSE;
}
#else
void signalHandler(int sig) {
    (void)sig;
    printf("\n[DemoApp Main] Shutting down...\n");
    g_running = 0;
}
#endif

/* ========================================================================
 * Help
 * ======================================================================== */

void print_usage(const char* prog) {
    printf("Usage: %s [options]\n", prog);
    printf("Options:\n");
    printf("  -p <port>     Agent port (default: 23000)\n");
    printf("  -h <host>     Agent host (default: 127.0.0.1)\n");
    printf("  -d <domain>   DDS domain ID (default: 0)\n");
    printf("  -help         Show this help\n");
    printf("\nExample:\n");
    printf("  %s -p 23000 -h 127.0.0.1 -d 0\n", prog);
}

/* ========================================================================
 * Interactive Commands
 * ======================================================================== */

void print_menu(void) {
    printf("\n=== DemoApp Commands ===\n");
    printf("  s - Show status\n");
    printf("  i - Start IBIT\n");
    printf("  f - Inject fault (round/updown/sensor/power)\n");
    printf("  c - Clear fault (component name or 'all')\n");
    printf("  h - Show this menu\n");
    printf("  q - Quit\n");
    printf("========================\n");
}

void handle_command(char cmd) {
    char component[64];
    
    switch (cmd) {
        case 's':
        case 'S':
            if (g_demo_ctx) {
                printf("\n=== DemoApp Status ===\n");
                printf("State: %s\n", demo_state_name(g_demo_ctx->current_state));
                printf("Tick Count: %llu\n", g_demo_ctx->tick_count);
                printf("Signal Pub: %u\n", g_demo_ctx->signal_pub_count);
                printf("CBIT Pub: %u\n", g_demo_ctx->cbit_pub_count);
                printf("Control Rx: %u\n", g_demo_ctx->control_rx_count);
                printf("Component Status:\n");
                printf("  Round Motor: %s\n", g_demo_ctx->bit_state.pbit_components.roundMotor ? "OK" : "FAULT");
                printf("  UpDown Motor: %s\n", g_demo_ctx->bit_state.pbit_components.upDownMotor ? "OK" : "FAULT");
                printf("  Base Gyro: %s\n", g_demo_ctx->bit_state.pbit_components.baseGyro ? "OK" : "FAULT");
                printf("  Power: %s\n", g_demo_ctx->bit_state.pbit_components.powerController ? "OK" : "FAULT");
                printf("======================\n");
            }
            break;
            
        case 'i':
        case 'I':
            if (g_demo_ctx) {
                printf("Starting IBIT...\n");
                demo_app_trigger_ibit(g_demo_ctx, 1234, 2);  // I_BIT = 2
            }
            break;
            
        case 'f':
        case 'F':
            printf("Inject fault (round/updown/sensor/power/motor): ");
            if (scanf("%63s", component) == 1) {
                demo_app_inject_fault(g_demo_ctx, component);
            }
            break;
            
        case 'c':
        case 'C':
            printf("Clear fault (component or 'all'): ");
            if (scanf("%63s", component) == 1) {
                demo_app_clear_fault(g_demo_ctx, component);
            }
            break;
            
        case 'h':
        case 'H':
            print_menu();
            break;
            
        case 'q':
        case 'Q':
            g_running = 0;
            break;
            
        default:
            printf("Unknown command. Press 'h' for help.\n");
            break;
    }
}

/* ========================================================================
 * Main Entry Point
 * ======================================================================== */

int main(int argc, char* argv[]) {
    int agent_port = 23000;
    const char* agent_host = "127.0.0.1";
    int domain_id = 0;
    
    printf("==============================================\n");
    printf("  DemoApp - Windows Console Version\n");
    printf("  Phase 4.5 - Real JSON Schema Implementation\n");
    printf("==============================================\n\n");
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            agent_port = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            agent_host = argv[++i];
        }
        else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            domain_id = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        else {
            printf("Unknown option: %s\n", argv[i]);
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
    
    // Allocate context
    g_demo_ctx = (DemoAppContext*)malloc(sizeof(DemoAppContext));
    if (!g_demo_ctx) {
        printf("ERROR: Failed to allocate context\n");
        return 1;
    }
    
    // Initialize context
    demo_app_context_init(g_demo_ctx);
    g_demo_ctx->domain_id = domain_id;
    
    // Start demo application
    printf("Starting DemoApp...\n");
    printf("  Agent: %s:%d\n", agent_host, agent_port);
    printf("  DDS Domain: %d\n", domain_id);
    printf("\n");
    
    if (demo_app_start(g_demo_ctx, agent_host, (uint16_t)agent_port) != 0) {
        printf("ERROR: Failed to start DemoApp\n");
        free(g_demo_ctx);
        return 1;
    }
    
    printf("\nDemoApp started successfully!\n");
    print_menu();
    
    // Main loop - process keyboard input
    printf("\nPress 'h' for commands, 'q' to quit\n");
    
    while (g_running) {
#ifdef _WIN32
        if (_kbhit()) {
            char cmd = _getch();
            if (cmd != '\r' && cmd != '\n') {
                handle_command(cmd);
            }
        }
        Sleep(100);  // 100ms
#else
        // Linux: blocking input (not ideal but simple)
        char cmd;
        if (read(0, &cmd, 1) > 0) {
            handle_command(cmd);
        }
        usleep(100000);  // 100ms
#endif
    }
    
    // Cleanup
    printf("\nStopping DemoApp...\n");
    demo_app_stop(g_demo_ctx);
    free(g_demo_ctx);
    g_demo_ctx = NULL;
    
    printf("Goodbye!\n");
    return 0;
}
