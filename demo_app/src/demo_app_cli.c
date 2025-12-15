/*
 * demo_app_cli.c - CLI Command Processing (Common)
 * 
 * Handles CLI commands for both VxWorks and Windows
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "../include/demo_app.h"
#include "../include/demo_app_tcp.h"
#include "../include/demo_app_log.h"

// External reference to global context
extern DemoAppContext* g_demo_ctx;

/* ========================================================================
 * Command Tokenizer
 * ======================================================================== */

static int tokenize_command(char* line, char** tokens, int max_tokens) {

/* Forward declarations */
void demo_cli_stop(void);

    int count = 0;
    char* token = strtok(line, " \t");
    
    while (token != NULL && count < max_tokens) {
        tokens[count++] = token;
        token = strtok(NULL, " \t");
    }
    
    return count;
}

/* ========================================================================
 * Help Command
 * ======================================================================== */

static void cmd_help(void) {
    demo_tcp_cli_print(
        "\n=== DemoApp CLI Commands ===\n"
        "\n[Connection]\n"
        "  connect [ip] [port]       : Connect to Agent (default: 127.0.0.1:25000)\n"
        "  reset                     : Full reset (cleanup entities, set Idle)\n"
        "  status                    : Show current status\n"
        "\n[Entity Management]\n"
        "  create_entities           : Create DDS entities\n"
        "\n[Scenario Control]\n"
        "  start_scenario            : Start scenario (PBIT + timers)\n"
        "  stop_scenario             : Stop scenario (stop timers)\n"
        "\n[Test Commands]\n"
        "  test_write <topic>        : Send 1 test message\n"
        "    Topics: pbit, cbit, result_bit, signal, all\n"
        "  run_ibit <ref> <type>     : Trigger IBIT manually\n"
        "  fault_inject <component>  : Inject fault (azimuth|updown|sensor)\n"
        "  fault_clear <component>   : Clear fault (azimuth|updown|sensor|all)\n"
        "\n[Log Control]\n"
        "  log_mode <mode>           : Set log output mode\n"
        "    Modes: console, redirect, both\n"
        "  log_status                : Show current log mode\n"
        "\n[Other]\n"
        "  help                      : Show this help\n"
        "  quit                      : Close CLI connection\n"
        "============================\n"
    );
}

/* ========================================================================
 * Status Command
 * ======================================================================== */

