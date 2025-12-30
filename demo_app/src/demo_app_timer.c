/*
 * demo_app_timer.c - Periodic Timer and Scheduling
 */

#include "../include/demo_app.h"
#include "../include/demo_app_log.h"
#include <stdio.h>
#include <time.h>
#include "../include/demo_app_publisher.h"

#ifdef _VXWORKS_
#include <vxWorks.h>
#include <taskLib.h>
#include <sysLib.h>
#include <wdLib.h>
#include <tickLib.h>
#else
// Windows/Linux threading
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <mmsystem.h>  // For timeBeginPeriod/timeEndPeriod
#pragma comment(lib, "winmm.lib")
#else
#include <pthread.h>
#include <unistd.h>
#endif
#endif

/* ========================================================================
 * Global State
 * ======================================================================== */

#ifdef _VXWORKS_
static WDOG_ID g_timer_wdog = NULL;
static TASK_ID g_timer_task = TASK_ID_ERROR;
static volatile int g_timer_running = 0;
#else
// Windows/Linux
#ifdef _WIN32
static HANDLE g_timer_thread = NULL;
static volatile int g_timer_running = 0;
#else
static pthread_t g_timer_thread;
static volatile int g_timer_running = 0;
#endif
#endif

extern DemoAppContext* g_demo_ctx;

/* ========================================================================
 * Timer Task (VxWorks)
 * ======================================================================== */

#ifdef _VXWORKS_
static void timerTask(void) {
    int tick_rate = sysClkRateGet();  // System ticks per second
    int delay_ticks = 0; // 1ms in ticks (computed below)

    // Compute delay_ticks as ticks representing ~1ms. Use ceil to avoid 0.
    if (tick_rate > 0) {
        delay_ticks = (tick_rate + 999) / 1000; // ceil(tick_rate/1000)
    }
        if (delay_ticks < 1) {
         delay_ticks = 1;
         demo_log(LOG_DIR_INFO, "[DemoApp Timer] WARNING: System tick rate is %d Hz (<%d Hz required for 1ms)\n",
               tick_rate, 1000);
         demo_log(LOG_DIR_INFO, "[DemoApp Timer] Timer will run at %d ms interval instead\n",
               1000 / (tick_rate > 0 ? tick_rate : 1));
        }
    
        demo_log(LOG_DIR_INFO, "[DemoApp Timer] Timer task started\n");
        demo_log(LOG_DIR_INFO, "[DemoApp Timer]   System tick rate: %d Hz\n", tick_rate);
        demo_log(LOG_DIR_INFO, "[DemoApp Timer]   Delay ticks: %d (%.1f ms)\n", 
              delay_ticks, (float)delay_ticks * 1000.0f / tick_rate);
    
    while (g_timer_running) {
        if (g_demo_ctx) {
#ifdef DEMO_PERF_INSTRUMENTATION
            uint64_t t0 = 0;
#ifdef _VXWORKS_
            t0 = (uint64_t)tickGet() * (1000000000ULL / (tick_rate > 0 ? tick_rate : 1));
#else
            struct timespec ts0; clock_gettime(CLOCK_MONOTONIC, &ts0); t0 = (uint64_t)ts0.tv_sec*1000000000ULL + ts0.tv_nsec;
#endif
            demo_timer_tick(g_demo_ctx);
            uint64_t t1 = 0;
#ifdef _VXWORKS_
            t1 = (uint64_t)tickGet() * (1000000000ULL / (tick_rate > 0 ? tick_rate : 1));
#else
            struct timespec ts1; clock_gettime(CLOCK_MONOTONIC, &ts1); t1 = (uint64_t)ts1.tv_sec*1000000000ULL + ts1.tv_nsec;
#endif
            demo_log(LOG_DIR_INFO, "[TIMING] demo_timer_tick took %llu us (ticks=%d)\n", (unsigned long long)((t1 - t0) / 1000ULL), delay_ticks);
#else
            demo_timer_tick(g_demo_ctx);
#endif
        }

        taskDelay(delay_ticks);
    }
    
    demo_log(LOG_DIR_INFO, "[DemoApp Timer] Timer task exiting\n");
}
#else
// Windows/Linux Timer Thread
#ifdef _WIN32
static unsigned int __stdcall timerThreadFunc(void* arg) {
    (void)arg;
    
    // Set Windows timer resolution to 1ms
    TIMECAPS tc;
    timeGetDevCaps(&tc, sizeof(TIMECAPS));
    UINT wTimerRes = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
    timeBeginPeriod(wTimerRes);
    
    demo_log(LOG_DIR_INFO, "[DemoApp Timer] Timer thread started (1ms interval, resolution=%ums)\n", wTimerRes);
    
    LARGE_INTEGER freq, start, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    
    uint64_t tick = 0;
    
    while (g_timer_running) {
        // Calculate elapsed time in ms
        QueryPerformanceCounter(&now);
        uint64_t elapsed_ms = ((now.QuadPart - start.QuadPart) * 1000) / freq.QuadPart;
        
        // Process all ticks that should have happened
        while (tick < elapsed_ms && g_timer_running) {
            if (g_demo_ctx) {
                demo_timer_tick(g_demo_ctx);
            }
            tick++;
        }
        
        // Sleep for remaining time (at least 1ms)
        Sleep(1);
    }
    
    timeEndPeriod(wTimerRes);
    demo_log(LOG_DIR_INFO, "[DemoApp Timer] Timer thread exiting\n");
    return 0;
}
#else
static void* timerThreadFunc(void* arg) {
    (void)arg;
    demo_log(LOG_DIR_INFO, "[DemoApp Timer] Timer thread started (1ms interval)\n");
    
    while (g_timer_running) {
        if (g_demo_ctx) {
            demo_timer_tick(g_demo_ctx);
        }
        usleep(1000);  // 1ms = 1000us
    }
    
    demo_log(LOG_DIR_INFO, "[DemoApp Timer] Timer thread exiting\n");
    return NULL;
}
#endif
#endif

