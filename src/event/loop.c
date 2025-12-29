/*
  +----------------------------------------------------------------------+
  | ext-tui: Event loop (poll-based)                                    |
  +----------------------------------------------------------------------+
*/

#include "loop.h"
#include "../terminal/terminal.h"
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>

/* Constants for configuration */
#define DEFAULT_POLL_TIMEOUT_MS 100
#define MIN_POLL_TIMEOUT_MS 1

/* Maximum terminal dimension in characters. This caps the terminal size
   reported to applications, preventing excessive memory allocation in
   render buffers (width * height * sizeof(cell) bytes). 1000x1000 allows
   for very large terminals while keeping buffer size manageable (~8MB max).
   Standard terminals are typically 80-200 columns and 24-100 rows. */
#define MAX_TERMINAL_DIMENSION 1000

#define MAX_TIMERS 32

typedef struct {
    int id;
    int interval_ms;
    int elapsed_ms;
    tui_timer_callback callback;
    void *userdata;
} tui_timer;

struct tui_loop {
    int running;
    tui_input_callback input_cb;
    void *input_userdata;
    tui_resize_callback resize_cb;
    void *resize_userdata;
    tui_tick_callback tick_cb;
    void *tick_userdata;
    tui_timer timers[MAX_TIMERS];
    int timer_count;
    int next_timer_id;
};

/* Use C11 atomic for async-signal-safe resize detection
 * We use atomic_int instead of atomic_flag for clearer semantics:
 * - Signal handler sets it to 1 (async-signal-safe)
 * - Main loop reads and clears atomically using exchange */
static volatile sig_atomic_t resize_pending = 0;

static void sigwinch_handler(int sig)
{
    (void)sig;
    resize_pending = 1;  /* sig_atomic_t write is async-signal-safe */
}

tui_loop* tui_loop_create(void)
{
    tui_loop *loop = calloc(1, sizeof(tui_loop));
    if (!loop) return NULL;

    loop->next_timer_id = 1;

    /* Set up SIGWINCH handler */
    struct sigaction sa;
    sa.sa_handler = sigwinch_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGWINCH, &sa, NULL);

    return loop;
}

void tui_loop_destroy(tui_loop *loop)
{
    if (loop) {
        free(loop);
    }
}

void tui_loop_on_input(tui_loop *loop, tui_input_callback cb, void *userdata)
{
    if (!loop) return;
    loop->input_cb = cb;
    loop->input_userdata = userdata;
}

void tui_loop_on_resize(tui_loop *loop, tui_resize_callback cb, void *userdata)
{
    if (!loop) return;
    loop->resize_cb = cb;
    loop->resize_userdata = userdata;
}

void tui_loop_on_tick(tui_loop *loop, tui_tick_callback cb, void *userdata)
{
    if (!loop) return;
    loop->tick_cb = cb;
    loop->tick_userdata = userdata;
}

/* Check if a timer ID is already in use */
static int timer_id_in_use(tui_loop *loop, int id)
{
    for (int i = 0; i < loop->timer_count; i++) {
        if (loop->timers[i].id == id) return 1;
    }
    return 0;
}

int tui_loop_add_timer(tui_loop *loop, int interval_ms, tui_timer_callback cb, void *userdata)
{
    if (!loop) return -1;
    if (loop->timer_count >= MAX_TIMERS) {
        return -1;
    }

    /* Handle timer ID overflow by wrapping to 1 (0 is invalid) */
    if (loop->next_timer_id >= INT_MAX) {
        loop->next_timer_id = 1;
    }

    /* Skip IDs already in use (can happen after wrap) */
    int id = loop->next_timer_id++;
    int attempts = 0;
    while (timer_id_in_use(loop, id) && attempts < MAX_TIMERS) {
        if (loop->next_timer_id >= INT_MAX) {
            loop->next_timer_id = 1;
        }
        id = loop->next_timer_id++;
        attempts++;
    }
    if (attempts >= MAX_TIMERS) {
        /* All IDs in range are in use (should never happen with MAX_TIMERS < INT_MAX) */
        return -1;
    }

    tui_timer *t = &loop->timers[loop->timer_count++];
    t->id = id;
    t->interval_ms = interval_ms;
    t->elapsed_ms = 0;
    t->callback = cb;
    t->userdata = userdata;

    return id;
}

void tui_loop_remove_timer(tui_loop *loop, int timer_id)
{
    if (!loop) return;
    for (int i = 0; i < loop->timer_count; i++) {
        if (loop->timers[i].id == timer_id) {
            /* Shift remaining timers */
            for (int j = i; j < loop->timer_count - 1; j++) {
                loop->timers[j] = loop->timers[j + 1];
            }
            loop->timer_count--;
            break;
        }
    }
}

int tui_loop_run(tui_loop *loop)
{
    if (!loop) return -1;

    struct pollfd fds[1];
    char buf[64];

    loop->running = 1;

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    /* Single iteration - poll once and process events */
    /* Calculate shortest timer interval */
    int timeout = DEFAULT_POLL_TIMEOUT_MS;
    for (int i = 0; i < loop->timer_count; i++) {
        int remaining = loop->timers[i].interval_ms - loop->timers[i].elapsed_ms;
        if (remaining < timeout && remaining > 0) {
            timeout = remaining;
        }
    }
    if (timeout <= 0) timeout = MIN_POLL_TIMEOUT_MS;

    int ret = poll(fds, 1, timeout);

    /* Check for resize using sig_atomic_t flag
     * Read and clear in one check - sig_atomic_t ensures atomic read/write.
     * We always get current terminal size when handling, so missing
     * intermediate signals is fine. */
    if (loop->resize_cb && resize_pending) {
        resize_pending = 0;  /* Clear before processing */
        /* Get actual terminal size (always current, regardless of signal count) */
        int width, height;
        if (tui_terminal_get_size(&width, &height) == 0) {
            /* Validate terminal size before using */
            if (width > 0 && width <= MAX_TERMINAL_DIMENSION &&
                height > 0 && height <= MAX_TERMINAL_DIMENSION) {
                loop->resize_cb(width, height, loop->resize_userdata);
            }
        }
    }

    if (ret > 0 && (fds[0].revents & POLLIN)) {
        int n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        if (n > 0 && loop->input_cb) {
            buf[n] = '\0';
            loop->input_cb(buf, n, loop->input_userdata);
        }
    }

    /* Update timers based on actual elapsed time */
    for (int i = 0; i < loop->timer_count; i++) {
        loop->timers[i].elapsed_ms += timeout;
        if (loop->timers[i].elapsed_ms >= loop->timers[i].interval_ms) {
            loop->timers[i].elapsed_ms = 0;
            if (loop->timers[i].callback) {
                loop->timers[i].callback(loop->timers[i].userdata);
            }
        }
    }

    /* Call tick callback (once per loop iteration) */
    if (loop->tick_cb) {
        loop->tick_cb(loop->tick_userdata);
    }

    return 0;
}

void tui_loop_stop(tui_loop *loop)
{
    if (!loop) return;
    loop->running = 0;
}
