/*
 * demo_app_cli.c - TCP CLI Server for DemoApp
 * 
 * Extracted and adapted from examples/demo_dkm.c
 */

#include <vxWorks.h>
#include <taskLib.h>
#include <sysLib.h>
#include <sockLib.h>
#include <inetLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semLib.h>
#include "../include/demo_app.h"

/* ========================================================================
 * Global State
 * ======================================================================== */

static int g_cli_server_sock = -1;
static int g_cli_client_sock = -1;
static TASK_ID g_cli_task = TASK_ID_ERROR;
static volatile int g_cli_running = 0;
static SEM_ID g_cli_mutex = NULL;

// External reference to main context (set by dkm module)
extern DemoAppContext* g_demo_ctx;

/* ========================================================================
 * Output Helper
 * ======================================================================== */

void demo_cli_print(const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (g_cli_mutex) semTake(g_cli_mutex, WAIT_FOREVER);
    
    if (g_cli_client_sock >= 0) {
        // Send line by line with \r\n for proper terminal display
        char* line_start = buffer;
        char* line_end;
        while (*line_start) {
            line_end = strchr(line_start, '\n');
            if (line_end) {
                if (line_end > line_start) {
                    send(g_cli_client_sock, line_start, line_end - line_start, 0);
                }
                send(g_cli_client_sock, "\r\n", 2, 0);
                line_start = line_end + 1;
                taskDelay(1);
            } else {
                if (*line_start) {
                    send(g_cli_client_sock, line_start, strlen(line_start), 0);
                }
                break;
            }
        }
    } else {
        printf("%s", buffer);
    }
    
    if (g_cli_mutex) semGive(g_cli_mutex);
}

/* ========================================================================
 * Command Tokenizer
 * ======================================================================== */

static int tokenize_command(char* line, char** tokens, int max_tokens) {
    int count = 0;
    char* token = strtok(line, " \t");
    
    while (token != NULL && count < max_tokens) {
        tokens[count++] = token;
        token = strtok(NULL, " \t");
    }
    
    return count;
}

/* ========================================================================
 * Help
 * ======================================================================== */

static void print_help(void) {
    demo_cli_print(
        "\n=== DemoApp CLI Commands ===\n"
        "  help                : Show this help\n"
        "  status              : Show current state and statistics\n"
        "\n[3-Step Execution]\n"
        "  connect [ip] [port] : Connect to Agent (hello + clear)\n"
        "  create_entities     : Create DDS entities\n"
        "  start_scenario      : Start scenario (PBIT + timers)\n"
        "\n[Legacy Commands]\n"
        "  demo_init           : Run all 3 steps automatically\n"
        "  demo_start          : Alias for demo_init\n"
        "  demo_stop           : Stop demo (->PEND, pause timers)\n"
        "\n[Testing]\n"
        "  run_ibit <ref> <type> : Trigger IBIT manually\n"
        "  fault_inject <comp> : Inject fault (azimuth|updown|sensor)\n"
        "  fault_clear <comp>  : Clear fault (azimuth|updown|sensor|all)\n"
        "\n[Misc]\n"
        "  quit                : Close CLI connection (server will close)\n"
        "=============================\n"
    );
}

/* ========================================================================
 * Command Processing
 * ======================================================================== */

