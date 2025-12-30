/*
  +----------------------------------------------------------------------+
  | ext-tui: Braille/block canvas for high-resolution drawing           |
  +----------------------------------------------------------------------+
*/

#include "canvas.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>

/*
 * Braille Unicode block: U+2800 to U+28FF
 * Each character represents a 2x4 dot matrix:
 *   ⠁ ⠈    (0,0) (1,0)
 *   ⠂ ⠐    (0,1) (1,1)
 *   ⠄ ⠠    (0,2) (1,2)
 *   ⡀ ⢀    (0,3) (1,3)
 */
static const uint8_t braille_dots[2][4] = {
    {0x01, 0x02, 0x04, 0x40},  /* Left column  (bits 0,1,2,6) */
    {0x08, 0x10, 0x20, 0x80}   /* Right column (bits 3,4,5,7) */
};

/*
 * Block characters for 2x2 resolution
 * Each character represents a 2x2 block:
 *   ▘ ▝    (0,0) (1,0)
 *   ▖ ▗    (0,1) (1,1)
 */
static const char* block_chars[16] = {
    " ",    /* 0000 */
    "▗",    /* 0001 - bottom-right */
    "▖",    /* 0010 - bottom-left */
    "▄",    /* 0011 - bottom half */
    "▝",    /* 0100 - top-right */
    "▐",    /* 0101 - right half */
    "▞",    /* 0110 - diagonal \ */
    "▟",    /* 0111 - all but top-left */
    "▘",    /* 1000 - top-left */
    "▚",    /* 1001 - diagonal / */
    "▌",    /* 1010 - left half */
    "▙",    /* 1011 - all but top-right */
    "▀",    /* 1100 - top half */
    "▜",    /* 1101 - all but bottom-left */
    "▛",    /* 1110 - all but bottom-right */
    "█"     /* 1111 - full block */
};

/* Get pixel dimensions per character cell */
static void get_cell_dimensions(tui_canvas_mode mode, int *width, int *height)
{
    switch (mode) {
        case TUI_CANVAS_BRAILLE:
            *width = 2;
            *height = 4;
            break;
        case TUI_CANVAS_BLOCK:
            *width = 2;
            *height = 2;
            break;
        case TUI_CANVAS_ASCII:
        default:
            *width = 1;
            *height = 1;
            break;
    }
}

tui_canvas* tui_canvas_create(int width, int height, tui_canvas_mode mode)
{
    /* Validate dimensions */
    if (width <= 0 || height <= 0) return NULL;
    if (width > 10000 || height > 10000) return NULL;

    int cell_w, cell_h;
    get_cell_dimensions(mode, &cell_w, &cell_h);

    /* Check for integer overflow in pixel dimensions */
    if (width > INT_MAX / cell_w || height > INT_MAX / cell_h) return NULL;
    int pixel_width = width * cell_w;
    int pixel_height = height * cell_h;

    /* Check for overflow in pixel count calculation using size_t */
    if (pixel_width > 0 && (size_t)pixel_height > SIZE_MAX / (size_t)pixel_width) return NULL;
    size_t pixel_count = (size_t)pixel_width * (size_t)pixel_height;

    /* Check for overflow in byte count calculation */
    if (pixel_count > SIZE_MAX - 7) return NULL;
    size_t byte_count = (pixel_count + 7) / 8;
    if (byte_count == 0) byte_count = 1;  /* At least 1 byte */

    tui_canvas *canvas = calloc(1, sizeof(tui_canvas));
    if (!canvas) return NULL;

    canvas->char_width = width;
    canvas->char_height = height;
    canvas->pixel_width = pixel_width;
    canvas->pixel_height = pixel_height;
    canvas->mode = mode;

    canvas->pixels = calloc(byte_count, 1);
    if (!canvas->pixels) {
        free(canvas);
        return NULL;
    }

    return canvas;
}