/* ========================================================================
 * Timer Initialization
 * ======================================================================== */

int demo_timer_init(DemoAppContext* ctx) {
    if (!ctx) return -1;
    
    demo_log(LOG_DIR_INFO, "[DemoApp Timer] Initializing timer subsystem...\n");
    
    ctx->tick_count = 0;
    ctx->last_200hz_tick = 0;
    ctx->last_1hz_tick = 0;
    
#ifdef _VXWORKS_
    // Start timer task
    g_timer_running = 1;
    
    g_timer_task = taskSpawn(
        "tDemoTimer",
        90,         // High priority for timing
        0,
        32768,
        (FUNCPTR)timerTask,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    );
    
    if (g_timer_task == TASK_ID_ERROR) {
        demo_log(LOG_DIR_INFO, "[DemoApp Timer] ERROR: Failed to spawn timer task\n");
        g_timer_running = 0;
        return -1;
    }
    
    demo_log(LOG_DIR_INFO, "[DemoApp Timer] Timer task spawned (taskId=%d)\n", g_timer_task);
#else
    // Windows/Linux thread
    g_timer_running = 1;
    
#ifdef _WIN32
    g_timer_thread = (HANDLE)_beginthreadex(NULL, 0, timerThreadFunc, NULL, 0, NULL);
    if (g_timer_thread == NULL) {
        demo_log(LOG_DIR_INFO, "[DemoApp Timer] ERROR: Failed to create timer thread\n");
        g_timer_running = 0;
        return -1;
    }
    demo_log(LOG_DIR_INFO, "[DemoApp Timer] Timer thread created\n");
#else
    if (pthread_create(&g_timer_thread, NULL, timerThreadFunc, NULL) != 0) {
        demo_log(LOG_DIR_INFO, "[DemoApp Timer] ERROR: Failed to create timer thread\n");
        g_timer_running = 0;
        return -1;
    }
    demo_log(LOG_DIR_INFO, "[DemoApp Timer] Timer thread created\n");
#endif
#endif
    
    demo_log(LOG_DIR_INFO, "[DemoApp Timer] Timer initialized\n");
    // Start publisher (queue + worker)
    demo_publisher_init();
    return 0;
}

void demo_timer_cleanup(DemoAppContext* ctx) {
    if (!ctx) return;
    
    demo_log(LOG_DIR_INFO, "[DemoApp Timer] Cleaning up timer subsystem...\n");
    
    g_timer_running = 0;
    
#ifdef _VXWORKS_
    if (g_timer_task != TASK_ID_ERROR) {
        taskDelay(sysClkRateGet() / 10);  // 100ms
        
        if (taskIdVerify(g_timer_task) == OK) {
            taskDelete(g_timer_task);
        }
        g_timer_task = TASK_ID_ERROR;
    }
    
    if (g_timer_wdog) {
        wdDelete(g_timer_wdog);
        g_timer_wdog = NULL;
    }
#else
    // Windows/Linux
#ifdef _WIN32
    if (g_timer_thread) {
        WaitForSingleObject(g_timer_thread, 1000);  // Wait 1 second
        CloseHandle(g_timer_thread);
        g_timer_thread = NULL;
    }
#else
    pthread_join(g_timer_thread, NULL);
#endif
#endif
    
    demo_log(LOG_DIR_INFO, "[DemoApp Timer] Timer cleanup complete\n");
    // Shutdown publisher
    demo_publisher_shutdown();
}

