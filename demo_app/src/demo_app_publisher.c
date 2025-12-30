/* demo_app_publisher.c - Publisher queue and worker
 * Timer enqueues publish events; worker performs legacy_agent_write_json calls
 * to avoid blocking timer context.
 */

#include "../include/demo_app_publisher.h"
#include "../include/demo_app_log.h"
#include "../include/demo_app.h"

#ifdef _VXWORKS_
#include <msgQLib.h>
#include <taskLib.h>
#include <sysLib.h>
#include <tickLib.h>
#include <string.h>

static MSG_Q_ID g_pubQ = NULL;
static TASK_ID g_pubTask = TASK_ID_ERROR;
static int g_pubq_max = 256;
static uint64_t g_pubq_drop_count = 0;

typedef enum { PUB_EVT_SIGNAL = 1 } PubEventType;
typedef struct {
    PubEventType type;
    DemoAppContext* ctx;
} PubEvent;

static void publisherTask(void) {
    PubEvent evt;
    while (1) {
        int n = msgQReceive(g_pubQ, (char*)&evt, sizeof(evt), WAIT_FOREVER);
        if (n == sizeof(evt)) {
            if (evt.type == PUB_EVT_SIGNAL && evt.ctx) {
                demo_log(LOG_DIR_INFO, "[Publisher] Dequeued signal event\n");
                /* Measure worker dequeue->publish duration when instrumentation enabled */
#ifdef DEMO_PERF_INSTRUMENTATION
                uint64_t _tw0 = 0, _tw1 = 0;
                {
                    unsigned long tk = tickGet(); int tr = sysClkRateGet();
                    _tw0 = (uint64_t)tk * (1000000000ULL / (tr > 0 ? tr : 1));
                }
                demo_msg_publish_actuator_signal(evt.ctx);
                {
                    unsigned long tk = tickGet(); int tr = sysClkRateGet();
                    _tw1 = (uint64_t)tk * (1000000000ULL / (tr > 0 ? tr : 1));
                }
                if (evt.ctx) {
                    evt.ctx->pub_worker_write_ns_total += (_tw1 > _tw0) ? (_tw1 - _tw0) : 0ULL;
                    evt.ctx->pub_worker_write_count++;
                }
#else
                demo_msg_publish_actuator_signal(evt.ctx);
#endif
            }
        } else {
            // queue error or deleted
            break;
        }
    }
}

int demo_publisher_init(void) {
    if (g_pubQ) return 0; // already
    g_pubq_max = 4096;
    g_pubQ = msgQCreate(g_pubq_max, sizeof(PubEvent), MSG_Q_FIFO);
    if (!g_pubQ) {
        demo_log(LOG_DIR_INFO, "[Publisher] ERROR: msgQCreate failed\n");
        return -1;
    }

    g_pubTask = taskSpawn("tDemoPub", 80, 0, 32768, (FUNCPTR)publisherTask, 0,0,0,0,0,0,0,0,0,0);
    if (g_pubTask == TASK_ID_ERROR) {
        demo_log(LOG_DIR_INFO, "[Publisher] ERROR: taskSpawn failed\n");
        msgQDelete(g_pubQ);
        g_pubQ = NULL;
        return -1;
    }
    demo_log(LOG_DIR_INFO, "[Publisher] Started (taskId=%d)\n", g_pubTask);
    return 0;
}

void demo_publisher_shutdown(void) {
    if (g_pubQ) {
        msgQDelete(g_pubQ);
        g_pubQ = NULL;
    }
    // task will exit when msgQReceive fails
}

int demo_publisher_enqueue_signal(DemoAppContext* ctx) {
    if (!g_pubQ) return -1;
    PubEvent e;
    e.type = PUB_EVT_SIGNAL;
    e.ctx = ctx;
    STATUS s = msgQSend(g_pubQ, (char*)&e, sizeof(e), NO_WAIT, MSG_PRI_NORMAL);
    if (s == OK) return 0;
    /* dropped (queue full) */
    g_pubq_drop_count++;
    return -1;
}

