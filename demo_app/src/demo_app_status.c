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
#include "../include/demo_app_publisher.h"

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
        /* Status should always print to console regardless of log enabled */
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

    double measured_hz = 0.0;
    double theoretical_hz = 0.0;

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

        /* prepare measured rates for status output */
        if (diff_ms > 0 && g_demo_ctx->pub_signal_count > 0) {
            double elapsed_s = (double)diff_ms / 1000.0;
            measured_hz = (double)g_demo_ctx->pub_signal_count / elapsed_s;
        }
        if (g_demo_ctx->pub_signal_count) {
            long long sig_avg_ms = 0;
            if (g_demo_ctx->pub_signal_count) sig_avg_ms = (long long)(g_demo_ctx->pub_signal_ns_total / g_demo_ctx->pub_signal_count / 1000000ULL);
            if (sig_avg_ms > 0) theoretical_hz = 1000.0 / (double)sig_avg_ms;
        }
    } else {
        status_print(to_tcp, "Scenario start time: (not started)\n");
    }

    status_print(to_tcp, "\nStatistics:\n");
    status_print(to_tcp, "  Signal Published: %u (%u Hz)\n", g_demo_ctx->signal_pub_count, g_demo_ctx->signal_pub_hz);
    status_print(to_tcp, "  CBIT Published: %u (%u Hz)\n", g_demo_ctx->cbit_pub_count, g_demo_ctx->cbit_pub_hz);
    status_print(to_tcp, "  Control Received: %u (%u Hz)\n", g_demo_ctx->control_rx_count, g_demo_ctx->control_rx_hz);
    status_print(to_tcp, "  Speed Received: %u (%u Hz)\n", g_demo_ctx->speed_rx_count, g_demo_ctx->speed_rx_hz);

