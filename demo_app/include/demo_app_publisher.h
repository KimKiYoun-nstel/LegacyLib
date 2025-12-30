#ifndef DEMO_APP_PUBLISHER_H
#define DEMO_APP_PUBLISHER_H

#include "demo_app.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize publisher (creates queue + worker). Returns 0 on success.
int demo_publisher_init(void);

// Shutdown publisher and free resources.
void demo_publisher_shutdown(void);

// Enqueue a signal publish event (non-blocking). Returns 0 if enqueued, -1 if dropped.
int demo_publisher_enqueue_signal(DemoAppContext* ctx);

// Query publisher queue statistics. Returns 0 on success.
int demo_publisher_get_stats(int* out_queue_len, int* out_queue_max, uint64_t* out_drop_count);

#ifdef __cplusplus
}
#endif

#endif // DEMO_APP_PUBLISHER_H
