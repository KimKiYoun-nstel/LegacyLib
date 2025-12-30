/*
 * demo_app_log.h - DemoApp Logging System
 * 
 * Features:
 *  - Unified logging interface
 *  - TX/RX message distinction
 *  - Output mode selection (console/redirect/both)
 *  - TCP log redirection (port 24000)
 */

#ifndef DEMO_APP_LOG_H
#define DEMO_APP_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Types
 * ======================================================================== */

typedef enum {
    LOG_MODE_CONSOLE = 0,    // Output to DemoApp console only
    LOG_MODE_REDIRECT = 1,   // Output to TCP log port only
    LOG_MODE_BOTH = 2        // Output to both
} LogOutputMode;

typedef enum {
    LOG_DIR_NONE = 0,        // No direction prefix
    LOG_DIR_TX = 1,          // [TX] prefix
    LOG_DIR_RX = 2,          // [RX] prefix
    LOG_DIR_INFO = 3         // [INFO] prefix
} LogDirection;

/* ========================================================================
 * Core Logging Functions
 * ======================================================================== */

/**
 * Initialize logging system
 * 
 * @param initial_mode  Initial log output mode
 * @return 0 on success, -1 on failure
 */
int demo_log_init(LogOutputMode initial_mode);

/**
 * Cleanup logging system
 */
void demo_log_cleanup(void);

/**
 * Unified logging function
 * 
 * @param dir      Direction (NONE/TX/RX/INFO)
 * @param fmt      Printf-style format string
 * @param ...      Variable arguments
 */
void demo_log(LogDirection dir, const char* fmt, ...);

/**
 * Set log output mode
 * 
 * @param mode  New mode (console/redirect/both)
 */
void demo_log_set_mode(LogOutputMode mode);

/**
 * Get current log output mode
 * 
 * @return Current mode
 */
LogOutputMode demo_log_get_mode(void);

/**
 * Get mode name string
 * 
 * @param mode  Mode enum value
 * @return Mode name ("console", "redirect", "both")
 */
const char* demo_log_mode_name(LogOutputMode mode);

/**
 * Enable or disable logging at runtime. When disabled, `demo_log` is a no-op.
 */
void demo_log_set_enabled(int enabled);
int  demo_log_get_enabled(void);

/* ========================================================================
 * TCP Log Redirection (Internal)
 * ======================================================================== */

/**
 * Send log to TCP client (if connected)
 * Called internally by demo_log()
 * 
 * @param message  Log message string
 */
void demo_log_send_to_tcp(const char* message, int sock);

/**
 * Set TCP log client socket
 * Called by TCP log server when client connects/disconnects
 * 
 * @param sock  Client socket (-1 to disconnect)
 */
void demo_log_set_tcp_client(int sock);

/**
 * Get TCP log client socket
 * 
 * @return Client socket or -1 if not connected
 */
int demo_log_get_tcp_client(void);

/* ========================================================================
 * Convenience Macros
 * ======================================================================== */

// TX/RX/INFO logging
#define LOG_TX(fmt, ...)    demo_log(LOG_DIR_TX, fmt, ##__VA_ARGS__)
#define LOG_RX(fmt, ...)    demo_log(LOG_DIR_RX, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  demo_log(LOG_DIR_INFO, fmt, ##__VA_ARGS__)
#define LOG_PLAIN(fmt, ...) demo_log(LOG_DIR_NONE, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* DEMO_APP_LOG_H */