static void cmd_status(void) {
    if (!g_demo_ctx) {
        demo_tcp_cli_print("ERROR: DemoApp context not initialized\n");
        return;
    }
    
    uint64_t tick_ms = g_demo_ctx->tick_count;
    uint64_t tick_sec = tick_ms / 1000;
    uint64_t tick_min = tick_sec / 60;
    uint64_t tick_sec_remain = tick_sec % 60;
    
    demo_tcp_cli_print("\n=== DemoApp Status ===\n");
    demo_tcp_cli_print("State: %s\n", demo_state_name(g_demo_ctx->current_state));
    
    if (tick_min > 0) {
        demo_tcp_cli_print("Uptime: %llu min %llu sec (%llu ms)\n", 
            (unsigned long long)tick_min, 
            (unsigned long long)tick_sec_remain,
            (unsigned long long)tick_ms);
    } else {
        demo_tcp_cli_print("Uptime: %llu.%03llu sec (%llu ms)\n", 
            (unsigned long long)tick_sec,
            (unsigned long long)(tick_ms % 1000),
            (unsigned long long)tick_ms);
    }
    
    demo_tcp_cli_print("\n[Published Messages - Total / Current Hz]\n");
    uint32_t target_signal_hz = (g_demo_ctx->signal_period_ms == 0) ? 0 : (1000u / g_demo_ctx->signal_period_ms);
    uint32_t target_cbit_hz = (g_demo_ctx->cbit_period_ms == 0) ? 0 : (1000u / g_demo_ctx->cbit_period_ms);
    uint32_t target_pbit_hz = (g_demo_ctx->pbit_period_ms == 0) ? 0 : (1000u / g_demo_ctx->pbit_period_ms);

    demo_tcp_cli_print("  Signal (target %u Hz) : %5u total (%3u Hz)\n", 
        target_signal_hz, g_demo_ctx->signal_pub_count, g_demo_ctx->signal_pub_hz);
    demo_tcp_cli_print("  CBIT   (target %u Hz) : %5u total (%3u Hz)\n", 
        target_cbit_hz, g_demo_ctx->cbit_pub_count, g_demo_ctx->cbit_pub_hz);
    demo_tcp_cli_print("  PBIT   (target %u Hz) : %5u total (%3u Hz)\n", 
        target_pbit_hz, g_demo_ctx->pbit_pub_count, g_demo_ctx->pbit_pub_hz);
    demo_tcp_cli_print("  ResultBIT             : %5u total (%3u Hz)\n", 
        g_demo_ctx->result_pub_count, g_demo_ctx->result_pub_hz);
    
    demo_tcp_cli_print("\n[Subscribed Messages - Total / Current Hz]\n");
    demo_tcp_cli_print("  Control            : %5u total (%3u Hz)\n", 
        g_demo_ctx->control_rx_count, g_demo_ctx->control_rx_hz);
    demo_tcp_cli_print("  Speed              : %5u total (%3u Hz)\n", 
        g_demo_ctx->speed_rx_count, g_demo_ctx->speed_rx_hz);
    demo_tcp_cli_print("  RunBIT             : %5u total (%3u Hz)\n", 
        g_demo_ctx->runbit_rx_count, g_demo_ctx->runbit_rx_hz);
    
    demo_tcp_cli_print("\n[BIT State]\n");
    demo_tcp_cli_print("  PBIT Completed: %s\n", g_demo_ctx->bit_state.pbit_completed ? "Yes" : "No");
    demo_tcp_cli_print("  CBIT Active: %s\n", g_demo_ctx->bit_state.cbit_active ? "Yes" : "No");
    demo_tcp_cli_print("  IBIT Running: %s\n", g_demo_ctx->bit_state.ibit_running ? "Yes" : "No");
    
    demo_tcp_cli_print("\n[System Status]\n");
    demo_tcp_cli_print("  Timer Running: %s\n", demo_timer_is_running() ? "Yes" : "No");
    demo_tcp_cli_print("  Log Mode: %s\n", demo_log_mode_name(demo_log_get_mode()));
    
    if (!demo_timer_is_running() && g_demo_ctx->current_state == DEMO_STATE_RUN) {
        demo_tcp_cli_print("\n[WARNING] Timer not running but state is Run!\n");
        demo_tcp_cli_print("          Uptime counter is frozen.\n");
    }
    
    demo_tcp_cli_print("======================\n");
}

/* ========================================================================
 * Command Handlers
 * ======================================================================== */

static void cmd_connect(int token_count, char** tokens) {
    if (!g_demo_ctx) {
        demo_tcp_cli_print("ERROR: DemoApp context not initialized\n");
        return;
    }
    
    const char* agent_ip = (token_count >= 2) ? tokens[1] : "127.0.0.1";
    int agent_port = (token_count >= 3) ? atoi(tokens[2]) : 25000;
    
    demo_tcp_cli_print("Connecting to Agent at %s:%d...\n", agent_ip, agent_port);
    
    int ret = demo_app_start(g_demo_ctx, agent_ip, (uint16_t)agent_port);
    if (ret == 0) {
        demo_tcp_cli_print("OK: Connected to Agent\n");
        demo_tcp_cli_print("Next: run 'create_entities' command\n");
    } else {
        demo_tcp_cli_print("ERROR: Failed to connect to Agent\n");
    }
}

// 'disconnect' command removed (use 'reset' to cleanup)

