/*
 * demo_app_log.c - DemoApp Logging System Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "demo_app_log.h"

#ifdef _VXWORKS_
#include <vxWorks.h>
#include <semLib.h>
#include <sockLib.h>
static SEM_ID g_log_mutex = NULL;
#else
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
static CRITICAL_SECTION g_log_mutex;
static int g_log_mutex_init = 0;
#else
#include <pthread.h>
#include <sys/socket.h>
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
#endif

/* ========================================================================
 * Global State
 * ======================================================================== */

static LogOutputMode g_log_mode = LOG_MODE_CONSOLE;
static LogLevel g_log_level = LOG_LEVEL_INFO;
static int g_tcp_log_client = -1;
static int g_log_enabled = 1; /* runtime enable/disable */

/* ========================================================================
 * Platform-specific Mutex
 * ======================================================================== */

static void log_lock(void) {
#ifdef _VXWORKS_
    if (g_log_mutex) semTake(g_log_mutex, WAIT_FOREVER);
#else
#ifdef _WIN32
    if (g_log_mutex_init) EnterCriticalSection(&g_log_mutex);
#else
    pthread_mutex_lock(&g_log_mutex);
#endif
#endif
}

static void log_unlock(void) {
#ifdef _VXWORKS_
    if (g_log_mutex) semGive(g_log_mutex);
#else
#ifdef _WIN32
    if (g_log_mutex_init) LeaveCriticalSection(&g_log_mutex);
#else
    pthread_mutex_unlock(&g_log_mutex);
#endif
#endif
}

/* ========================================================================
 * Core Functions
 * ======================================================================== */

int demo_log_init(LogOutputMode initial_mode) {
    g_log_mode = initial_mode;
    g_tcp_log_client = -1;
    
#ifdef _VXWORKS_
    if (!g_log_mutex) {
        g_log_mutex = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE);
        if (!g_log_mutex) {
            printf("[DemoApp Log] Failed to create mutex\n");
            return -1;
        }
    }
#else
#ifdef _WIN32
    if (!g_log_mutex_init) {
        InitializeCriticalSection(&g_log_mutex);
        g_log_mutex_init = 1;
    }
#endif
#endif
    
    return 0;
}

void demo_log_cleanup(void) {
    log_lock();
    g_tcp_log_client = -1;
    log_unlock();
    
#ifdef _VXWORKS_
    if (g_log_mutex) {
        semDelete(g_log_mutex);
        g_log_mutex = NULL;
    }
#else
#ifdef _WIN32
    if (g_log_mutex_init) {
        DeleteCriticalSection(&g_log_mutex);
        g_log_mutex_init = 0;
    }
#endif
#endif
}

static void internal_log_output(const char* message) {
    log_lock();
    LogOutputMode mode = g_log_mode;
    int tcp_sock = g_tcp_log_client;
    log_unlock();

    // Output to console
    if (mode == LOG_MODE_CONSOLE || mode == LOG_MODE_BOTH) {
        printf("%s", message);
        fflush(stdout);
    }

    // Output to TCP log client
    if ((mode == LOG_MODE_REDIRECT || mode == LOG_MODE_BOTH) && tcp_sock >= 0) {
        demo_log_send_to_tcp(message, tcp_sock);
    }
}

void demo_log(LogLevel level, const char* fmt, ...) {
    if (!g_log_enabled) return;
    
    log_lock();
    LogLevel current_level = g_log_level;
    log_unlock();
    
    if (level > current_level) return;

    char buffer[1024];
    char prefixed[1100];
    va_list args;
    
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    // Format: [DemoApp] message
    snprintf(prefixed, sizeof(prefixed), "[DemoApp]%s", buffer);
    
    internal_log_output(prefixed);
}

void demo_log_set_level(LogLevel level) {
    log_lock();
    g_log_level = level;
    log_unlock();
}

LogLevel demo_log_get_level(void) {
    LogLevel level;
    log_lock();
    level = g_log_level;
    log_unlock();
    return level;
}

void demo_log_set_mode(LogOutputMode mode) {
    log_lock();
    g_log_mode = mode;
    log_unlock();
}

void demo_log_set_enabled(int enabled) {
    log_lock();
    g_log_enabled = enabled ? 1 : 0;
    log_unlock();
}

int demo_log_get_enabled(void) {
    int v;
    log_lock();
    v = g_log_enabled;
    log_unlock();
    return v;
}

LogOutputMode demo_log_get_mode(void) {
    LogOutputMode mode;
    log_lock();
    mode = g_log_mode;
    log_unlock();
    return mode;
}

const char* demo_log_mode_name(LogOutputMode mode) {
    switch (mode) {
        case LOG_MODE_CONSOLE:  return "console";
        case LOG_MODE_REDIRECT: return "redirect";
        case LOG_MODE_BOTH:     return "both";
        default:                return "unknown";
    }
}

/* ========================================================================
 * TCP Log Redirection
 * ======================================================================== */

// Note: This function does NOT take the log mutex. Caller must provide a
// valid `sock` value (copied under lock) to avoid holding the log mutex
// during network IO which can block high-frequency paths.
void demo_log_send_to_tcp(const char* message, int sock) {
    if (sock < 0) return;

    // Send line by line with \r\n for proper terminal display
    const char* line_start = message;
    const char* line_end;

    while (*line_start) {
        line_end = strchr(line_start, '\n');

        if (line_end) {
            if (line_end > line_start) {
                int result = send(sock, line_start, (int)(line_end - line_start), 0);
                if (result < 0) {
                    // Client disconnected
                    log_lock();
                    g_tcp_log_client = -1;
                    log_unlock();
                    return;
                }
            }

            // Send \r\n for proper Windows terminal line ending
            send(sock, "\r\n", 2, 0);
            line_start = line_end + 1;
        } else {
            if (*line_start) {
                int result = send(sock, line_start, (int)strlen(line_start), 0);
                if (result < 0) {
                    log_lock();
                    g_tcp_log_client = -1;
                    log_unlock();
                }
            }
            break;
        }
    }
}

void demo_log_set_tcp_client(int sock) {
    log_lock();
    g_tcp_log_client = sock;
    log_unlock();
}

int demo_log_get_tcp_client(void) {
    int sock;
    log_lock();
    sock = g_tcp_log_client;
    log_unlock();
    return sock;
}