/* ========================================================================
 * Timer Status Query
 * ======================================================================== */

int demo_timer_is_running(void) {
    return g_timer_running;
}

/* ========================================================================
 * Tick Handler (Called every 1ms)
 * ======================================================================== */

void demo_timer_tick(DemoAppContext* ctx) {
    if (!ctx) return;
    
    ctx->tick_count++;
    
    // Update Hz statistics every 1 second (1000ms)
    if ((ctx->tick_count - ctx->stats_last_tick) >= 1000) {
        ctx->signal_pub_hz = ctx->signal_pub_count - ctx->signal_pub_prev;
        ctx->cbit_pub_hz = ctx->cbit_pub_count - ctx->cbit_pub_prev;
        ctx->pbit_pub_hz = ctx->pbit_pub_count - ctx->pbit_pub_prev;
        ctx->result_pub_hz = ctx->result_pub_count - ctx->result_pub_prev;
        ctx->control_rx_hz = ctx->control_rx_count - ctx->control_rx_prev;
        ctx->speed_rx_hz = ctx->speed_rx_count - ctx->speed_rx_prev;
        ctx->runbit_rx_hz = ctx->runbit_rx_count - ctx->runbit_rx_prev;
        
        ctx->signal_pub_prev = ctx->signal_pub_count;
        ctx->cbit_pub_prev = ctx->cbit_pub_count;
        ctx->pbit_pub_prev = ctx->pbit_pub_count;
        ctx->result_pub_prev = ctx->result_pub_count;
        ctx->control_rx_prev = ctx->control_rx_count;
        ctx->speed_rx_prev = ctx->speed_rx_count;
        ctx->runbit_rx_prev = ctx->runbit_rx_count;
        
        ctx->stats_last_tick = ctx->tick_count;
    }
    
    // Handle IBIT execution (IBitRunning state)
    if (ctx->current_state == DEMO_STATE_IBIT_RUNNING) {
        uint64_t elapsed = ctx->tick_count - ctx->bit_state.ibit_start_time;
        
        // IBIT duration: 3 seconds (3000ms)
        if (elapsed >= 3000) {
            demo_log(LOG_DIR_INFO, "[DemoApp Timer] IBIT completed after %llu ms\n", elapsed);
            
            // Publish resultBIT
            demo_msg_publish_result_bit(ctx);
            
            // Clear IBIT state
            ctx->bit_state.ibit_running = false;
            
            // Transition back to Run state
            extern void enter_state(DemoAppContext* ctx, DemoState new_state);
            enter_state(ctx, DEMO_STATE_RUN);
            
            demo_log(LOG_DIR_INFO, "[DemoApp Timer] Returned to Run state\n");
        }
        
        return;  // Don't publish periodic messages during IBIT
    }
    
    // Only process in Run state
    if (ctx->current_state != DEMO_STATE_RUN) {
        return;
    }
    
    // Update simulation (position/velocity)
    demo_timer_update_simulation(ctx);
    
    // Signal processing (configurable period)
    if (ctx->signal_period_ms > 0) {
        if ((ctx->tick_count - ctx->last_200hz_tick) >= ctx->signal_period_ms) {
            /* Measure publish duration if instrumentation enabled */
#ifdef DEMO_PERF_INSTRUMENTATION
            uint64_t t0 = 0, t1 = 0;
            /* now_ns: VxWorks use tickGet conversion, otherwise clock_gettime */
#if defined(_VXWORKS_)
            {
                unsigned long tk = tickGet();
                int tr = sysClkRateGet();
                t0 = (uint64_t)tk * (1000000000ULL / (tr > 0 ? tr : 1));
            }
#else
            struct timespec ts0; clock_gettime(CLOCK_MONOTONIC, &ts0); t0 = (uint64_t)ts0.tv_sec*1000000000ULL + ts0.tv_nsec;
#endif
            // Enqueue publish to publisher task to avoid blocking timer
            if (demo_publisher_enqueue_signal(ctx) != 0) {
                // If enqueue failed (queue full), optionally drop and log
                demo_log(LOG_DIR_INFO, "[Timer] Publisher queue full, dropped signal event\n");
            }
#if defined(_VXWORKS_)
            {
                unsigned long tk = tickGet();
                int tr = sysClkRateGet();
                t1 = (uint64_t)tk * (1000000000ULL / (tr > 0 ? tr : 1));
            }
#else
            struct timespec ts1; clock_gettime(CLOCK_MONOTONIC, &ts1); t1 = (uint64_t)ts1.tv_sec*1000000000ULL + ts1.tv_nsec;
#endif
            ctx->pub_signal_ns_total += (t1 > t0) ? (t1 - t0) : 0ULL;
            ctx->pub_signal_count++;
#else
            if (demo_publisher_enqueue_signal(ctx) != 0) {
                demo_log(LOG_DIR_INFO, "[Timer] Publisher queue full, dropped signal event\n");
            }
#endif
            ctx->last_200hz_tick = ctx->tick_count;
        }
    }

    // CBIT processing (configurable period)
    if (ctx->cbit_period_ms > 0) {
        if ((ctx->tick_count - ctx->last_1hz_tick) >= ctx->cbit_period_ms) {
            /* Measure CBIT publish duration */
#ifdef DEMO_PERF_INSTRUMENTATION
            uint64_t t0 = 0, t1 = 0;
#if defined(_VXWORKS_)
            {
                unsigned long tk = tickGet();
                int tr = sysClkRateGet();
                t0 = (uint64_t)tk * (1000000000ULL / (tr > 0 ? tr : 1));
            }
#else
            struct timespec ts0; clock_gettime(CLOCK_MONOTONIC, &ts0); t0 = (uint64_t)ts0.tv_sec*1000000000ULL + ts0.tv_nsec;
#endif
            demo_msg_publish_cbit(ctx);
#if defined(_VXWORKS_)
            {
                unsigned long tk = tickGet();
                int tr = sysClkRateGet();
                t1 = (uint64_t)tk * (1000000000ULL / (tr > 0 ? tr : 1));
            }
#else
            struct timespec ts1; clock_gettime(CLOCK_MONOTONIC, &ts1); t1 = (uint64_t)ts1.tv_sec*1000000000ULL + ts1.tv_nsec;
#endif
            ctx->pub_cbit_ns_total += (t1 > t0) ? (t1 - t0) : 0ULL;
            ctx->pub_cbit_count++;
#else
            demo_msg_publish_cbit(ctx);
#endif
            ctx->last_1hz_tick = ctx->tick_count;
        }
    }
}

