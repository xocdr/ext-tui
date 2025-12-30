/*
  +----------------------------------------------------------------------+
  | ext-tui: Drawing primitives                                         |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_PRIMITIVES_H
#define TUI_PRIMITIVES_H

#include "../render/buffer.h"
#include "../node/node.h"

/**
 * Draw a single point on the buffer.
 */
void tui_draw_point(tui_buffer *buf, int x, int y, uint32_t ch, const tui_style *style);

/**
 * Draw a line using Bresenham's algorithm.
 */
void tui_draw_line(tui_buffer *buf, int x1, int y1, int x2, int y2,
                   uint32_t ch, const tui_style *style);

/**
 * Draw a rectangle outline.
 */
void tui_draw_rect(tui_buffer *buf, int x, int y, int width, int height,
                   tui_border_style border, const tui_style *style);

/**
 * Draw a filled rectangle.
 */
void tui_fill_rect(tui_buffer *buf, int x, int y, int width, int height,
                   uint32_t fill_char, const tui_style *style);

/**
 * Draw a circle using midpoint circle algorithm.
 */
void tui_draw_circle(tui_buffer *buf, int cx, int cy, int radius,
                     uint32_t ch, const tui_style *style);

/**
 * Draw a filled circle.
 */
void tui_fill_circle(tui_buffer *buf, int cx, int cy, int radius,
                     uint32_t fill_char, const tui_style *style);

/**
 * Draw an ellipse.
 */
void tui_draw_ellipse(tui_buffer *buf, int cx, int cy, int rx, int ry,
                      uint32_t ch, const tui_style *style);

/**
 * Draw a filled ellipse.
 */
void tui_fill_ellipse(tui_buffer *buf, int cx, int cy, int rx, int ry,
                      uint32_t fill_char, const tui_style *style);

/**
 * Draw a triangle outline.
 */
void tui_draw_triangle(tui_buffer *buf, int x1, int y1, int x2, int y2,
                       int x3, int y3, uint32_t ch, const tui_style *style);

/**
 * Fill a triangle using scanline algorithm.
 */
void tui_fill_triangle(tui_buffer *buf, int x1, int y1, int x2, int y2,
                       int x3, int y3, uint32_t fill_char, const tui_style *style);

#endif /* TUI_PRIMITIVES_H */