#ifdef DEMO_PERF_INSTRUMENTATION
    if (g_demo_ctx->pub_signal_count || g_demo_ctx->json_dump_count || g_demo_ctx->legacy_write_count) {
        long long sig_avg_ms = 0;
        if (g_demo_ctx->pub_signal_count) sig_avg_ms = (long long)(g_demo_ctx->pub_signal_ns_total / g_demo_ctx->pub_signal_count / 1000000ULL);
        long long json_avg_ms = 0;
        if (g_demo_ctx->json_dump_count) json_avg_ms = (long long)(g_demo_ctx->json_dump_ns_total / g_demo_ctx->json_dump_count / 1000000ULL);
        long long lw_avg_ms = 0;
        if (g_demo_ctx->legacy_write_count) lw_avg_ms = (long long)(g_demo_ctx->legacy_write_ns_total / g_demo_ctx->legacy_write_count / 1000000ULL);

        status_print(to_tcp, "\nPerformance (perf instrumentation):\n");
        {
            double sig_hz = 0.0;
            if (sig_avg_ms > 0) sig_hz = 1000.0 / (double)sig_avg_ms;
            status_print(to_tcp, "  Signal publish: count=%u, avg=%lld ms    %.1f Hz\n", g_demo_ctx->pub_signal_count, sig_avg_ms, sig_hz);
            if (g_demo_ctx->scenario_started && measured_hz > 0.0) {
                status_print(to_tcp, "    Measured signal rate: %.1f Hz (count/elapsed)\n", measured_hz);
            }
            if (theoretical_hz > 0.0) {
                status_print(to_tcp, "    Theoretical (from avg): %.1f Hz (1/avg_ms)\n", theoretical_hz);
            }
            double json_hz = 0.0;
            if (json_avg_ms > 0) json_hz = 1000.0 / (double)json_avg_ms;
            status_print(to_tcp, "  JSON dump: count=%u, avg=%lld ms    %.1f Hz\n", g_demo_ctx->json_dump_count, json_avg_ms, json_hz);
            double lw_hz = 0.0;
            if (lw_avg_ms > 0) lw_hz = 1000.0 / (double)lw_avg_ms;
            status_print(to_tcp, "  legacy write: count=%u, avg=%lld ms    %.1f Hz\n", g_demo_ctx->legacy_write_count, lw_avg_ms, lw_hz);
        }
    }

    /* Library-side perf (from legacy_agent / internal client+transport) */
    if (g_demo_ctx->agent) {
        LegacyPerfStats ps;
        if (legacy_agent_get_perf_stats(g_demo_ctx->agent, &ps) == LEGACY_OK) {
            long long parse_avg_ms = 0;
            long long cbor_avg_ms = 0;
            long long trans_avg_ms = 0;
            if (ps.ipc_parse_count) parse_avg_ms = (long long)(ps.ipc_parse_ns_total / ps.ipc_parse_count / 1000000ULL);
            if (ps.ipc_cbor_count) cbor_avg_ms = (long long)(ps.ipc_cbor_ns_total / ps.ipc_cbor_count / 1000000ULL);
            if (ps.transport_send_count) trans_avg_ms = (long long)(ps.transport_send_us_total / ps.transport_send_count / 1000ULL);
            status_print(to_tcp, "\nLibrary Performance (legacy lib):\n");
            {
                double parse_hz = 0.0;
                double cbor_hz = 0.0;
                double trans_hz = 0.0;
                if (parse_avg_ms > 0) parse_hz = 1000.0 / (double)parse_avg_ms;
                if (cbor_avg_ms > 0) cbor_hz = 1000.0 / (double)cbor_avg_ms;
                if (trans_avg_ms > 0) trans_hz = 1000.0 / (double)trans_avg_ms;
                status_print(to_tcp, "  IPC parse: count=%u, avg=%lld ms    %.1f Hz\n", ps.ipc_parse_count, parse_avg_ms, parse_hz);
                status_print(to_tcp, "  IPC to_cbor: count=%u, avg=%lld ms    %.1f Hz\n", ps.ipc_cbor_count, cbor_avg_ms, cbor_hz);
                status_print(to_tcp, "  Transport send: count=%u, avg=%lld ms    %.1f Hz\n", ps.transport_send_count, trans_avg_ms, trans_hz);
            long long write_avg_ms = 0;
            if (ps.write_count) write_avg_ms = (long long)(ps.write_ns_total / ps.write_count / 1000000ULL);
            status_print(to_tcp, "  Library write: count=%u, avg=%lld ms\n", ps.write_count, write_avg_ms);
            /* Publisher worker dequeue->write timing (app-side) */
            long long worker_write_avg_ms = 0;
            if (g_demo_ctx->pub_worker_write_count) worker_write_avg_ms = (long long)(g_demo_ctx->pub_worker_write_ns_total / g_demo_ctx->pub_worker_write_count / 1000000ULL);
            status_print(to_tcp, "  Pub worker write: count=%u, avg=%lld ms\n", g_demo_ctx->pub_worker_write_count, worker_write_avg_ms);
        }
    }

    /* Publisher queue stats */
    {
        int qlen = 0, qmax = 0; uint64_t qdrop = 0;
        if (demo_publisher_get_stats(&qlen, &qmax, &qdrop) == 0) {
            status_print(to_tcp, "\nPublisher Queue:\n");
            status_print(to_tcp, "  Queue length: %d / %d\n", qlen, qmax);
            status_print(to_tcp, "  Dropped events: %llu\n", (unsigned long long)qdrop);
        }
    }
#endif /* DEMO_PERF_INSTRUMENTATION */

    status_print(to_tcp, "\nBIT State:\n");
    status_print(to_tcp, "  PBIT Completed: %s\n", g_demo_ctx->bit_state.pbit_completed ? "Yes" : "No");
    status_print(to_tcp, "  CBIT Active: %s\n", g_demo_ctx->bit_state.cbit_active ? "Yes" : "No");
    status_print(to_tcp, "  IBIT Running: %s\n", g_demo_ctx->bit_state.ibit_running ? "Yes" : "No");

    status_print(to_tcp, "\nComponent Status:\n");
    status_print(to_tcp, "  Round Motor: %s\n", g_demo_ctx->bit_state.pbit_components.roundMotor == L_BITResultType_NORMAL ? "OK" : "FAULT");
    status_print(to_tcp, "  UpDown Motor: %s\n", g_demo_ctx->bit_state.pbit_components.upDownMotor == L_BITResultType_NORMAL ? "OK" : "FAULT");
    status_print(to_tcp, "  Base Gyro: %s\n", g_demo_ctx->bit_state.pbit_components.baseGyro == L_BITResultType_NORMAL ? "OK" : "FAULT");
    status_print(to_tcp, "  Power Controller: %s\n", g_demo_ctx->bit_state.pbit_components.powerController == L_BITResultType_NORMAL ? "OK" : "FAULT");
    status_print(to_tcp, "======================\n");
}
