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
#include <stdatomic.h>

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

/* Use C11 atomic for async-signal-safe flag */
static atomic_flag resize_pending = ATOMIC_FLAG_INIT;

static void sigwinch_handler(int sig)
{
    (void)sig;
    atomic_flag_test_and_set(&resize_pending);
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

int tui_loop_add_timer(tui_loop *loop, int interval_ms, tui_timer_callback cb, void *userdata)
{
    if (!loop) return -1;
    if (loop->timer_count >= MAX_TIMERS) {
        return -1;
    }

    int id = loop->next_timer_id++;
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
    int timeout = 100; /* Default 100ms */
    for (int i = 0; i < loop->timer_count; i++) {
        int remaining = loop->timers[i].interval_ms - loop->timers[i].elapsed_ms;
        if (remaining < timeout && remaining > 0) {
            timeout = remaining;
        }
    }
    if (timeout <= 0) timeout = 1; /* Minimum 1ms */

    int ret = poll(fds, 1, timeout);

    /* Check for resize using atomic flag to avoid race condition */
    if (loop->resize_cb) {
        /* Test and clear atomically - returns true if flag was set */
        if (atomic_flag_test_and_set(&resize_pending)) {
            atomic_flag_clear(&resize_pending);
            /* Get actual terminal size */
            int width, height;
            if (tui_terminal_get_size(&width, &height) == 0) {
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
