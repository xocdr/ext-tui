/*
  +----------------------------------------------------------------------+
  | ext-tui: Character buffer                                           |
  +----------------------------------------------------------------------+
*/

#include "buffer.h"
#include "../text/measure.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

tui_buffer* tui_buffer_create(int width, int height)
{
    /* Validate dimensions */
    if (width <= 0 || height <= 0) return NULL;
    if (width > 10000 || height > 10000) return NULL;

    /* Check for integer overflow before allocation */
    size_t cell_count = (size_t)width * (size_t)height;
    if (cell_count > SIZE_MAX / sizeof(tui_cell)) return NULL;

    tui_buffer *buf = calloc(1, sizeof(tui_buffer));
    if (!buf) return NULL;

    buf->width = width;
    buf->height = height;
    buf->cells = calloc(cell_count, sizeof(tui_cell));

    if (!buf->cells) {
        free(buf);
        return NULL;
    }

    /* Initialize with spaces */
    for (size_t i = 0; i < cell_count; i++) {
        buf->cells[i].codepoint = ' ';
        buf->cells[i].dirty = 1;
    }

    return buf;
}

void tui_buffer_destroy(tui_buffer *buf)
{
    if (buf) {
        free(buf->cells);
        free(buf);
    }
}

int tui_buffer_resize(tui_buffer *buf, int width, int height)
{
    if (!buf) return -1;
    if (width <= 0 || height <= 0) return -1;
    if (width > 10000 || height > 10000) return -1;

    /* Check for integer overflow before allocation */
    size_t cell_count = (size_t)width * (size_t)height;
    if (cell_count > SIZE_MAX / sizeof(tui_cell)) return -1;

    tui_cell *new_cells = calloc(cell_count, sizeof(tui_cell));
    if (!new_cells) return -1;  /* Keep buffer unchanged on failure */

    /* Initialize with spaces */
    for (size_t i = 0; i < cell_count; i++) {
        new_cells[i].codepoint = ' ';
        new_cells[i].dirty = 1;
    }

    /* Copy old content (what fits) */
    int copy_width = buf->width < width ? buf->width : width;
    int copy_height = buf->height < height ? buf->height : height;

    for (int y = 0; y < copy_height; y++) {
        for (int x = 0; x < copy_width; x++) {
            new_cells[y * width + x] = buf->cells[y * buf->width + x];
        }
    }

    free(buf->cells);
    buf->cells = new_cells;
    buf->width = width;
    buf->height = height;
    return 0;
}

void tui_buffer_clear(tui_buffer *buf)
{
    if (!buf) return;

    for (int i = 0; i < buf->width * buf->height; i++) {
        buf->cells[i].codepoint = ' ';
        memset(&buf->cells[i].style, 0, sizeof(tui_style));
        buf->cells[i].dirty = 1;
    }
}

void tui_buffer_set_cell(tui_buffer *buf, int x, int y, uint32_t ch, const tui_style *style)
{
    if (!buf || x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        return;
    }

    tui_cell *cell = &buf->cells[y * buf->width + x];
    cell->codepoint = ch;
    if (style) {
        cell->style = *style;
    }
    cell->dirty = 1;
}

void tui_buffer_write_text(tui_buffer *buf, int x, int y, const char *text, const tui_style *style)
{
    if (!buf || !text) return;

    const char *p = text;
    int cx = x;

    while (*p && cx < buf->width) {
        uint32_t codepoint;
        int bytes = tui_utf8_decode(p, &codepoint);
        int char_width = tui_char_width(codepoint);

        if (char_width > 0) {
            /* Set the main cell */
            tui_buffer_set_cell(buf, cx, y, codepoint, style);

            /* For wide characters (CJK, emoji), mark the next cell as a continuation
             * Use NULL style so it doesn't inherit colors that could bleed */
            if (char_width == 2 && cx + 1 < buf->width) {
                tui_buffer_set_cell(buf, cx + 1, y, 0, NULL);  /* 0 = continuation, no style */
            }

            cx += char_width;
        }

        p += bytes;
    }
}

void tui_buffer_fill_rect(tui_buffer *buf, int x, int y, int w, int h, uint32_t ch, const tui_style *style)
{
    if (!buf) return;

    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            tui_buffer_set_cell(buf, x + dx, y + dy, ch, style);
        }
    }
}

tui_cell* tui_buffer_get_cell(tui_buffer *buf, int x, int y)
{
    if (!buf || x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
        return NULL;
    }
    return &buf->cells[y * buf->width + x];
}

void tui_buffer_mark_all_dirty(tui_buffer *buf)
{
    if (!buf) return;

    for (int i = 0; i < buf->width * buf->height; i++) {
        buf->cells[i].dirty = 1;
    }
}