static void process_command(char* line) {
    char* tokens[8];
    int token_count = tokenize_command(line, tokens, 8);
    
    if (token_count == 0) return;
    
    const char* cmd = tokens[0];
    
    if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
        demo_cli_print("Closing CLI connection...\n");
        if (g_cli_client_sock >= 0) {
            close(g_cli_client_sock);
            g_cli_client_sock = -1;
        }
        return;
    }
    else if (strcmp(cmd, "help") == 0) {
        print_help();
    }
    else if (strcmp(cmd, "connect") == 0) {
        if (!g_demo_ctx) {
            demo_cli_print("DemoApp context not initialized\n");
            return;
        }
        
        const char* agent_ip = (token_count >= 2) ? tokens[1] : "127.0.0.1";
        int agent_port = (token_count >= 3) ? atoi(tokens[2]) : 25000;
        
        int ret = demo_app_start(g_demo_ctx, agent_ip, agent_port);
        if (ret == 0) {
            demo_cli_print("Connected to Agent at %s:%d\n", agent_ip, agent_port);
            demo_cli_print("Next: run 'create_entities' command\n");
        } else {
            demo_cli_print("Failed to connect to Agent\n");
        }
    }
    else if (strcmp(cmd, "create_entities") == 0) {
        if (!g_demo_ctx) {
            demo_cli_print("DemoApp context not initialized\n");
            return;
        }
        
        int ret = demo_app_create_entities(g_demo_ctx);
        if (ret == 0) {
            demo_cli_print("DDS entities created successfully\n");
            demo_cli_print("Next: run 'start_scenario' command\n");
        } else {
            demo_cli_print("Failed to create DDS entities\n");
        }
    }
    else if (strcmp(cmd, "start_scenario") == 0) {
        if (!g_demo_ctx) {
            demo_cli_print("DemoApp context not initialized\n");
            return;
        }
        
        int ret = demo_app_start_scenario(g_demo_ctx);
        if (ret == 0) {
            demo_cli_print("Scenario started (PBIT + timers)\n");
            demo_cli_print("Check AgentUI for DDS messages\n");
        } else {
            demo_cli_print("Failed to start scenario\n");
        }
    }
    else if (strcmp(cmd, "status") == 0) {
        if (!g_demo_ctx) {
            demo_cli_print("DemoApp context not initialized\n");
            return;
        }
        
        demo_cli_print("\n=== DemoApp Status ===\n");
        demo_cli_print("State: %s\n", demo_state_name(g_demo_ctx->current_state));
        demo_cli_print("Tick Count: %llu\n", g_demo_ctx->tick_count);
        demo_cli_print("\nStatistics:\n");
        demo_cli_print("  Signal Published: %u\n", g_demo_ctx->signal_pub_count);
        demo_cli_print("  CBIT Published: %u\n", g_demo_ctx->cbit_pub_count);
        demo_cli_print("  Control Received: %u\n", g_demo_ctx->control_rx_count);
        demo_cli_print("  Speed Received: %u\n", g_demo_ctx->speed_rx_count);
        demo_cli_print("\nBIT State:\n");
        demo_cli_print("  PBIT Completed: %s\n", g_demo_ctx->bit_state.pbit_completed ? "Yes" : "No");
        demo_cli_print("  CBIT Active: %s\n", g_demo_ctx->bit_state.cbit_active ? "Yes" : "No");
        demo_cli_print("  IBIT Running: %s\n", g_demo_ctx->bit_state.ibit_running ? "Yes" : "No");
        demo_cli_print("\nComponent Status:\n");
        demo_cli_print("  Round Motor: %s\n", g_demo_ctx->bit_state.pbit_components.roundMotor == L_BITResultType_NORMAL ? "OK" : "FAULT");
        demo_cli_print("  UpDown Motor: %s\n", g_demo_ctx->bit_state.pbit_components.upDownMotor == L_BITResultType_NORMAL ? "OK" : "FAULT");
        demo_cli_print("  Base Giro: %s\n", g_demo_ctx->bit_state.pbit_components.baseGiro == L_BITResultType_NORMAL ? "OK" : "FAULT");
        demo_cli_print("  Power Controller: %s\n", g_demo_ctx->bit_state.pbit_components.powerController == L_BITResultType_NORMAL ? "OK" : "FAULT");
        demo_cli_print("======================\n");
    }
    else if (strcmp(cmd, "demo_init") == 0 || strcmp(cmd, "demo_start") == 0) {
        if (!g_demo_ctx) {
            demo_cli_print("DemoApp context not initialized\n");
            return;
        }
        
        const char* agent_ip = (token_count >= 2) ? tokens[1] : "127.0.0.1";
        int agent_port = (token_count >= 3) ? atoi(tokens[2]) : 25000;
        
        demo_cli_print("Running 3-step initialization...\n");
        
        // Step 1: Connect
        int ret = demo_app_start(g_demo_ctx, agent_ip, agent_port);
        if (ret != 0) {
            demo_cli_print("[FAILED] Step 1: Connect to Agent\n");
            return;
        }
        demo_cli_print("[OK] Step 1: Connected to Agent\n");
        
        // Step 2: Create entities
        ret = demo_app_create_entities(g_demo_ctx);
        if (ret != 0) {
            demo_cli_print("[FAILED] Step 2: Create DDS entities\n");
            return;
        }
        demo_cli_print("[OK] Step 2: DDS entities created\n");
        
        // Step 3: Start scenario
        ret = demo_app_start_scenario(g_demo_ctx);
        if (ret != 0) {
            demo_cli_print("[FAILED] Step 3: Start scenario\n");
            return;
        }
        demo_cli_print("[OK] Step 3: Scenario started\n");
        demo_cli_print("Demo initialization completed successfully!\n");
    }
    else if (strcmp(cmd, "demo_stop") == 0) {
        if (!g_demo_ctx) {
            demo_cli_print("DemoApp context not initialized\n");
            return;
        }
        
        demo_app_stop(g_demo_ctx);
        demo_cli_print("Demo stopped\n");
    }
    else if (strcmp(cmd, "run_ibit") == 0) {
        if (!g_demo_ctx) {
            demo_cli_print("DemoApp context not initialized\n");
            return;
        }
        
        uint32_t ref = (token_count >= 2) ? (uint32_t)atoi(tokens[1]) : 1;
        int type = (token_count >= 3) ? atoi(tokens[2]) : 0;
        
        int ret = demo_app_trigger_ibit(g_demo_ctx, ref, type);
        if (ret == 0) {
            demo_cli_print("IBIT triggered (ref=%u, type=%d)\n", ref, type);
        } else {
            demo_cli_print("IBIT trigger failed\n");
        }
    }
    else if (strcmp(cmd, "fault_inject") == 0) {
        if (!g_demo_ctx) {
            demo_cli_print("DemoApp context not initialized\n");
            return;
        }
        
        if (token_count < 2) {
            demo_cli_print("Usage: fault_inject <component>\n");
            demo_cli_print("  Components: azimuth, updown, sensor\n");
            return;
        }
        
        demo_app_inject_fault(g_demo_ctx, tokens[1]);
    }
    else if (strcmp(cmd, "fault_clear") == 0) {
        if (!g_demo_ctx) {
            demo_cli_print("DemoApp context not initialized\n");
            return;
        }
        
        if (token_count < 2) {
            demo_cli_print("Usage: fault_clear <component>\n");
            demo_cli_print("  Components: azimuth, updown, sensor, all\n");
            return;
        }
        
        demo_app_clear_fault(g_demo_ctx, tokens[1]);
    }
    else {
        demo_cli_print("Unknown command: %s (type 'help' for commands)\n", cmd);
    }
}