void tui_canvas_free(tui_canvas *canvas)
{
    if (canvas) {
        free(canvas->pixels);
        free(canvas);
    }
}

void tui_canvas_clear_all(tui_canvas *canvas)
{
    if (!canvas) return;
    int byte_count = (canvas->pixel_width * canvas->pixel_height + 7) / 8;
    memset(canvas->pixels, 0, byte_count);
}

static inline int pixel_index(tui_canvas *canvas, int x, int y)
{
    if (x < 0 || x >= canvas->pixel_width || y < 0 || y >= canvas->pixel_height) {
        return -1;
    }
    return y * canvas->pixel_width + x;
}

void tui_canvas_set(tui_canvas *canvas, int x, int y)
{
    if (!canvas) return;
    int idx = pixel_index(canvas, x, y);
    if (idx < 0) return;
    canvas->pixels[idx / 8] |= (1 << (idx % 8));
}

void tui_canvas_unset(tui_canvas *canvas, int x, int y)
{
    if (!canvas) return;
    int idx = pixel_index(canvas, x, y);
    if (idx < 0) return;
    canvas->pixels[idx / 8] &= ~(1 << (idx % 8));
}

void tui_canvas_toggle(tui_canvas *canvas, int x, int y)
{
    if (!canvas) return;
    int idx = pixel_index(canvas, x, y);
    if (idx < 0) return;
    canvas->pixels[idx / 8] ^= (1 << (idx % 8));
}

int tui_canvas_get(tui_canvas *canvas, int x, int y)
{
    if (!canvas) return 0;
    int idx = pixel_index(canvas, x, y);
    if (idx < 0) return 0;
    return (canvas->pixels[idx / 8] >> (idx % 8)) & 1;
}

void tui_canvas_line(tui_canvas *canvas, int x1, int y1, int x2, int y2)
{
    if (!canvas) return;

    /* Bresenham's line algorithm */
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    while (1) {
        tui_canvas_set(canvas, x1, y1);

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

void tui_canvas_rect(tui_canvas *canvas, int x, int y, int w, int h)
{
    if (!canvas) return;

    /* Top and bottom edges */
    for (int i = 0; i < w; i++) {
        tui_canvas_set(canvas, x + i, y);
        tui_canvas_set(canvas, x + i, y + h - 1);
    }

    /* Left and right edges */
    for (int i = 0; i < h; i++) {
        tui_canvas_set(canvas, x, y + i);
        tui_canvas_set(canvas, x + w - 1, y + i);
    }
}

void tui_canvas_fill_rect(tui_canvas *canvas, int x, int y, int w, int h)
{
    if (!canvas) return;

    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            tui_canvas_set(canvas, x + dx, y + dy);
        }
    }
}

