/*
 * demo_app_tcp_win.c - TCP Server Implementation for Windows
 * 
 * TCP CLI Server: Port 23000 - Command input/output
 * TCP Log Server: Port 24000 - Log redirection
 */

#ifdef _WIN32

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/demo_app_tcp.h"
#include "../include/demo_app_log.h"

#pragma comment(lib, "ws2_32.lib")

/* ========================================================================
 * TCP CLI Server (Port 23000)
 * ======================================================================== */

static SOCKET g_cli_server_sock = INVALID_SOCKET;
static SOCKET g_cli_client_sock = INVALID_SOCKET;
static HANDLE g_cli_thread = NULL;
static volatile int g_cli_running = 0;
static CRITICAL_SECTION g_cli_mutex;
static int g_cli_mutex_init = 0;
static int g_cli_port = 23000;

// Forward declaration
extern void demo_cli_process_command(char* line);

void demo_tcp_cli_print(const char* fmt, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (g_cli_mutex_init) EnterCriticalSection(&g_cli_mutex);
    
    if (g_cli_client_sock != INVALID_SOCKET) {
        // Send line by line with \r\n
        char* line_start = buffer;
        char* line_end;
        while (*line_start) {
            line_end = strchr(line_start, '\n');
            if (line_end) {
                if (line_end > line_start) {
                    send(g_cli_client_sock, line_start, (int)(line_end - line_start), 0);
                }
                send(g_cli_client_sock, "\r\n", 2, 0);
                line_start = line_end + 1;
                Sleep(1);
            } else {
                if (*line_start) {
                    send(g_cli_client_sock, line_start, (int)strlen(line_start), 0);
                }
                break;
            }
        }
    } else {
        printf("%s", buffer);
    }
    
    if (g_cli_mutex_init) LeaveCriticalSection(&g_cli_mutex);
}

/**
 * Disconnect current CLI client only (close client socket, keep server running)
 */
void demo_tcp_cli_disconnect_client(void) {
    if (g_cli_mutex_init) EnterCriticalSection(&g_cli_mutex);
    if (g_cli_client_sock != INVALID_SOCKET) {
        closesocket(g_cli_client_sock);
        g_cli_client_sock = INVALID_SOCKET;
    }
    if (g_cli_mutex_init) LeaveCriticalSection(&g_cli_mutex);
}

