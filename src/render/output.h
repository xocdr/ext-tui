/*
  +----------------------------------------------------------------------+
  | ext-tui: Terminal output                                            |
  +----------------------------------------------------------------------+
  | Manages terminal output using double-buffered rendering for         |
  | flicker-free updates. Handles alternate screen buffer switching     |
  | and cursor visibility.                                              |
  |                                                                      |
  | Thread Safety: NOT thread-safe. All calls must be from the same     |
  | thread that created the output instance.                            |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_OUTPUT_H
#define TUI_OUTPUT_H

#include "buffer.h"

/* Output modes */
typedef enum {
    TUI_OUTPUT_NORMAL,      /* Normal screen */
    TUI_OUTPUT_ALTERNATE    /* Alternate screen buffer */
} tui_output_mode;

/* Renderer state (double-buffered) */
typedef struct {
    tui_buffer *front;      /* Current display (what user sees) */
    tui_buffer *back;       /* Next frame (being prepared) */
    tui_output_mode mode;   /* Current screen mode */
    int cursor_x;           /* Cursor X position */
    int cursor_y;           /* Cursor Y position */
    int cursor_visible;     /* Whether cursor is visible */
} tui_output;

/* ----------------------------------------------------------------
 * Lifecycle
 * ---------------------------------------------------------------- */

/**
 * Create a new output instance with double-buffered rendering.
 * @param width  Terminal width in columns
 * @param height Terminal height in rows
 * @return New output instance, or NULL on allocation failure
 */
tui_output* tui_output_create(int width, int height);

/**
 * Destroy output instance and free all resources.
 * @param out Output instance (NULL-safe)
 */
void tui_output_destroy(tui_output *out);

/* ----------------------------------------------------------------
 * Screen modes
 * ---------------------------------------------------------------- */

/**
 * Enter alternate screen buffer (preserves normal screen content).
 * Commonly used for full-screen TUI applications.
 * @param out Output instance
 */
void tui_output_enter_alternate(tui_output *out);

/**
 * Exit alternate screen buffer (restores normal screen content).
 * @param out Output instance
 */
void tui_output_exit_alternate(tui_output *out);

/* ----------------------------------------------------------------
 * Rendering
 * ---------------------------------------------------------------- */

/**
 * Render buffer to terminal with differential updates.
 * Only changed cells are written for efficiency.
 * @param out Output instance
 * @param buf Buffer to render
 */
void tui_output_render(tui_output *out, tui_buffer *buf);

/**
 * Render buffer with explicit cursor visibility control.
 * @param out Output instance
 * @param buf Buffer to render
 * @param show_cursor Whether to show cursor after render
 */
void tui_output_render_with_cursor(tui_output *out, tui_buffer *buf, int show_cursor);

/**
 * Flush any pending output to terminal.
 * @param out Output instance
 */
void tui_output_flush(tui_output *out);

/* ----------------------------------------------------------------
 * Cursor control
 * ---------------------------------------------------------------- */

/**
 * Show the terminal cursor.
 * @param out Output instance
 */
void tui_output_show_cursor(tui_output *out);

/**
 * Hide the terminal cursor.
 * @param out Output instance
 */
void tui_output_hide_cursor(tui_output *out);

/**
 * Move cursor to specified position.
 * @param out Output instance
 * @param x   Column (0-indexed)
 * @param y   Row (0-indexed)
 */
void tui_output_move_cursor(tui_output *out, int x, int y);

/* ----------------------------------------------------------------
 * Low-level I/O utilities
 * ---------------------------------------------------------------- */

/**
 * Write all bytes to file descriptor, handling partial writes and EINTR.
 * @param fd  File descriptor to write to (must be >= 0)
 * @param buf Data buffer
 * @param len Number of bytes to write
 * @return 0 on success, -1 on error (sets errno)
 */
int tui_write_all(int fd, const void *buf, size_t len);

/**
 * Check if terminal output is available and valid.
 * Use before operations that require a terminal.
 * @return 1 if STDOUT is a valid TTY, 0 otherwise
 */
int tui_output_is_valid(void);

#endif /* TUI_OUTPUT_H */