void tui_canvas_circle(tui_canvas *canvas, int cx, int cy, int radius)
{
    if (!canvas) return;

    /* Midpoint circle algorithm */
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        tui_canvas_set(canvas, cx + x, cy + y);
        tui_canvas_set(canvas, cx + y, cy + x);
        tui_canvas_set(canvas, cx - y, cy + x);
        tui_canvas_set(canvas, cx - x, cy + y);
        tui_canvas_set(canvas, cx - x, cy - y);
        tui_canvas_set(canvas, cx - y, cy - x);
        tui_canvas_set(canvas, cx + y, cy - x);
        tui_canvas_set(canvas, cx + x, cy - y);

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

void tui_canvas_fill_circle(tui_canvas *canvas, int cx, int cy, int radius)
{
    if (!canvas) return;

    for (int y = -radius; y <= radius; y++) {
        int dx = (int)(sqrt((double)(radius * radius - y * y)) + 0.5);
        for (int x = -dx; x <= dx; x++) {
            tui_canvas_set(canvas, cx + x, cy + y);
        }
    }
}

void tui_canvas_set_color(tui_canvas *canvas, uint8_t r, uint8_t g, uint8_t b)
{
    if (!canvas) return;
    canvas->color.r = r;
    canvas->color.g = g;
    canvas->color.b = b;
    canvas->color.is_set = 1;
}

void tui_canvas_get_resolution(tui_canvas *canvas, int *width, int *height)
{
    if (!canvas) {
        *width = 0;
        *height = 0;
        return;
    }
    *width = canvas->pixel_width;
    *height = canvas->pixel_height;
}

/* Encode UTF-8 codepoint to string buffer, return bytes written */
static int utf8_encode(uint32_t codepoint, char *buf)
{
    if (codepoint < 0x80) {
        buf[0] = (char)codepoint;
        return 1;
    } else if (codepoint < 0x800) {
        buf[0] = (char)(0xC0 | (codepoint >> 6));
        buf[1] = (char)(0x80 | (codepoint & 0x3F));
        return 2;
    } else if (codepoint < 0x10000) {
        buf[0] = (char)(0xE0 | (codepoint >> 12));
        buf[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        buf[2] = (char)(0x80 | (codepoint & 0x3F));
        return 3;
    } else {
        buf[0] = (char)(0xF0 | (codepoint >> 18));
        buf[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        buf[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        buf[3] = (char)(0x80 | (codepoint & 0x3F));
        return 4;
    }
}

char** tui_canvas_render(tui_canvas *canvas, int *line_count)
{
    if (!canvas) {
        *line_count = 0;
        return NULL;
    }

    *line_count = canvas->char_height;
    char **lines = calloc(canvas->char_height, sizeof(char*));
    if (!lines) return NULL;

    int cell_w, cell_h;
    get_cell_dimensions(canvas->mode, &cell_w, &cell_h);

    /* Allocate line buffers (worst case: 4 bytes per char + null) */
    for (int row = 0; row < canvas->char_height; row++) {
        lines[row] = calloc(canvas->char_width * 4 + 1, 1);
        if (!lines[row]) {
            tui_canvas_render_free(lines, row);
            *line_count = 0;
            return NULL;
        }

        char *p = lines[row];

        for (int col = 0; col < canvas->char_width; col++) {
            int base_x = col * cell_w;
            int base_y = row * cell_h;

            if (canvas->mode == TUI_CANVAS_BRAILLE) {
                /* Convert 2x4 pixel block to braille character */
                uint32_t codepoint = 0x2800;  /* Braille base */

                for (int dx = 0; dx < 2; dx++) {
                    for (int dy = 0; dy < 4; dy++) {
                        if (tui_canvas_get(canvas, base_x + dx, base_y + dy)) {
                            codepoint |= braille_dots[dx][dy];
                        }
                    }
                }

                p += utf8_encode(codepoint, p);

            } else if (canvas->mode == TUI_CANVAS_BLOCK) {
                /* Convert 2x2 pixel block to block character */
                int bits = 0;
                /* Block bit order: top-left=8, top-right=4, bottom-left=2, bottom-right=1 */
                if (tui_canvas_get(canvas, base_x, base_y)) bits |= 8;
                if (tui_canvas_get(canvas, base_x + 1, base_y)) bits |= 4;
                if (tui_canvas_get(canvas, base_x, base_y + 1)) bits |= 2;
                if (tui_canvas_get(canvas, base_x + 1, base_y + 1)) bits |= 1;

                const char *block = block_chars[bits];
                size_t len = strlen(block);
                memcpy(p, block, len);
                p += len;

            } else {
                /* ASCII mode: 1 char per pixel */
                if (tui_canvas_get(canvas, base_x, base_y)) {
                    *p++ = '#';
                } else {
                    *p++ = ' ';
                }
            }
        }
        *p = '\0';
    }

    return lines;
}

void tui_canvas_render_free(char **lines, int line_count)
{
    if (!lines) return;
    for (int i = 0; i < line_count; i++) {
        free(lines[i]);
    }
    free(lines);
}