static void cmd_reset(void) {
    if (!g_demo_ctx) {
        demo_tcp_cli_print("ERROR: DemoApp context not initialized\n");
        return;
    }
    demo_tcp_cli_print("Resetting DemoApp (full cleanup)...\n");
    if (demo_app_reset(g_demo_ctx) == 0) {
        demo_tcp_cli_print("OK: State reset to Idle\n");
    } else {
        demo_tcp_cli_print("ERROR: Failed to reset DemoApp\n");
    }
}

static void cmd_create_entities(void) {
    if (!g_demo_ctx) {
        demo_tcp_cli_print("ERROR: DemoApp context not initialized\n");
        return;
    }
    
    demo_tcp_cli_print("Creating DDS entities...\n");
    
    int ret = demo_app_create_entities(g_demo_ctx);
    if (ret == 0) {
        demo_tcp_cli_print("OK: DDS entities created\n");
        demo_tcp_cli_print("Next: run 'test_write all' or 'start_scenario'\n");
    } else {
        demo_tcp_cli_print("ERROR: Failed to create DDS entities\n");
    }
}

static void cmd_start_scenario(void) {
    if (!g_demo_ctx) {
        demo_tcp_cli_print("ERROR: DemoApp context not initialized\n");
        return;
    }
    
    demo_tcp_cli_print("Starting scenario...\n");
    
    int ret = demo_app_start_scenario(g_demo_ctx);
    if (ret == 0) {
        demo_tcp_cli_print("OK: Scenario started (PBIT + timers active)\n");
        demo_tcp_cli_print("Check log output (port 24000) for periodic messages\n");
    } else {
        demo_tcp_cli_print("ERROR: Failed to start scenario\n");
    }
}

static void cmd_stop_scenario(void) {
    if (!g_demo_ctx) {
        demo_tcp_cli_print("ERROR: DemoApp context not initialized\n");
        return;
    }
    demo_tcp_cli_print("Pausing scenario (timers will be stopped)...\n");
    demo_app_stop(g_demo_ctx); // pause only
    demo_tcp_cli_print("OK: Scenario paused (state=PEND)\n");
}

static void cmd_test_write(int token_count, char** tokens) {
    if (!g_demo_ctx) {
        demo_tcp_cli_print("ERROR: DemoApp context not initialized\n");
        return;
    }
    
    if (token_count < 2) {
        demo_tcp_cli_print("Usage: test_write <topic>\n");
        demo_tcp_cli_print("  Topics: pbit, cbit, result_bit, signal, all\n");
        return;
    }
    
    const char* topic = tokens[1];
    int ret = 0;
    
    if (strcmp(topic, "pbit") == 0) {
        demo_tcp_cli_print("Sending PBIT test message...\n");
        ret = demo_msg_test_write_pbit(g_demo_ctx);
    }
    else if (strcmp(topic, "cbit") == 0) {
        demo_tcp_cli_print("Sending CBIT test message...\n");
        ret = demo_msg_test_write_cbit(g_demo_ctx);
    }
    else if (strcmp(topic, "result_bit") == 0) {
        demo_tcp_cli_print("Sending resultBIT test message...\n");
        ret = demo_msg_test_write_result_bit(g_demo_ctx);
    }
    else if (strcmp(topic, "signal") == 0) {
        demo_tcp_cli_print("Sending Actuator Signal test message...\n");
        ret = demo_msg_test_write_signal(g_demo_ctx);
    }
    else if (strcmp(topic, "all") == 0) {
        demo_tcp_cli_print("Sending all 4 test messages...\n");
        ret |= demo_msg_test_write_pbit(g_demo_ctx);
        ret |= demo_msg_test_write_cbit(g_demo_ctx);
        ret |= demo_msg_test_write_result_bit(g_demo_ctx);
        ret |= demo_msg_test_write_signal(g_demo_ctx);
    }
    else {
        demo_tcp_cli_print("ERROR: Unknown topic: %s\n", topic);
        demo_tcp_cli_print("Valid topics: pbit, cbit, result_bit, signal, all\n");
        return;
    }
    
    if (ret == 0) {
        demo_tcp_cli_print("OK: Test message(s) sent\n");
    } else {
        demo_tcp_cli_print("ERROR: Failed to send test message(s)\n");
    }
}

