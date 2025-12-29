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
static int g_tcp_log_client = -1;

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

void demo_log(LogDirection dir, const char* fmt, ...) {
    char buffer[1024];
    char prefixed[1088];  // buffer + prefix
    va_list args;
    int len;
    
    // Format message
    va_start(args, fmt);
    len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    if (len < 0 || len >= (int)sizeof(buffer)) {
        len = sizeof(buffer) - 1;
        buffer[len] = '\0';
    }
    
    // Add direction prefix
    const char* prefix = "";
    switch (dir) {
        case LOG_DIR_TX:   prefix = "[TX] "; break;
        case LOG_DIR_RX:   prefix = "[RX] "; break;
        case LOG_DIR_INFO: prefix = "[INFO] "; break;
        case LOG_DIR_NONE:
        default:           prefix = ""; break;
    }
    
    snprintf(prefixed, sizeof(prefixed), "%s%s", prefix, buffer);
    
    // Copy needed state under lock, then do IO without holding the log mutex
    log_lock();
    LogOutputMode mode = g_log_mode;
    int tcp_sock = g_tcp_log_client;
    log_unlock();

    // Output to console (do not hold log mutex while performing IO)
    if (mode == LOG_MODE_CONSOLE || mode == LOG_MODE_BOTH) {
        printf("%s", prefixed);
        fflush(stdout);
    }

    // Output to TCP log client (pass the copied socket)
    if ((mode == LOG_MODE_REDIRECT || mode == LOG_MODE_BOTH) && tcp_sock >= 0) {
        demo_log_send_to_tcp(prefixed, tcp_sock);
    }
}

void demo_log_set_mode(LogOutputMode mode) {
    log_lock();
    g_log_mode = mode;
    log_unlock();
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
