/*
 * demo_app_status.c - Shared status printing helper
 *
 * Provides `demo_app_print_status(int to_tcp)` so both the VxWorks
 * shell command and the TCP CLI `status` command print identical output.
 */

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#if !defined(_WIN32) && !defined(_WIN64)
#include <sys/time.h>
#endif

#include "../include/demo_app.h"
#include "../include/demo_app_tcp.h"
#include "../include/demo_app_log.h"

/* Externs declared in demo_app_dkm.c */
extern DemoAppContext* g_demo_ctx;
extern int g_cli_port;
extern char g_agent_ip[];

static void status_print(int to_tcp, const char* fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (to_tcp) {
        demo_tcp_cli_print("%s", buf);
    } else {
        printf("%s", buf);
    }
}

void demo_app_print_status(int to_tcp) {
    if (!g_demo_ctx) {
        status_print(to_tcp, "[DemoApp] Not running\n");
        status_print(to_tcp, "Use: demoAppStart(23000, \"127.0.0.1\")\n");
        return;
    }

    status_print(to_tcp, "\n=== DemoApp Status ===\n");
    status_print(to_tcp, "State: %s\n", demo_state_name(g_demo_ctx->current_state));
    status_print(to_tcp, "CLI Port: %d\n", g_cli_port);
    status_print(to_tcp, "Agent IP: %s\n", g_agent_ip);
    status_print(to_tcp, "Tick Count: %llu\n", (unsigned long long)g_demo_ctx->tick_count);

    if (g_demo_ctx->scenario_started) {
        char start_buf[64] = {0};
        time_t s = (time_t)g_demo_ctx->scenario_start_time.A_second;
        int ms = g_demo_ctx->scenario_start_time.A_nanoseconds / 1000000;
        struct tm tm_buf;
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm_buf, &s);
#else
        localtime_r(&s, &tm_buf);
#endif
        snprintf(start_buf, sizeof(start_buf), "%04d.%02d.%02d.%02d.%02d.%02d.%03d",
                 tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
                 tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec, ms);

        time_t now_s = time(NULL);
        int now_ms = 0;
#if defined(_WIN32) || defined(_WIN64)
        /* best-effort: no sub-second precision on Windows here */
#else
        struct timeval tv;
        if (gettimeofday(&tv, NULL) == 0) {
            now_s = tv.tv_sec;
            now_ms = (int)(tv.tv_usec / 1000);
        }
#endif
        char now_buf[64] = {0};
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm_buf, &now_s);
#else
        localtime_r(&now_s, &tm_buf);
#endif
        snprintf(now_buf, sizeof(now_buf), "%04d.%02d.%02d.%02d.%02d.%02d.%03d",
                 tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
                 tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec, now_ms);

        int64_t start_ms = ((int64_t)g_demo_ctx->scenario_start_time.A_second) * 1000 + (g_demo_ctx->scenario_start_time.A_nanoseconds / 1000000);
        int64_t cur_ms = ((int64_t)now_s) * 1000 + now_ms;
        int64_t diff_ms = cur_ms - start_ms;

        status_print(to_tcp, "startTime : %s\n", start_buf);
        status_print(to_tcp, "statusTime : %s ( +%lld ms )\n", now_buf, (long long)diff_ms);
    } else {
        status_print(to_tcp, "Scenario start time: (not started)\n");
    }

    status_print(to_tcp, "\nStatistics:\n");
    status_print(to_tcp, "  Signal Published: %u\n", g_demo_ctx->signal_pub_count);
    status_print(to_tcp, "  CBIT Published: %u\n", g_demo_ctx->cbit_pub_count);
    status_print(to_tcp, "  Control Received: %u\n", g_demo_ctx->control_rx_count);
    status_print(to_tcp, "  Speed Received: %u\n", g_demo_ctx->speed_rx_count);

    if (g_demo_ctx->pub_signal_count || g_demo_ctx->json_dump_count || g_demo_ctx->legacy_write_count) {
        long long sig_avg_ms = 0;
        if (g_demo_ctx->pub_signal_count) sig_avg_ms = (long long)(g_demo_ctx->pub_signal_ns_total / g_demo_ctx->pub_signal_count / 1000000ULL);
        long long json_avg_ms = 0;
        if (g_demo_ctx->json_dump_count) json_avg_ms = (long long)(g_demo_ctx->json_dump_ns_total / g_demo_ctx->json_dump_count / 1000000ULL);
        long long lw_avg_ms = 0;
        if (g_demo_ctx->legacy_write_count) lw_avg_ms = (long long)(g_demo_ctx->legacy_write_ns_total / g_demo_ctx->legacy_write_count / 1000000ULL);

        status_print(to_tcp, "\nPerformance (perf instrumentation):\n");
        status_print(to_tcp, "  Signal publish: count=%u, avg=%lld ms\n", g_demo_ctx->pub_signal_count, sig_avg_ms);
        status_print(to_tcp, "  JSON dump: count=%u, avg=%lld ms\n", g_demo_ctx->json_dump_count, json_avg_ms);
        status_print(to_tcp, "  legacy write: count=%u, avg=%lld ms\n", g_demo_ctx->legacy_write_count, lw_avg_ms);
    }

    status_print(to_tcp, "\nBIT State:\n");
    status_print(to_tcp, "  PBIT Completed: %s\n", g_demo_ctx->bit_state.pbit_completed ? "Yes" : "No");
    status_print(to_tcp, "  CBIT Active: %s\n", g_demo_ctx->bit_state.cbit_active ? "Yes" : "No");
    status_print(to_tcp, "  IBIT Running: %s\n", g_demo_ctx->bit_state.ibit_running ? "Yes" : "No");

    status_print(to_tcp, "\nComponent Status:\n");
    status_print(to_tcp, "  Round Motor: %s\n", g_demo_ctx->bit_state.pbit_components.roundMotor == L_BITResultType_NORMAL ? "OK" : "FAULT");
    status_print(to_tcp, "  UpDown Motor: %s\n", g_demo_ctx->bit_state.pbit_components.upDownMotor == L_BITResultType_NORMAL ? "OK" : "FAULT");
    status_print(to_tcp, "  Base Giro: %s\n", g_demo_ctx->bit_state.pbit_components.baseGiro == L_BITResultType_NORMAL ? "OK" : "FAULT");
    status_print(to_tcp, "  Power Controller: %s\n", g_demo_ctx->bit_state.pbit_components.powerController == L_BITResultType_NORMAL ? "OK" : "FAULT");
    status_print(to_tcp, "======================\n");
}