static void cmd_run_ibit(int token_count, char** tokens) {
    if (!g_demo_ctx) {
        demo_tcp_cli_print("ERROR: DemoApp context not initialized\n");
        return;
    }
    
    uint32_t ref = (token_count >= 2) ? (uint32_t)atoi(tokens[1]) : 1;
    int type = (token_count >= 3) ? atoi(tokens[2]) : 0;
    
    demo_tcp_cli_print("Triggering IBIT (ref=%u, type=%d)...\n", ref, type);
    
    int ret = demo_app_trigger_ibit(g_demo_ctx, ref, type);
    if (ret == 0) {
        demo_tcp_cli_print("OK: IBIT triggered\n");
    } else {
        demo_tcp_cli_print("ERROR: Failed to trigger IBIT\n");
    }
}

static void cmd_fault_inject(int token_count, char** tokens) {
    if (!g_demo_ctx) {
        demo_tcp_cli_print("ERROR: DemoApp context not initialized\n");
        return;
    }
    
    if (token_count < 2) {
        demo_tcp_cli_print("Usage: fault_inject <component>\n");
        demo_tcp_cli_print("  Components: azimuth, updown, sensor\n");
        return;
    }
    
    demo_app_inject_fault(g_demo_ctx, tokens[1]);
}

static void cmd_fault_clear(int token_count, char** tokens) {
    if (!g_demo_ctx) {
        demo_tcp_cli_print("ERROR: DemoApp context not initialized\n");
        return;
    }
    
    if (token_count < 2) {
        demo_tcp_cli_print("Usage: fault_clear <component>\n");
        demo_tcp_cli_print("  Components: azimuth, updown, sensor, all\n");
        return;
    }
    
    demo_app_clear_fault(g_demo_ctx, tokens[1]);
}

static void cmd_log_mode(int token_count, char** tokens) {
    if (token_count < 2) {
        demo_tcp_cli_print("Usage: log_mode <mode>\n");
        demo_tcp_cli_print("  Modes: console, redirect, both\n");
        return;
    }
    
    const char* mode_str = tokens[1];
    LogOutputMode mode;
    
    if (strcmp(mode_str, "console") == 0) {
        mode = LOG_MODE_CONSOLE;
    }
    else if (strcmp(mode_str, "redirect") == 0) {
        mode = LOG_MODE_REDIRECT;
    }
    else if (strcmp(mode_str, "both") == 0) {
        mode = LOG_MODE_BOTH;
    }
    else {
        demo_tcp_cli_print("ERROR: Unknown mode: %s\n", mode_str);
        demo_tcp_cli_print("Valid modes: console, redirect, both\n");
        return;
    }
    
    demo_log_set_mode(mode);
    demo_tcp_cli_print("OK: Log mode set to '%s'\n", demo_log_mode_name(mode));
}

static void cmd_log_status(void) {
    LogOutputMode mode = demo_log_get_mode();
    demo_tcp_cli_print("Current log mode: %s\n", demo_log_mode_name(mode));
    
    int tcp_client = demo_log_get_tcp_client();
    if (tcp_client >= 0) {
        demo_tcp_cli_print("Log TCP client: Connected\n");
    } else {
        demo_tcp_cli_print("Log TCP client: Not connected\n");
    }
}

static void cmd_set_hz(int token_count, char** tokens) {
    if (!g_demo_ctx) {
        demo_tcp_cli_print("ERROR: DemoApp context not initialized\n");
        return;
    }

    if (token_count < 3) {
        demo_tcp_cli_print("Usage: set_hz <topic> <hz>\n");
        demo_tcp_cli_print("  Topics: signal, cbit, pbit\n");
        return;
    }

    const char* topic = tokens[1];
    uint32_t hz = (uint32_t)atoi(tokens[2]);

    if (demo_app_set_publish_hz(g_demo_ctx, topic, hz) == 0) {
        demo_tcp_cli_print("OK: %s publish rate set to %u Hz\n", topic, hz);
    } else {
        demo_tcp_cli_print("ERROR: Failed to set hz for topic '%s'\n", topic);
    }
}