static DWORD WINAPI tcpCliServerThread(LPVOID param) {
    struct sockaddr_in addr;
    
    printf("[TCP CLI] Server thread started on port %d\n", g_cli_port);
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((u_short)g_cli_port);
    
    if (bind(g_cli_server_sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("[TCP CLI] Bind failed: %d\n", WSAGetLastError());
        g_cli_running = 0;
        return 1;
    }
    
    if (listen(g_cli_server_sock, 1) == SOCKET_ERROR) {
        printf("[TCP CLI] Listen failed: %d\n", WSAGetLastError());
        g_cli_running = 0;
        return 1;
    }
    
    printf("[TCP CLI] Listening on port %d...\n", g_cli_port);
    
    while (g_cli_running) {
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        
        g_cli_client_sock = accept(g_cli_server_sock, (struct sockaddr*)&client_addr, &addr_len);
        
        if (g_cli_client_sock == INVALID_SOCKET) {
            if (g_cli_running) {
                printf("[TCP CLI] Accept failed: %d\n", WSAGetLastError());
            }
            break;
        }
        
        printf("[TCP CLI] Client connected!\n");
        
        demo_tcp_cli_print("DemoApp TCP CLI\nType 'help' for commands\n> ");
        
        // CLI Loop
        char line[256];
        int line_idx = 0;
        
        while (g_cli_running && g_cli_client_sock != INVALID_SOCKET) {
            char buf[64];
            int len = recv(g_cli_client_sock, buf, sizeof(buf), 0);
            
            if (len <= 0) break;
            
            for (int i = 0; i < len; i++) {
                unsigned char c = (unsigned char)buf[i];
                
                if (c == 0xFF) continue;
                
                if (c == '\n' || c == '\r') {
                    if (line_idx > 0) {
                        line[line_idx] = 0;
                        send(g_cli_client_sock, "\r\n", 2, 0);
                        
                        demo_cli_process_command(line);
                        
                        line_idx = 0;
                        if (g_cli_client_sock == INVALID_SOCKET) break;
                        demo_tcp_cli_print("> ");
                    }
                }
                else if (c == 0x08 || c == 0x7F) {
                    if (line_idx > 0) {
                        line_idx--;
                        send(g_cli_client_sock, "\b \b", 3, 0);
                    }
                }
                else if (c >= 32 && c <= 126) {
                    if (line_idx < sizeof(line) - 1) {
                        line[line_idx++] = (char)c;
                    }
                }
            }
        }
        
        printf("[TCP CLI] Client disconnected\n");
        if (g_cli_client_sock != INVALID_SOCKET) {
            closesocket(g_cli_client_sock);
            g_cli_client_sock = INVALID_SOCKET;
        }
    }
    
    printf("[TCP CLI] Server thread exiting\n");
    return 0;
}

int demo_tcp_cli_start(int port) {
    if (g_cli_running) {
        printf("[TCP CLI] Already running\n");
        return -1;
    }
    
    g_cli_port = port;
    
    if (!g_cli_mutex_init) {
        InitializeCriticalSection(&g_cli_mutex);
        g_cli_mutex_init = 1;
    }
    
    g_cli_server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_cli_server_sock == INVALID_SOCKET) {
        printf("[TCP CLI] Failed to create socket: %d\n", WSAGetLastError());
        return -1;
    }
    
    int opt = 1;
    setsockopt(g_cli_server_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    int nodelay = 1;
    setsockopt(g_cli_server_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
    
    g_cli_running = 1;
    
    g_cli_thread = CreateThread(NULL, 0, tcpCliServerThread, NULL, 0, NULL);
    if (g_cli_thread == NULL) {
        printf("[TCP CLI] Failed to create thread\n");
        closesocket(g_cli_server_sock);
        g_cli_server_sock = INVALID_SOCKET;
        g_cli_running = 0;
        return -1;
    }
    
    return 0;
}

void demo_tcp_cli_stop(void) {
    if (!g_cli_running) return;
    
    printf("[TCP CLI] Stopping...\n");
    g_cli_running = 0;
    
    if (g_cli_client_sock != INVALID_SOCKET) {
        closesocket(g_cli_client_sock);
        g_cli_client_sock = INVALID_SOCKET;
    }
    
    if (g_cli_server_sock != INVALID_SOCKET) {
        closesocket(g_cli_server_sock);
        g_cli_server_sock = INVALID_SOCKET;
    }
    
    if (g_cli_thread) {
        WaitForSingleObject(g_cli_thread, 1000);
        CloseHandle(g_cli_thread);
        g_cli_thread = NULL;
    }
    
    if (g_cli_mutex_init) {
        DeleteCriticalSection(&g_cli_mutex);
        g_cli_mutex_init = 0;
    }
    
    printf("[TCP CLI] Stopped\n");
}

int demo_tcp_cli_is_running(void) {
    return g_cli_running;
}

/* ========================================================================
 * TCP Log Server (Port 24000)
 * ======================================================================== */

static SOCKET g_log_server_sock = INVALID_SOCKET;
static SOCKET g_log_client_sock = INVALID_SOCKET;
static HANDLE g_log_thread = NULL;
static volatile int g_log_running = 0;
static int g_log_port = 24000;

static DWORD WINAPI tcpLogServerThread(LPVOID param) {
    struct sockaddr_in addr;
    
    printf("[TCP Log] Server thread started on port %d\n", g_log_port);
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((u_short)g_log_port);
    
    if (bind(g_log_server_sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("[TCP Log] Bind failed: %d\n", WSAGetLastError());
        g_log_running = 0;
        return 1;
    }
    
    if (listen(g_log_server_sock, 1) == SOCKET_ERROR) {
        printf("[TCP Log] Listen failed: %d\n", WSAGetLastError());
        g_log_running = 0;
        return 1;
    }
    
    printf("[TCP Log] Listening on port %d...\n", g_log_port);
    
    while (g_log_running) {
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        
        g_log_client_sock = accept(g_log_server_sock, (struct sockaddr*)&client_addr, &addr_len);
        
        if (g_log_client_sock == INVALID_SOCKET) {
            if (g_log_running) {
                printf("[TCP Log] Accept failed: %d\n", WSAGetLastError());
            }
            break;
        }
        
        printf("[TCP Log] Client connected!\n");
        
        // Register with log system
        demo_log_set_tcp_client((int)g_log_client_sock);
        
        // Keep connection alive
        while (g_log_running && g_log_client_sock != INVALID_SOCKET) {
            char buf[64];
            int len = recv(g_log_client_sock, buf, sizeof(buf), 0);
            
            if (len <= 0) break;
            // Ignore data from client
        }
        
        printf("[TCP Log] Client disconnected\n");
        
        // Unregister
        demo_log_set_tcp_client(-1);
        
        if (g_log_client_sock != INVALID_SOCKET) {
            closesocket(g_log_client_sock);
            g_log_client_sock = INVALID_SOCKET;
        }
    }
    
    printf("[TCP Log] Server thread exiting\n");
    return 0;
}

int demo_tcp_log_start(int port) {
    if (g_log_running) {
        printf("[TCP Log] Already running\n");
        return -1;
    }
    
    g_log_port = port;
    
    g_log_server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_log_server_sock == INVALID_SOCKET) {
        printf("[TCP Log] Failed to create socket: %d\n", WSAGetLastError());
        return -1;
    }
    
    int opt = 1;
    setsockopt(g_log_server_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    int nodelay = 1;
    setsockopt(g_log_server_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
    
    g_log_running = 1;
    
    g_log_thread = CreateThread(NULL, 0, tcpLogServerThread, NULL, 0, NULL);
    if (g_log_thread == NULL) {
        printf("[TCP Log] Failed to create thread\n");
        closesocket(g_log_server_sock);
        g_log_server_sock = INVALID_SOCKET;
        g_log_running = 0;
        return -1;
    }
    
    return 0;
}

void demo_tcp_log_stop(void) {
    if (!g_log_running) return;
    
    printf("[TCP Log] Stopping...\n");
    g_log_running = 0;
    
    demo_log_set_tcp_client(-1);
    
    if (g_log_client_sock != INVALID_SOCKET) {
        closesocket(g_log_client_sock);
        g_log_client_sock = INVALID_SOCKET;
    }
    
    if (g_log_server_sock != INVALID_SOCKET) {
        closesocket(g_log_server_sock);
        g_log_server_sock = INVALID_SOCKET;
    }
    
    if (g_log_thread) {
        WaitForSingleObject(g_log_thread, 1000);
        CloseHandle(g_log_thread);
        g_log_thread = NULL;
    }
    
    printf("[TCP Log] Stopped\n");
}

int demo_tcp_log_is_running(void) {
    return g_log_running;
}

#endif /* _WIN32 */
