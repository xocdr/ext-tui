/*
  +----------------------------------------------------------------------+
  | ext-tui: Drawing primitives                                         |
  +----------------------------------------------------------------------+
*/

#include "primitives.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Border characters */
static const char* border_chars_single[] = {
    "┌", "─", "┐",
    "│", " ", "│",
    "└", "─", "┘"
};

static const char* border_chars_double[] = {
    "╔", "═", "╗",
    "║", " ", "║",
    "╚", "═", "╝"
};

static const char* border_chars_round[] = {
    "╭", "─", "╮",
    "│", " ", "│",
    "╰", "─", "╯"
};

static const char* border_chars_bold[] = {
    "┏", "━", "┓",
    "┃", " ", "┃",
    "┗", "━", "┛"
};

static const char* border_chars_dashed[] = {
    "┌", "┄", "┐",
    "┆", " ", "┆",
    "└", "┄", "┘"
};

/* Get unicode codepoint from border character */
static uint32_t border_char_to_codepoint(const char *ch)
{
    /* UTF-8 decode for box drawing characters (3 bytes) */
    unsigned char *p = (unsigned char *)ch;
    if ((p[0] & 0xF0) == 0xE0) {
        return ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
    }
    return (uint32_t)ch[0];
}

void tui_draw_point(tui_buffer *buf, int x, int y, uint32_t ch, const tui_style *style)
{
    tui_buffer_set_cell(buf, x, y, ch, style);
}

void tui_draw_line(tui_buffer *buf, int x1, int y1, int x2, int y2,
                   uint32_t ch, const tui_style *style)
{
    /* Bresenham's line algorithm */
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    while (1) {
        tui_buffer_set_cell(buf, x1, y1, ch, style);

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void tui_draw_rect(tui_buffer *buf, int x, int y, int width, int height,
                   tui_border_style border, const tui_style *style)
{
    if (width < 2 || height < 2) return;

    const char **chars;
    switch (border) {
        case TUI_BORDER_SINGLE: chars = border_chars_single; break;
        case TUI_BORDER_DOUBLE: chars = border_chars_double; break;
        case TUI_BORDER_ROUND: chars = border_chars_round; break;
        case TUI_BORDER_BOLD: chars = border_chars_bold; break;
        case TUI_BORDER_DASHED: chars = border_chars_dashed; break;
        default: chars = border_chars_single; break;
    }

    /* Corners */
    tui_buffer_set_cell(buf, x, y, border_char_to_codepoint(chars[0]), style);
    tui_buffer_set_cell(buf, x + width - 1, y, border_char_to_codepoint(chars[2]), style);
    tui_buffer_set_cell(buf, x, y + height - 1, border_char_to_codepoint(chars[6]), style);
    tui_buffer_set_cell(buf, x + width - 1, y + height - 1, border_char_to_codepoint(chars[8]), style);

    /* Horizontal edges */
    uint32_t h_char = border_char_to_codepoint(chars[1]);
    for (int i = 1; i < width - 1; i++) {
        tui_buffer_set_cell(buf, x + i, y, h_char, style);
        tui_buffer_set_cell(buf, x + i, y + height - 1, h_char, style);
    }

    /* Vertical edges */
    uint32_t v_char = border_char_to_codepoint(chars[3]);
    for (int i = 1; i < height - 1; i++) {
        tui_buffer_set_cell(buf, x, y + i, v_char, style);
        tui_buffer_set_cell(buf, x + width - 1, y + i, v_char, style);
    }
}

void tui_fill_rect(tui_buffer *buf, int x, int y, int width, int height,
                   uint32_t fill_char, const tui_style *style)
{
    tui_buffer_fill_rect(buf, x, y, width, height, fill_char, style);
}

void tui_draw_circle(tui_buffer *buf, int cx, int cy, int radius,
                     uint32_t ch, const tui_style *style)
{
    /* Midpoint circle algorithm */
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        tui_buffer_set_cell(buf, cx + x, cy + y, ch, style);
        tui_buffer_set_cell(buf, cx + y, cy + x, ch, style);
        tui_buffer_set_cell(buf, cx - y, cy + x, ch, style);
        tui_buffer_set_cell(buf, cx - x, cy + y, ch, style);
        tui_buffer_set_cell(buf, cx - x, cy - y, ch, style);
        tui_buffer_set_cell(buf, cx - y, cy - x, ch, style);
        tui_buffer_set_cell(buf, cx + y, cy - x, ch, style);
        tui_buffer_set_cell(buf, cx + x, cy - y, ch, style);

        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        }
        if (err > 0) {
            x--;
            err -= 2 * x + 1;
        }
    }
}