/* ========================================================================
 * TCP Server Task
 * ======================================================================== */

static void cliServerTask(void) {
    struct sockaddr_in addr;
    
    printf("[DemoApp CLI] TCP Server Task started\n");
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(23000);  // Will be parameterized later
    
    if (bind(g_cli_server_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("[DemoApp CLI] Bind failed\n");
        g_cli_running = 0;
        return;
    }
    
    if (listen(g_cli_server_sock, 1) < 0) {
        printf("[DemoApp CLI] Listen failed\n");
        g_cli_running = 0;
        return;
    }
    
    printf("[DemoApp CLI] Listening on port 23000...\n");
    
    while (g_cli_running) {
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        
        g_cli_client_sock = accept(g_cli_server_sock, 
                                    (struct sockaddr*)&client_addr, 
                                    (socklen_t*)&addr_len);
        
        if (g_cli_client_sock < 0) {
            if (g_cli_running) {
                printf("[DemoApp CLI] Accept failed\n");
            }
            break;
        }
        
        printf("[DemoApp CLI] Client connected!\n");
        
        // Welcome message
        demo_cli_print("DemoApp CLI\nType 'help' for commands\n> ");
        
        // CLI Loop
        char line[256];
        int line_idx = 0;
        
        while (g_cli_running && g_cli_client_sock >= 0) {
            char buf[64];
            int len = recv(g_cli_client_sock, buf, sizeof(buf), 0);
            
            if (len <= 0) break;
            
            for (int i = 0; i < len; i++) {
                unsigned char c = (unsigned char)buf[i];
                
                if (c == 0xFF) continue;  // Skip Telnet IAC
                
                if (c == '\n' || c == '\r') {
                    if (line_idx > 0) {
                        line[line_idx] = 0;
                        send(g_cli_client_sock, "\r\n", 2, 0);
                        
                        process_command(line);
                        
                        line_idx = 0;
                        if (g_cli_client_sock < 0) break;
                        demo_cli_print("> ");
                    }
                }
                else if (c == 0x08 || c == 0x7F) {  // Backspace
                    if (line_idx > 0) {
                        line_idx--;
                        send(g_cli_client_sock, "\b \b", 3, 0);
                    }
                }
                else if (c >= 32 && c <= 126) {
                    if (line_idx < sizeof(line) - 1) {
                        line[line_idx++] = c;
                    }
                }
            }
        }
        
        printf("[DemoApp CLI] Client disconnected\n");
        if (g_cli_client_sock >= 0) {
            close(g_cli_client_sock);
            g_cli_client_sock = -1;
        }
    }
    
    printf("[DemoApp CLI] TCP Server Task exiting\n");
}

/* ========================================================================
 * Public API
 * ======================================================================== */

int demo_cli_start(int port) {
    if (g_cli_running) {
        printf("[DemoApp CLI] Already running\n");
        return -1;
    }
    
    // Create mutex
    if (!g_cli_mutex) {
        g_cli_mutex = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE);
        if (!g_cli_mutex) {
            printf("[DemoApp CLI] Failed to create mutex\n");
            return -1;
        }
    }
    
    g_cli_server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (g_cli_server_sock < 0) {
        printf("[DemoApp CLI] Failed to create socket\n");
        return -1;
    }
    
    // Socket options
    int opt = 1;
    setsockopt(g_cli_server_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    int nodelay = 1;
    setsockopt(g_cli_server_sock, 6, 1, (char*)&nodelay, sizeof(nodelay));
    
    g_cli_running = 1;
    
    g_cli_task = taskSpawn(
        "tDemoAppCli",
        100,
        0,
        32768,
        (FUNCPTR)cliServerTask,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    );
    
    if (g_cli_task == TASK_ID_ERROR) {
        printf("[DemoApp CLI] Failed to spawn server task\n");
        close(g_cli_server_sock);
        g_cli_server_sock = -1;
        g_cli_running = 0;
        return -1;
    }
    
    printf("[DemoApp CLI] Started on port %d\n", port);
    return 0;
}

void demo_cli_stop(void) {
    if (!g_cli_running) {
        return;
    }
    
    printf("[DemoApp CLI] Stopping...\n");
    g_cli_running = 0;
    
    if (g_cli_client_sock >= 0) {
        close(g_cli_client_sock);
        g_cli_client_sock = -1;
    }
    
    if (g_cli_server_sock >= 0) {
        close(g_cli_server_sock);
        g_cli_server_sock = -1;
    }
    
    if (g_cli_task != TASK_ID_ERROR) {
        taskDelay(sysClkRateGet() / 10);  // 100ms
        if (taskIdVerify(g_cli_task) == OK) {
            taskDelete(g_cli_task);
        }
        g_cli_task = TASK_ID_ERROR;
    }
    
    if (g_cli_mutex) {
        semDelete(g_cli_mutex);
        g_cli_mutex = NULL;
    }
    
    printf("[DemoApp CLI] Stopped\n");
}
