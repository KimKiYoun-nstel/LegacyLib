/*
 * demo_app_tcp.h - TCP Server Interface for DemoApp
 * 
 * Features:
 *  - TCP CLI Server (port 23000) - Command input/output
 *  - TCP Log Server (port 24000) - Log redirection
 */

#ifndef DEMO_APP_TCP_H
#define DEMO_APP_TCP_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * TCP CLI Server (Port 23000)
 * ======================================================================== */

/**
 * Start TCP CLI server
 * 
 * @param port  TCP port to listen on (default: 23000)
 * @return 0 on success, -1 on failure
 */
int demo_tcp_cli_start(int port);

/**
 * Stop TCP CLI server
 */
void demo_tcp_cli_stop(void);

/**
 * Check if TCP CLI server is running
 * 
 * @return 1 if running, 0 if not
 */
int demo_tcp_cli_is_running(void);

/**
 * Send message to CLI client (for responses)
 * 
 * @param fmt  Printf-style format string
 * @param ...  Variable arguments
 */
void demo_tcp_cli_print(const char* fmt, ...);

/* ========================================================================
 * TCP Log Server (Port 24000)
 * ======================================================================== */

/**
 * Start TCP log server
 * 
 * @param port  TCP port to listen on (default: 24000)
 * @return 0 on success, -1 on failure
 */
int demo_tcp_log_start(int port);

/**
 * Stop TCP log server
 */
void demo_tcp_log_stop(void);

/**
 * Check if TCP log server is running
 * 
 * @return 1 if running, 0 if not
 */
int demo_tcp_log_is_running(void);

#ifdef __cplusplus
}
#endif

#endif /* DEMO_APP_TCP_H */