static void cmd_reset_hz(void) {
    if (!g_demo_ctx) {
        demo_tcp_cli_print("ERROR: DemoApp context not initialized\n");
        return;
    }
    demo_app_reset_publish_periods(g_demo_ctx);
    demo_tcp_cli_print("OK: Publish periods reset to defaults\n");
}

/* ========================================================================
 * Main Command Processor
 * ======================================================================== */

void demo_cli_process_command(char* line) {
    char* tokens[8];
    int token_count = tokenize_command(line, tokens, 8);
    
    if (token_count == 0) return;
    
    const char* cmd = tokens[0];
    
    // Command dispatch
    if (strcmp(cmd, "help") == 0) {
        cmd_help();
    }
    else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
        demo_tcp_cli_print("Closing CLI connection...\n");
        demo_cli_stop();
        return;
    }
    else if (strcmp(cmd, "status") == 0) {
        cmd_status();
    }
    else if (strcmp(cmd, "connect") == 0) {
        cmd_connect(token_count, tokens);
    }
    else if (strcmp(cmd, "reset") == 0) {
        cmd_reset();
    }
    else if (strcmp(cmd, "create_entities") == 0) {
        cmd_create_entities();
    }
    else if (strcmp(cmd, "start_scenario") == 0) {
        cmd_start_scenario();
    }
    else if (strcmp(cmd, "stop_scenario") == 0) {
        cmd_stop_scenario();
    }
    else if (strcmp(cmd, "test_write") == 0) {
        cmd_test_write(token_count, tokens);
    }
    else if (strcmp(cmd, "run_ibit") == 0) {
        cmd_run_ibit(token_count, tokens);
    }
    else if (strcmp(cmd, "fault_inject") == 0) {
        cmd_fault_inject(token_count, tokens);
    }
    else if (strcmp(cmd, "fault_clear") == 0) {
        cmd_fault_clear(token_count, tokens);
    }
    else if (strcmp(cmd, "log_mode") == 0) {
        cmd_log_mode(token_count, tokens);
    }
    else if (strcmp(cmd, "log_status") == 0) {
        cmd_log_status();
    }
    else if (strcmp(cmd, "set_hz") == 0) {
        cmd_set_hz(token_count, tokens);
    }
    else if (strcmp(cmd, "reset_hz") == 0) {
        cmd_reset_hz();
    }
    else {
        demo_tcp_cli_print("Unknown command: %s\n", cmd);
        demo_tcp_cli_print("Type 'help' for available commands\n");
    }
}

/* ========================================================================
 * Public Interface Functions
 * ======================================================================== */

/**
 * Initialize CLI system
 * Called from TCP server or main initialization
 */
int demo_cli_init(void) {
    // CLI system has no internal state - just command processing
    return 0;
}

/**
 * Cleanup CLI system
 */
void demo_cli_cleanup(void) {
    // Nothing to cleanup
}

/**
 * Start CLI server (wrapper for TCP CLI server)
 * @param port TCP port number for CLI server
 * @return 0 on success, -1 on failure
 */
int demo_cli_start(int port) {
    return demo_tcp_cli_start(port);
}

/**
 * Stop CLI server (wrapper for TCP CLI server)
 */
void demo_cli_stop(void) {
    demo_tcp_cli_stop();
}

/**
 * Print to CLI client (wrapper for TCP CLI print)
 * @param fmt printf-style format string
 */
void demo_cli_print(const char* fmt, ...) {
    char buffer[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    demo_tcp_cli_print("%s", buffer);
}
