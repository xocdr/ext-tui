/*
  +----------------------------------------------------------------------+
  | ext-tui: Event loop (poll-based)                                    |
  +----------------------------------------------------------------------+
*/

#include "loop.h"
#include "../terminal/terminal.h"
#include "../render/buffer.h"
#include "php.h"
#include "php_tui.h"
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>

/* Constants for configuration */
#define DEFAULT_POLL_TIMEOUT_MS 100
#define MIN_POLL_TIMEOUT_MS 1

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
    struct sigaction old_sigwinch;  /* Saved SIGWINCH handler for restoration */
    int sigwinch_installed;          /* Whether we installed a handler */
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

    /* Set up SIGWINCH handler, saving old handler for restoration */
    struct sigaction sa;
    sa.sa_handler = sigwinch_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGWINCH, &sa, &loop->old_sigwinch) == 0) {
        loop->sigwinch_installed = 1;
    }

    return loop;
}

void tui_loop_destroy(tui_loop *loop)
{
    if (loop) {
        /* Restore original SIGWINCH handler if we installed one */
        if (loop->sigwinch_installed) {
            sigaction(SIGWINCH, &loop->old_sigwinch, NULL);
        }
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

    /* Track loop iteration */
    TUI_METRIC_INC(loop_iterations);

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

    /* Handle poll errors */
    if (ret < 0) {
        if (errno == EINTR) {
            /* Interrupted by signal - this is normal, continue processing
             * (resize signal handling will happen below) */
        } else {
            /* Real error - log with php_error_docref and continue.
             * We don't abort the event loop on poll errors to maintain
             * best-effort TUI operation. Common causes:
             * - EBADF: fd became invalid (unlikely for stdin)
             * - ENOMEM: out of memory for poll internals
             * - EINVAL: timeout overflow (handled by MIN_POLL_TIMEOUT_MS) */
            TUI_METRIC_INC(poll_errors);
            php_error_docref(NULL, E_WARNING,
                "poll() failed with errno %d: %s",
                errno, strerror(errno));
        }
    }

    /* Check for resize using sig_atomic_t flag.
     * We atomically test-and-clear to avoid race conditions:
     * 1. Read the flag value
     * 2. If set, clear it immediately before processing
     * This ensures we don't lose signals that arrive during processing,
     * since we always fetch the current terminal size anyway. */
    if (loop->resize_cb) {
        sig_atomic_t pending = resize_pending;
        if (pending) {
            resize_pending = 0;  /* Clear before processing to catch new signals */
            /* Get actual terminal size (always current, regardless of signal count) */
            int width, height;
            if (tui_terminal_get_size(&width, &height) == 0) {
                /* Validate terminal size against buffer limits to prevent
                 * excessive memory allocation. Use configurable buffer limits. */
                int max_width = tui_buffer_get_max_width();
                int max_height = tui_buffer_get_max_height();
                if (width > 0 && width <= max_width &&
                    height > 0 && height <= max_height) {
                    TUI_METRIC_INC(resize_events);
                    loop->resize_cb(width, height, loop->resize_userdata);
                }
            }
        }
    }

    if (ret > 0 && (fds[0].revents & POLLIN)) {
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        if (n > 0 && loop->input_cb) {
            buf[n] = '\0';
            TUI_METRIC_INC(input_events);
            loop->input_cb(buf, (int)n, loop->input_userdata);
        }
    }

    /* Update timers based on actual elapsed time */
    for (int i = 0; i < loop->timer_count; i++) {
        loop->timers[i].elapsed_ms += timeout;
        if (loop->timers[i].elapsed_ms >= loop->timers[i].interval_ms) {
            loop->timers[i].elapsed_ms = 0;
            if (loop->timers[i].callback) {
                TUI_METRIC_INC(timer_fires);
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

void tui_loop_tick_timers(tui_loop *loop, int ms)
{
    if (!loop || ms <= 0) return;

    /* Advance all timers by the specified milliseconds */
    for (int i = 0; i < loop->timer_count; i++) {
        loop->timers[i].elapsed_ms += ms;
        /* Fire timer if it's due */
        while (loop->timers[i].elapsed_ms >= loop->timers[i].interval_ms) {
            loop->timers[i].elapsed_ms -= loop->timers[i].interval_ms;
            if (loop->timers[i].callback) {
                TUI_METRIC_INC(timer_fires);
                loop->timers[i].callback(loop->timers[i].userdata);
            }
        }
    }
}

int tui_loop_is_stdin_valid(void)
{
    return isatty(STDIN_FILENO) ? 1 : 0;
}
