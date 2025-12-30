/*
  +----------------------------------------------------------------------+
  | ext-tui: Braille/block canvas for high-resolution drawing           |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_CANVAS_H
#define TUI_CANVAS_H

#include <stdint.h>
#include "../node/node.h"

typedef enum {
    TUI_CANVAS_BRAILLE,     /* 2x4 resolution per cell (braille chars) */
    TUI_CANVAS_BLOCK,       /* 2x2 resolution per cell (block chars) */
    TUI_CANVAS_ASCII        /* 1x1 resolution (ASCII chars) */
} tui_canvas_mode;

typedef struct {
    uint8_t *pixels;        /* Bit array for set pixels */
    int pixel_width;        /* Width in pixels */
    int pixel_height;       /* Height in pixels */
    int char_width;         /* Width in terminal chars */
    int char_height;        /* Height in terminal chars */
    tui_canvas_mode mode;
    tui_color color;        /* Drawing color */
} tui_canvas;

/**
 * Create a canvas.
 *
 * @param width  Width in terminal characters
 * @param height Height in terminal characters
 * @param mode   Drawing mode (braille gives 2x4 resolution per char)
 */
tui_canvas* tui_canvas_create(int width, int height, tui_canvas_mode mode);

/**
 * Free a canvas.
 */
void tui_canvas_free(tui_canvas *canvas);

/**
 * Clear all pixels.
 */
void tui_canvas_clear_all(tui_canvas *canvas);

/**
 * Set a pixel.
 */
void tui_canvas_set(tui_canvas *canvas, int x, int y);

/**
 * Clear a pixel.
 */
void tui_canvas_unset(tui_canvas *canvas, int x, int y);

/**
 * Toggle a pixel.
 */
void tui_canvas_toggle(tui_canvas *canvas, int x, int y);

/**
 * Check if pixel is set.
 */
int tui_canvas_get(tui_canvas *canvas, int x, int y);

/**
 * Draw line on canvas (pixel coords).
 */
void tui_canvas_line(tui_canvas *canvas, int x1, int y1, int x2, int y2);

/**
 * Draw rectangle outline on canvas (pixel coords).
 */
void tui_canvas_rect(tui_canvas *canvas, int x, int y, int w, int h);

/**
 * Fill rectangle on canvas (pixel coords).
 */
void tui_canvas_fill_rect(tui_canvas *canvas, int x, int y, int w, int h);

/**
 * Draw circle on canvas (pixel coords).
 */
void tui_canvas_circle(tui_canvas *canvas, int cx, int cy, int radius);

/**
 * Fill circle on canvas (pixel coords).
 */
void tui_canvas_fill_circle(tui_canvas *canvas, int cx, int cy, int radius);

/**
 * Set canvas drawing color.
 */
void tui_canvas_set_color(tui_canvas *canvas, uint8_t r, uint8_t g, uint8_t b);

/**
 * Get pixel resolution of canvas.
 */
void tui_canvas_get_resolution(tui_canvas *canvas, int *width, int *height);

/**
 * Render canvas to array of UTF-8 strings.
 * Caller must free returned array and strings.
 *
 * @param canvas Canvas to render
 * @param line_count Output: number of lines
 * @return Array of UTF-8 strings (one per line)
 */
char** tui_canvas_render(tui_canvas *canvas, int *line_count);

/**
 * Free array returned by tui_canvas_render.
 */
void tui_canvas_render_free(char **lines, int line_count);

#endif /* TUI_CANVAS_H */