void tui_fill_circle(tui_buffer *buf, int cx, int cy, int radius,
                     uint32_t fill_char, const tui_style *style)
{
    /* Fill circle using horizontal lines */
    for (int y = -radius; y <= radius; y++) {
        int dx = (int)(sqrt((double)(radius * radius - y * y)) + 0.5);
        for (int x = -dx; x <= dx; x++) {
            tui_buffer_set_cell(buf, cx + x, cy + y, fill_char, style);
        }
    }
}

void tui_draw_ellipse(tui_buffer *buf, int cx, int cy, int rx, int ry,
                      uint32_t ch, const tui_style *style)
{
    /* Midpoint ellipse algorithm */
    int x = 0;
    int y = ry;
    long rx2 = (long)rx * rx;
    long ry2 = (long)ry * ry;
    long two_rx2 = 2 * rx2;
    long two_ry2 = 2 * ry2;
    long px = 0;
    long py = two_rx2 * y;

    /* Region 1 */
    long p = ry2 - rx2 * ry + rx2 / 4;
    while (px < py) {
        tui_buffer_set_cell(buf, cx + x, cy + y, ch, style);
        tui_buffer_set_cell(buf, cx - x, cy + y, ch, style);
        tui_buffer_set_cell(buf, cx + x, cy - y, ch, style);
        tui_buffer_set_cell(buf, cx - x, cy - y, ch, style);

        x++;
        px += two_ry2;
        if (p < 0) {
            p += ry2 + px;
        } else {
            y--;
            py -= two_rx2;
            p += ry2 + px - py;
        }
    }

    /* Region 2 */
    p = ry2 * (x * x + x) + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
    while (y >= 0) {
        tui_buffer_set_cell(buf, cx + x, cy + y, ch, style);
        tui_buffer_set_cell(buf, cx - x, cy + y, ch, style);
        tui_buffer_set_cell(buf, cx + x, cy - y, ch, style);
        tui_buffer_set_cell(buf, cx - x, cy - y, ch, style);

        y--;
        py -= two_rx2;
        if (p > 0) {
            p += rx2 - py;
        } else {
            x++;
            px += two_ry2;
            p += rx2 - py + px;
        }
    }
}

void tui_fill_ellipse(tui_buffer *buf, int cx, int cy, int rx, int ry,
                      uint32_t fill_char, const tui_style *style)
{
    /* Validate radii to avoid division by zero */
    if (rx <= 0 || ry <= 0) return;

    /* Fill ellipse using horizontal lines */
    for (int y = -ry; y <= ry; y++) {
        double normalized_y = (double)y / ry;
        int dx = (int)(rx * sqrt(1.0 - normalized_y * normalized_y) + 0.5);
        for (int x = -dx; x <= dx; x++) {
            tui_buffer_set_cell(buf, cx + x, cy + y, fill_char, style);
        }
    }
}

void tui_draw_triangle(tui_buffer *buf, int x1, int y1, int x2, int y2,
                       int x3, int y3, uint32_t ch, const tui_style *style)
{
    tui_draw_line(buf, x1, y1, x2, y2, ch, style);
    tui_draw_line(buf, x2, y2, x3, y3, ch, style);
    tui_draw_line(buf, x3, y3, x1, y1, ch, style);
}

