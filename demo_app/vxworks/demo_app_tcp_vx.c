/*
 * demo_app_tcp_vx.c - TCP Server Implementation for VxWorks
 * 
 * TCP CLI Server: Port 23000 - Command input/output
 * TCP Log Server: Port 24000 - Log redirection
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
#include "../include/demo_app_tcp.h"
#include "../include/demo_app_log.h"

/* ========================================================================
 * TCP CLI Server (Port 23000)
 * ======================================================================== */

static int g_cli_server_sock = -1;
static int g_cli_client_sock = -1;
static TASK_ID g_cli_task = TASK_ID_ERROR;
static volatile int g_cli_running = 0;
static SEM_ID g_cli_mutex = NULL;
static int g_cli_port = 23000;

// Forward declaration for command processing
extern void demo_cli_process_command(char* line);

void demo_tcp_cli_print(const char* fmt, ...) {
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

static void tcpCliServerTask(void) {
    struct sockaddr_in addr;
    
    printf("[TCP CLI] Server task started on port %d\n", g_cli_port);
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(g_cli_port);
    
    if (bind(g_cli_server_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("[TCP CLI] Bind failed\n");
        g_cli_running = 0;
        return;
    }
    
    if (listen(g_cli_server_sock, 1) < 0) {
        printf("[TCP CLI] Listen failed\n");
        g_cli_running = 0;
        return;
    }
    
    printf("[TCP CLI] Listening on port %d...\n", g_cli_port);
    
    while (g_cli_running) {
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        
        g_cli_client_sock = accept(g_cli_server_sock, 
                                    (struct sockaddr*)&client_addr, 
                                    (socklen_t*)&addr_len);
        
        if (g_cli_client_sock < 0) {
            if (g_cli_running) {
                printf("[TCP CLI] Accept failed\n");
            }
            break;
        }
        
        printf("[TCP CLI] Client connected!\n");
        
        // Welcome message
        demo_tcp_cli_print("DemoApp TCP CLI\nType 'help' for commands\n> ");
        
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
                        
                        demo_cli_process_command(line);
                        
                        line_idx = 0;
                        if (g_cli_client_sock < 0) break;
                        demo_tcp_cli_print("> ");
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
        
        printf("[TCP CLI] Client disconnected\n");
        if (g_cli_client_sock >= 0) {
            close(g_cli_client_sock);
            g_cli_client_sock = -1;
        }
    }
    
    printf("[TCP CLI] Server task exiting\n");
}

int demo_tcp_cli_start(int port) {
    if (g_cli_running) {
        printf("[TCP CLI] Already running\n");
        return -1;
    }
    
    g_cli_port = port;
    
    if (!g_cli_mutex) {
        g_cli_mutex = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE);
        if (!g_cli_mutex) {
            printf("[TCP CLI] Failed to create mutex\n");
            return -1;
        }
    }
    
    g_cli_server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (g_cli_server_sock < 0) {
        printf("[TCP CLI] Failed to create socket\n");
        return -1;
    }
    
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
        (FUNCPTR)tcpCliServerTask,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    );
    
    if (g_cli_task == TASK_ID_ERROR) {
        printf("[TCP CLI] Failed to spawn server task\n");
        close(g_cli_server_sock);
        g_cli_server_sock = -1;
        g_cli_running = 0;
        return -1;
    }
    
    return 0;
}

void demo_tcp_cli_stop(void) {
    if (!g_cli_running) return;
    
    printf("[TCP CLI] Stopping...\n");
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
        taskDelay(sysClkRateGet() / 10);
        if (taskIdVerify(g_cli_task) == OK) {
            taskDelete(g_cli_task);
        }
        g_cli_task = TASK_ID_ERROR;
    }
    
    if (g_cli_mutex) {
        semDelete(g_cli_mutex);
        g_cli_mutex = NULL;
    }
    
    printf("[TCP CLI] Stopped\n");
}