/* ========================================================================
 * Simulation Update (Called every 1ms)
 * ======================================================================== */

void demo_timer_update_simulation(DemoAppContext* ctx) {
    if (!ctx) return;
    
    // Simple position integration from velocity commands
    // dt = 1ms = 0.001s
    const float dt = 0.001f;
    
    ActuatorControlState* ctrl = &ctx->control_state;
    ActuatorSignalState* sig = &ctx->signal_state;
    
    // Update azimuth (roundAngle) based on velocity command
    if (ctrl->roundAngleVelocity != 0.0f) {
        // Direct velocity control
        sig->e1AngleVelocity = ctrl->roundAngleVelocity;
        sig->azAngle += sig->e1AngleVelocity * dt;
    } else {
        // Position control - move towards A_drivingPosition
        float error = ctrl->drivingPosition - sig->azAngle;
        if (error > 0.01f || error < -0.01f) {
            // Simple proportional control (P = 1.0)
            sig->e1AngleVelocity = error * 1.0f;
            // Limit velocity
            if (sig->e1AngleVelocity > 10.0f) sig->e1AngleVelocity = 10.0f;
            if (sig->e1AngleVelocity < -10.0f) sig->e1AngleVelocity = -10.0f;
            sig->azAngle += sig->e1AngleVelocity * dt;
        } else {
            sig->e1AngleVelocity = 0.0f;
        }
    }
    
    // Update giro values (simplified - just copy velocity)
    sig->roundGiro = sig->e1AngleVelocity;
    sig->upDownGiro = ctrl->upDownAngleVelocity;
    
    // Update enum status fields based on component faults
    // Set to NORMAL if no faults, otherwise indicate fault conditions
    BITComponentState* pbit = &ctx->bit_state.pbit_components;
    
    sig->energyStorage = (pbit->energyStorage == L_BITResultType_NORMAL) ? 
        L_ChangingStatusType_DISCHARGE : L_ChangingStatusType_NORMAL;
    sig->mainCannonFixStatus = (pbit->roundMotor == L_BITResultType_NORMAL) ? 
        L_MainCannonFixStatusType_FIX : L_MainCannonFixStatusType_NORMAL;
    sig->deckClearance = L_DekClearanceType_OUTSIDE;
    sig->autoArmPositionComplement = L_ArmPositionType_NORMAL;
    sig->manualArmPositionComple = L_ArmPositionType_NORMAL;
    sig->mainCannonRestoreComplement = L_MainCannonReturnStatusType_RUNNING;
    sig->armSafetyMainCannonLock = L_ArmSafetyMainCannonLock_NORMAL;
    sig->shutdown = L_CannonDrivingDeviceShutdownType_UNKNOWN;
}