/* Helper for triangle fill */
static void swap_int(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

void tui_fill_triangle(tui_buffer *buf, int x1, int y1, int x2, int y2,
                       int x3, int y3, uint32_t fill_char, const tui_style *style)
{
    /* Sort vertices by y coordinate */
    if (y1 > y2) { swap_int(&x1, &x2); swap_int(&y1, &y2); }
    if (y1 > y3) { swap_int(&x1, &x3); swap_int(&y1, &y3); }
    if (y2 > y3) { swap_int(&x2, &x3); swap_int(&y2, &y3); }

    /* Degenerate triangle (all points on same row) */
    if (y1 == y3) {
        int left = x1 < x2 ? (x1 < x3 ? x1 : x3) : (x2 < x3 ? x2 : x3);
        int right = x1 > x2 ? (x1 > x3 ? x1 : x3) : (x2 > x3 ? x2 : x3);
        for (int x = left; x <= right; x++) {
            tui_buffer_set_cell(buf, x, y1, fill_char, style);
        }
        return;
    }

    /* Flat bottom triangle case */
    if (y2 == y3) {
        int denom = y2 - y1 + 1;
        if (denom <= 0) return;  /* Prevent division by zero */
        float dx1 = (float)(x2 - x1) / denom;
        float dx2 = (float)(x3 - x1) / denom;
        float sx = (float)x1, ex = (float)x1;

        for (int y = y1; y <= y2; y++) {
            int left = (int)(sx < ex ? sx : ex);
            int right = (int)(sx < ex ? ex : sx);
            for (int x = left; x <= right; x++) {
                tui_buffer_set_cell(buf, x, y, fill_char, style);
            }
            sx += dx1;
            ex += dx2;
        }
        return;
    }

    /* Flat top triangle case */
    if (y1 == y2) {
        int denom1 = y3 - y1 + 1;
        int denom2 = y3 - y2 + 1;
        if (denom1 <= 0 || denom2 <= 0) return;  /* Prevent division by zero */
        float dx1 = (float)(x3 - x1) / denom1;
        float dx2 = (float)(x3 - x2) / denom2;
        float sx = (float)x1, ex = (float)x2;

        for (int y = y1; y <= y3; y++) {
            int left = (int)(sx < ex ? sx : ex);
            int right = (int)(sx < ex ? ex : sx);
            for (int x = left; x <= right; x++) {
                tui_buffer_set_cell(buf, x, y, fill_char, style);
            }
            sx += dx1;
            ex += dx2;
        }
        return;
    }

    /* General case: split into flat bottom and flat top triangles */
    int denom_split = y3 - y1;
    if (denom_split == 0) return;  /* Prevent division by zero */
    int x4 = x1 + (int)((float)(y2 - y1) / denom_split * (x3 - x1));
    int y4 = y2;

    /* Draw flat bottom triangle (top half) */
    {
        int denom1 = y2 - y1 + 1;
        int denom2 = y4 - y1 + 1;
        if (denom1 > 0 && denom2 > 0) {
            float dx1 = (float)(x2 - x1) / denom1;
            float dx2 = (float)(x4 - x1) / denom2;
            float sx = (float)x1, ex = (float)x1;

            for (int y = y1; y <= y2; y++) {
                int left = (int)(sx < ex ? sx : ex);
                int right = (int)(sx < ex ? ex : sx);
                for (int x = left; x <= right; x++) {
                    tui_buffer_set_cell(buf, x, y, fill_char, style);
                }
                sx += dx1;
                ex += dx2;
            }
        }
    }

    /* Draw flat top triangle (bottom half) */
    {
        int denom1 = y3 - y2 + 1;
        int denom2 = y3 - y4 + 1;
        if (denom1 > 0 && denom2 > 0) {
            float dx1 = (float)(x3 - x2) / denom1;
            float dx2 = (float)(x3 - x4) / denom2;
            float sx = (float)x2, ex = (float)x4;

            for (int y = y2; y <= y3; y++) {
                int left = (int)(sx < ex ? sx : ex);
                int right = (int)(sx < ex ? ex : sx);
                for (int x = left; x <= right; x++) {
                    tui_buffer_set_cell(buf, x, y, fill_char, style);
                }
                sx += dx1;
                ex += dx2;
            }
        }
    }
}
