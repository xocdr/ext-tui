/*
  +----------------------------------------------------------------------+
  | ext-tui: Event loop                                                 |
  +----------------------------------------------------------------------+
  | Poll-based event loop for keyboard input, terminal resize events,  |
  | and repeating timers. Provides the main application loop for TUI   |
  | applications.                                                       |
  |                                                                     |
  | Thread Safety: NOT thread-safe. All calls must be from the same    |
  | thread that created the loop instance.                             |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_LOOP_H
#define TUI_LOOP_H

/**
 * Callback for keyboard/mouse input.
 * @param input Raw input bytes (may contain escape sequences)
 * @param len   Number of bytes in input
 * @param userdata User-provided context pointer
 */
typedef void (*tui_input_callback)(const char *input, int len, void *userdata);

/**
 * Callback for timer expiration.
 * @param userdata User-provided context pointer
 */
typedef void (*tui_timer_callback)(void *userdata);

/**
 * Callback for terminal resize events.
 * @param width  New terminal width in columns
 * @param height New terminal height in rows
 * @param userdata User-provided context pointer
 */
typedef void (*tui_resize_callback)(int width, int height, void *userdata);

/**
 * Callback for each event loop iteration.
 * @param userdata User-provided context pointer
 */
typedef void (*tui_tick_callback)(void *userdata);

typedef struct tui_loop tui_loop;

/* ================================================================
 * Lifecycle
 * ================================================================ */

/**
 * Create a new event loop.
 * Installs SIGWINCH handler for terminal resize detection.
 * @return New loop instance, or NULL on failure
 */
tui_loop* tui_loop_create(void);

/**
 * Destroy event loop and free resources.
 * Restores original SIGWINCH handler.
 * @param loop Loop instance (NULL-safe)
 */
void tui_loop_destroy(tui_loop *loop);

/* ================================================================
 * Loop control
 * ================================================================ */

/**
 * Run one iteration of the event loop.
 * Polls for input, checks timers, and calls registered callbacks.
 * @param loop Loop instance
 * @return 0 to continue, non-zero if stop was requested
 */
int tui_loop_run(tui_loop *loop);

/**
 * Request loop to stop.
 * @param loop Loop instance
 */
void tui_loop_stop(tui_loop *loop);

/* ================================================================
 * Callback registration
 * ================================================================ */

/**
 * Set input event callback.
 * @param loop     Loop instance
 * @param cb       Callback function (NULL to disable)
 * @param userdata Context passed to callback
 */
void tui_loop_on_input(tui_loop *loop, tui_input_callback cb, void *userdata);

/**
 * Set terminal resize callback.
 * @param loop     Loop instance
 * @param cb       Callback function (NULL to disable)
 * @param userdata Context passed to callback
 */
void tui_loop_on_resize(tui_loop *loop, tui_resize_callback cb, void *userdata);

/**
 * Set per-iteration tick callback.
 * @param loop     Loop instance
 * @param cb       Callback function (NULL to disable)
 * @param userdata Context passed to callback
 */
void tui_loop_on_tick(tui_loop *loop, tui_tick_callback cb, void *userdata);

/* ================================================================
 * Timers
 * ================================================================ */

/**
 * Add a repeating timer.
 * @param loop        Loop instance
 * @param interval_ms Interval in milliseconds
 * @param cb          Timer callback
 * @param userdata    Context passed to callback
 * @return Timer ID (>= 1), or -1 on failure
 */
int tui_loop_add_timer(tui_loop *loop, int interval_ms, tui_timer_callback cb, void *userdata);

/**
 * Remove a timer by ID.
 * @param loop     Loop instance
 * @param timer_id Timer ID from tui_loop_add_timer
 */
void tui_loop_remove_timer(tui_loop *loop, int timer_id);

/**
 * Manually advance timers by specified milliseconds.
 * Used for headless testing to simulate time passage.
 * @param loop Loop instance
 * @param ms   Milliseconds to advance
 */
void tui_loop_tick_timers(tui_loop *loop, int ms);

/**
 * Check if input is available from a valid TTY.
 * Use before starting the event loop to verify stdin is usable.
 * @return 1 if STDIN is a valid TTY, 0 otherwise
 */
int tui_loop_is_stdin_valid(void);

#endif /* TUI_LOOP_H */
