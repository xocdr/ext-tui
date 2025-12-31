/*
  +----------------------------------------------------------------------+
  | ext-tui: Character buffer                                           |
  +----------------------------------------------------------------------+
  | Provides a 2D grid of styled cells for terminal rendering.          |
  | Each cell contains a Unicode codepoint and styling information.     |
  | Supports differential rendering via dirty flags.                    |
  |                                                                      |
  | Thread Safety: NOT thread-safe. All calls must be from the same     |
  | thread that created the buffer instance.                            |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_BUFFER_H
#define TUI_BUFFER_H

#include <stdint.h>
#include "../node/node.h"

/**
 * A single terminal cell containing character and style.
 */
typedef struct {
    uint32_t codepoint;  /* Unicode codepoint (0 = empty cell) */
    tui_style style;     /* Foreground, background, attributes */
    uint8_t dirty;       /* 1 if cell needs redraw, 0 otherwise */
} tui_cell;

/**
 * 2D grid of terminal cells.
 */
typedef struct {
    tui_cell *cells;     /* Contiguous array of width*height cells */
    int width;           /* Buffer width in columns */
    int height;          /* Buffer height in rows */
} tui_buffer;

/* ----------------------------------------------------------------
 * Lifecycle
 * ---------------------------------------------------------------- */

/**
 * Create a new buffer with specified dimensions.
 * @param width  Width in columns (1 to max_width)
 * @param height Height in rows (1 to max_height)
 * @return New buffer instance, or NULL on allocation failure
 */
tui_buffer* tui_buffer_create(int width, int height);

/**
 * Destroy buffer and free all resources.
 * @param buf Buffer instance (NULL-safe)
 */
void tui_buffer_destroy(tui_buffer *buf);

/**
 * Resize buffer to new dimensions.
 * Existing content is preserved where possible.
 * @param buf    Buffer to resize
 * @param width  New width (1 to max_width)
 * @param height New height (1 to max_height)
 * @return 0 on success, -1 on failure (buffer unchanged on failure)
 */
int tui_buffer_resize(tui_buffer *buf, int width, int height);

/* ----------------------------------------------------------------
 * Drawing operations
 * ---------------------------------------------------------------- */

/**
 * Clear buffer (all cells set to space with default style).
 * @param buf Buffer to clear
 */
void tui_buffer_clear(tui_buffer *buf);

/**
 * Set a single cell's character and style.
 * @param buf   Buffer
 * @param x     Column (0-indexed, clipped to bounds)
 * @param y     Row (0-indexed, clipped to bounds)
 * @param ch    Unicode codepoint
 * @param style Cell style (NULL uses default)
 */
void tui_buffer_set_cell(tui_buffer *buf, int x, int y, uint32_t ch, const tui_style *style);

/**
 * Write text starting at position with style.
 * @param buf   Buffer
 * @param x     Starting column (0-indexed)
 * @param y     Row (0-indexed)
 * @param text  UTF-8 text (NULL-safe)
 * @param style Text style (NULL uses default)
 */
void tui_buffer_write_text(tui_buffer *buf, int x, int y, const char *text, const tui_style *style);

/**
 * Fill rectangle with character and style.
 * @param buf   Buffer
 * @param x     Left column (0-indexed)
 * @param y     Top row (0-indexed)
 * @param w     Width in columns
 * @param h     Height in rows
 * @param ch    Fill character (Unicode codepoint)
 * @param style Fill style (NULL uses default)
 */
void tui_buffer_fill_rect(tui_buffer *buf, int x, int y, int w, int h, uint32_t ch, const tui_style *style);

/* ----------------------------------------------------------------
 * Cell access
 * ---------------------------------------------------------------- */

/**
 * Get pointer to cell at position.
 * @param buf Buffer
 * @param x   Column (0-indexed)
 * @param y   Row (0-indexed)
 * @return Cell pointer, or NULL if out of bounds
 */
tui_cell* tui_buffer_get_cell(tui_buffer *buf, int x, int y);

/* ----------------------------------------------------------------
 * Dirty tracking for differential rendering
 * ---------------------------------------------------------------- */

/**
 * Mark all cells as dirty (needing redraw).
 * @param buf Buffer
 */
void tui_buffer_mark_all_dirty(tui_buffer *buf);

/**
 * Mark all cells as clean (no redraw needed).
 * @param buf Buffer
 */
void tui_buffer_mark_clean(tui_buffer *buf);

/* ----------------------------------------------------------------
 * Serialization
 * ---------------------------------------------------------------- */

/**
 * Render buffer to string with ANSI escape codes.
 * @param buf Buffer
 * @return Newly allocated string (caller must free), or NULL on failure
 */
char* tui_buffer_to_string(tui_buffer *buf);

/* ----------------------------------------------------------------
 * Configuration (set from INI values by tui.c)
 * ---------------------------------------------------------------- */

/**
 * Set maximum allowed buffer dimensions.
 * @param max_width  Maximum width (must be >= 1)
 * @param max_height Maximum height (must be >= 1)
 */
void tui_buffer_set_max_dimensions(int max_width, int max_height);

/**
 * Get configured maximum buffer width.
 * @return Maximum width in columns
 */
int tui_buffer_get_max_width(void);

/**
 * Get configured maximum buffer height.
 * @return Maximum height in rows
 */
int tui_buffer_get_max_height(void);

#endif /* TUI_BUFFER_H */