void tui_buffer_mark_clean(tui_buffer *buf)
{
    if (!buf) return;

    for (int i = 0; i < buf->width * buf->height; i++) {
        buf->cells[i].dirty = 0;
    }
}

char* tui_buffer_to_string(tui_buffer *buf)
{
    if (!buf || !buf->cells) return NULL;

    /* Estimate output size: each cell could be up to ~60 bytes with full ANSI codes + UTF-8 */
    size_t max_size = (size_t)buf->width * buf->height * 60 + buf->height * 10 + 64;
    char *output = malloc(max_size);
    if (!output) return NULL;

    char *p = output;
    char *end = output + max_size - 64;  /* Reserve space for final reset + safety margin */
    tui_style prev_style = {0};
    int first_cell = 1;

    for (int y = 0; y < buf->height && p < end; y++) {
        for (int x = 0; x < buf->width && p < end; x++) {
            tui_cell *cell = &buf->cells[y * buf->width + x];

            /* Skip continuation cells (for wide characters) */
            if (cell->codepoint == 0) continue;

            /* Check if style changed */
            int style_changed = first_cell ||
                cell->style.fg.is_set != prev_style.fg.is_set ||
                (cell->style.fg.is_set && (cell->style.fg.r != prev_style.fg.r ||
                                           cell->style.fg.g != prev_style.fg.g ||
                                           cell->style.fg.b != prev_style.fg.b)) ||
                cell->style.bg.is_set != prev_style.bg.is_set ||
                (cell->style.bg.is_set && (cell->style.bg.r != prev_style.bg.r ||
                                           cell->style.bg.g != prev_style.bg.g ||
                                           cell->style.bg.b != prev_style.bg.b)) ||
                cell->style.bold != prev_style.bold ||
                cell->style.dim != prev_style.dim ||
                cell->style.italic != prev_style.italic ||
                cell->style.underline != prev_style.underline ||
                cell->style.inverse != prev_style.inverse ||
                cell->style.strikethrough != prev_style.strikethrough;

            if (style_changed) {
                size_t remaining = (size_t)(end - p);
                int written;

                /* Reset and apply new style */
                written = snprintf(p, remaining, "\033[0m");
                if (written > 0 && (size_t)written < remaining) p += written;

                if (cell->style.bold) {
                    written = snprintf(p, remaining, "\033[1m");
                    if (written > 0 && (size_t)written < remaining) p += written;
                }
                if (cell->style.dim) {
                    written = snprintf(p, remaining, "\033[2m");
                    if (written > 0 && (size_t)written < remaining) p += written;
                }
                if (cell->style.italic) {
                    written = snprintf(p, remaining, "\033[3m");
                    if (written > 0 && (size_t)written < remaining) p += written;
                }
                if (cell->style.underline) {
                    written = snprintf(p, remaining, "\033[4m");
                    if (written > 0 && (size_t)written < remaining) p += written;
                }
                if (cell->style.inverse) {
                    written = snprintf(p, remaining, "\033[7m");
                    if (written > 0 && (size_t)written < remaining) p += written;
                }
                if (cell->style.strikethrough) {
                    written = snprintf(p, remaining, "\033[9m");
                    if (written > 0 && (size_t)written < remaining) p += written;
                }

                if (cell->style.fg.is_set) {
                    written = snprintf(p, remaining, "\033[38;2;%d;%d;%dm",
                                       cell->style.fg.r, cell->style.fg.g, cell->style.fg.b);
                    if (written > 0 && (size_t)written < remaining) p += written;
                }
                if (cell->style.bg.is_set) {
                    written = snprintf(p, remaining, "\033[48;2;%d;%d;%dm",
                                       cell->style.bg.r, cell->style.bg.g, cell->style.bg.b);
                    if (written > 0 && (size_t)written < remaining) p += written;
                }

                prev_style = cell->style;
                first_cell = 0;
            }

            /* Encode codepoint to UTF-8 (max 4 bytes) */
            if (p + 4 < end) {
                p += tui_utf8_encode(cell->codepoint, p);
            }
        }

        /* Reset style at end of row to prevent bleeding across newlines */
        if (p + 8 < end) {
            int written = snprintf(p, (size_t)(end - p), "\033[0m");
            if (written > 0) p += written;
            prev_style = (tui_style){0};
            first_cell = 1;
        }

        /* Add newline (except for last row) */
        if (y < buf->height - 1 && p < end) {
            *p++ = '\n';
        }
    }

    /* Reset style at end */
    size_t remaining = (size_t)(output + max_size - p);
    snprintf(p, remaining, "\033[0m");
    p[remaining - 1] = '\0';  /* Ensure null termination */

    return output;
}