int demo_tcp_cli_is_running(void) {
    return g_cli_running;
}

/* ========================================================================
 * TCP Log Server (Port 24000)
 * ======================================================================== */

static int g_log_server_sock = -1;
static int g_log_client_sock = -1;
static TASK_ID g_log_task = TASK_ID_ERROR;
static volatile int g_log_running = 0;
static int g_log_port = 24000;

static void tcpLogServerTask(void) {
    struct sockaddr_in addr;
    
    printf("[TCP Log] Server task started on port %d\n", g_log_port);
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(g_log_port);
    
    if (bind(g_log_server_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("[TCP Log] Bind failed\n");
        g_log_running = 0;
        return;
    }
    
    if (listen(g_log_server_sock, 1) < 0) {
        printf("[TCP Log] Listen failed\n");
        g_log_running = 0;
        return;
    }
    
    printf("[TCP Log] Listening on port %d...\n", g_log_port);
    
    while (g_log_running) {
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        
        g_log_client_sock = accept(g_log_server_sock,
                                    (struct sockaddr*)&client_addr,
                                    (socklen_t*)&addr_len);
        
        if (g_log_client_sock < 0) {
            if (g_log_running) {
                printf("[TCP Log] Accept failed\n");
            }
            break;
        }
        
        printf("[TCP Log] Client connected!\n");
        
        // Register socket with log system
        demo_log_set_tcp_client(g_log_client_sock);
        
        // Keep connection alive
        while (g_log_running && g_log_client_sock >= 0) {
            char buf[64];
            int len = recv(g_log_client_sock, buf, sizeof(buf), 0);
            
            if (len <= 0) {
                // Client disconnected
                break;
            }
            // Ignore any data from client (log is output-only)
        }
        
        printf("[TCP Log] Client disconnected\n");
        
        // Unregister socket
        demo_log_set_tcp_client(-1);
        
        if (g_log_client_sock >= 0) {
            close(g_log_client_sock);
            g_log_client_sock = -1;
        }
    }
    
    printf("[TCP Log] Server task exiting\n");
}

int demo_tcp_log_start(int port) {
    if (g_log_running) {
        printf("[TCP Log] Already running\n");
        return -1;
    }
    
    g_log_port = port;
    
    g_log_server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (g_log_server_sock < 0) {
        printf("[TCP Log] Failed to create socket\n");
        return -1;
    }
    
    int opt = 1;
    setsockopt(g_log_server_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    int nodelay = 1;
    setsockopt(g_log_server_sock, 6, 1, (char*)&nodelay, sizeof(nodelay));
    
    g_log_running = 1;
    
    g_log_task = taskSpawn(
        "tDemoAppLog",
        100,
        0,
        32768,
        (FUNCPTR)tcpLogServerTask,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    );
    
    if (g_log_task == TASK_ID_ERROR) {
        printf("[TCP Log] Failed to spawn server task\n");
        close(g_log_server_sock);
        g_log_server_sock = -1;
        g_log_running = 0;
        return -1;
    }
    
    return 0;
}

void demo_tcp_log_stop(void) {
    if (!g_log_running) return;
    
    printf("[TCP Log] Stopping...\n");
    g_log_running = 0;
    
    // Unregister from log system
    demo_log_set_tcp_client(-1);
    
    if (g_log_client_sock >= 0) {
        close(g_log_client_sock);
        g_log_client_sock = -1;
    }
    
    if (g_log_server_sock >= 0) {
        close(g_log_server_sock);
        g_log_server_sock = -1;
    }
    
    if (g_log_task != TASK_ID_ERROR) {
        taskDelay(sysClkRateGet() / 10);
        if (taskIdVerify(g_log_task) == OK) {
            taskDelete(g_log_task);
        }
        g_log_task = TASK_ID_ERROR;
    }
    
    printf("[TCP Log] Stopped\n");
}

int demo_tcp_log_is_running(void) {
    return g_log_running;
}