int demo_publisher_get_stats(int* out_queue_len, int* out_queue_max, uint64_t* out_drop_count) {
    if (out_queue_len) {
        if (g_pubQ) *out_queue_len = msgQNumMsgs(g_pubQ);
        else *out_queue_len = 0;
    }
    if (out_queue_max) *out_queue_max = g_pubq_max;
    if (out_drop_count) *out_drop_count = g_pubq_drop_count;
    return 0;
}

#else // POSIX / Windows fallback

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <queue>
#include <condition_variable>
#include <mutex>

typedef enum { PUB_EVT_SIGNAL = 1 } PubEventType;
typedef struct {
    PubEventType type;
    DemoAppContext* ctx;
} PubEvent;

static std::queue<PubEvent> g_queue;
static std::mutex g_queue_mutex;
static std::condition_variable g_queue_cv;
static bool g_worker_running = false;
static pthread_t g_worker_thread;
static size_t g_queue_max = 4096;
static uint64_t g_queue_drop_count = 0;

static void* worker_thread_func(void* arg) {
    (void)arg;
    while (1) {
        PubEvent ev;
        {
            std::unique_lock<std::mutex> lk(g_queue_mutex);
            g_queue_cv.wait(lk, []{ return !g_queue.empty() || !g_worker_running; });
            if (!g_worker_running && g_queue.empty()) break;
            ev = g_queue.front(); g_queue.pop();
        }
        if (ev.type == PUB_EVT_SIGNAL && ev.ctx) {
            demo_log(LOG_DIR_INFO, "[Publisher] Dequeued signal event\n");
            /* Measure worker dequeue->publish duration when instrumentation enabled */
#ifdef DEMO_PERF_INSTRUMENTATION
            {
                uint64_t _tw0 = 0, _tw1 = 0;
                struct timespec _t0; clock_gettime(CLOCK_MONOTONIC, &_t0); _tw0 = (uint64_t)_t0.tv_sec*1000000000ULL + _t0.tv_nsec;
                demo_msg_publish_actuator_signal(ev.ctx);
                struct timespec _t1; clock_gettime(CLOCK_MONOTONIC, &_t1); _tw1 = (uint64_t)_t1.tv_sec*1000000000ULL + _t1.tv_nsec;
                if (ev.ctx) {
                    ev.ctx->pub_worker_write_ns_total += (_tw1 > _tw0) ? (_tw1 - _tw0) : 0ULL;
                    ev.ctx->pub_worker_write_count++;
                }
            }
#else
            demo_msg_publish_actuator_signal(ev.ctx);
#endif
        }
    }
    return NULL;
}

int demo_publisher_init(void) {
    if (g_worker_running) return 0;
    g_worker_running = true;
    if (pthread_create(&g_worker_thread, NULL, worker_thread_func, NULL) != 0) {
        g_worker_running = false;
        return -1;
    }
    demo_log(LOG_DIR_INFO, "[Publisher] Worker started\n");
    return 0;
}

void demo_publisher_shutdown(void) {
    {
        std::lock_guard<std::mutex> lk(g_queue_mutex);
        g_worker_running = false;
    }
    g_queue_cv.notify_all();
    if (g_worker_running == false) {
        pthread_join(g_worker_thread, NULL);
    }
}

int demo_publisher_enqueue_signal(DemoAppContext* ctx) {
    if (!g_worker_running) return -1;
    PubEvent e; e.type = PUB_EVT_SIGNAL; e.ctx = ctx;
    {
        std::lock_guard<std::mutex> lk(g_queue_mutex);
        if (g_queue.size() >= g_queue_max) {
            g_queue_drop_count++;
            return -1;
        }
        g_queue.push(e);
    }
    g_queue_cv.notify_one();
    return 0;
}

int demo_publisher_get_stats(int* out_queue_len, int* out_queue_max, uint64_t* out_drop_count) {
    if (out_queue_len) {
        std::lock_guard<std::mutex> lk(g_queue_mutex);
        *out_queue_len = (int)g_queue.size();
    }
    if (out_queue_max) *out_queue_max = (int)g_queue_max;
    if (out_drop_count) *out_drop_count = g_queue_drop_count;
    return 0;
}

#endif
