/*
  +----------------------------------------------------------------------+
  | ext-tui: Terminal output with diff-based rendering                  |
  +----------------------------------------------------------------------+
*/

#include "output.h"
#include "../terminal/ansi.h"
#include "../text/measure.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define OUTPUT_BUFFER_SIZE 65536

/* Maximum supported terminal dimensions for buffer size validation.
 * Larger terminals will still work but may use more chunks. */
#define MAX_VALIDATED_WIDTH 500
#define MAX_VALIDATED_HEIGHT 500

/**
 * Write all bytes to file descriptor, handling partial writes and EINTR.
 * Returns 0 on success, -1 on error.
 */
static int write_all(int fd, const void *buf, size_t len)
{
    const char *p = buf;
    size_t remaining = len;

    while (remaining > 0) {
        ssize_t written = write(fd, p, remaining);
        if (written < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        p += written;
        remaining -= (size_t)written;
    }
    return 0;
}

tui_output* tui_output_create(int width, int height)
{
    tui_output *out = calloc(1, sizeof(tui_output));
    if (!out) return NULL;

    out->front = tui_buffer_create(width, height);
    out->back = tui_buffer_create(width, height);
    out->mode = TUI_OUTPUT_NORMAL;
    out->cursor_visible = 1;

    if (!out->front || !out->back) {
        tui_buffer_destroy(out->front);
        tui_buffer_destroy(out->back);
        free(out);
        return NULL;
    }

    return out;
}

void tui_output_destroy(tui_output *out)
{
    if (out) {
        tui_buffer_destroy(out->front);
        tui_buffer_destroy(out->back);
        free(out);
    }
}

void tui_output_enter_alternate(tui_output *out)
{
    if (!out || out->mode == TUI_OUTPUT_ALTERNATE) return;

    char buf[64];
    size_t len;

    /* Enter alternate screen buffer */
    tui_ansi_alternate_screen_enter(buf, &len);
    write_all(STDOUT_FILENO, buf, len);

    /* Clear screen and move cursor to home position */
    tui_ansi_clear_screen(buf, &len);
    write_all(STDOUT_FILENO, buf, len);

    /* Hide cursor */
    tui_ansi_cursor_hide(buf, &len);
    write_all(STDOUT_FILENO, buf, len);

    out->mode = TUI_OUTPUT_ALTERNATE;
}

void tui_output_exit_alternate(tui_output *out)
{
    if (!out || out->mode == TUI_OUTPUT_NORMAL) return;

    char buf[32];
    size_t len;

    tui_ansi_cursor_show(buf, &len);
    write_all(STDOUT_FILENO, buf, len);

    tui_ansi_alternate_screen_exit(buf, &len);
    write_all(STDOUT_FILENO, buf, len);

    out->mode = TUI_OUTPUT_NORMAL;
}

/**
 * Compare two styles for equality.
 * Uses explicit field comparison instead of memcmp to avoid
 * issues with struct padding bytes containing garbage.
 */
static int styles_equal(const tui_style *a, const tui_style *b)
{
    return a->fg.is_set == b->fg.is_set &&
           a->fg.r == b->fg.r && a->fg.g == b->fg.g && a->fg.b == b->fg.b &&
           a->bg.is_set == b->bg.is_set &&
           a->bg.r == b->bg.r && a->bg.g == b->bg.g && a->bg.b == b->bg.b &&
           a->bold == b->bold && a->dim == b->dim &&
           a->italic == b->italic && a->underline == b->underline &&
           a->inverse == b->inverse && a->strikethrough == b->strikethrough;
}

static size_t apply_style_diff(char *buf, const tui_style *old_style, const tui_style *new_style)
{
    size_t len = 0;

    /* Check if we need to reset first - this includes:
     * - Removing text styles (bold, dim, etc.)
     * - Removing foreground or background colors */
    int need_reset = 0;
    if ((old_style->bold && !new_style->bold) ||
        (old_style->dim && !new_style->dim) ||
        (old_style->italic && !new_style->italic) ||
        (old_style->underline && !new_style->underline) ||
        (old_style->inverse && !new_style->inverse) ||
        (old_style->strikethrough && !new_style->strikethrough) ||
        (old_style->fg.is_set && !new_style->fg.is_set) ||
        (old_style->bg.is_set && !new_style->bg.is_set)) {
        need_reset = 1;
    }

    if (need_reset) {
        size_t slen;
        tui_ansi_reset(buf + len, &slen);
        len += slen;
    }

    /* Apply text styles */
    if (new_style->bold) {
        size_t slen;
        tui_ansi_bold(buf + len, &slen);
        len += slen;
    }
    if (new_style->dim) {
        size_t slen;
        tui_ansi_dim(buf + len, &slen);
        len += slen;
    }
    if (new_style->italic) {
        size_t slen;
        tui_ansi_italic(buf + len, &slen);
        len += slen;
    }
    if (new_style->underline) {
        size_t slen;
        tui_ansi_underline(buf + len, &slen);
        len += slen;
    }
    if (new_style->inverse) {
        size_t slen;
        tui_ansi_inverse(buf + len, &slen);
        len += slen;
    }
    if (new_style->strikethrough) {
        size_t slen;
        tui_ansi_strikethrough(buf + len, &slen);
        len += slen;
    }

    /* Apply foreground color */
    if (new_style->fg.is_set) {
        size_t slen;
        tui_ansi_fg_rgb(buf + len, &slen, new_style->fg.r, new_style->fg.g, new_style->fg.b);
        len += slen;
    }

    /* Apply background color */
    if (new_style->bg.is_set) {
        size_t slen;
        tui_ansi_bg_rgb(buf + len, &slen, new_style->bg.r, new_style->bg.g, new_style->bg.b);
        len += slen;
    }

    return len;
}

/* Maximum size for a single ANSI escape sequence (generous for RGB colors + attributes) */
#define ANSI_BUFFER_SIZE 256

void tui_output_render(tui_output *out, tui_buffer *buf)
{
    if (!out || !buf) return;

    char output[OUTPUT_BUFFER_SIZE];
    size_t output_len = 0;
    char ansi[ANSI_BUFFER_SIZE];
    size_t ansi_len;

    /* Begin synchronized output (DEC mode 2026) to prevent flicker */
    tui_ansi_sync_start(ansi, &ansi_len);
    memcpy(output + output_len, ansi, ansi_len);
    output_len += ansi_len;

    tui_style current_style = {0};
    int last_x = -1, last_y = -1;

    for (int y = 0; y < buf->height && y < out->front->height; y++) {
        for (int x = 0; x < buf->width && x < out->front->width; x++) {
            tui_cell *new_cell = tui_buffer_get_cell(buf, x, y);
            tui_cell *old_cell = tui_buffer_get_cell(out->front, x, y);

            /* Skip continuation cells (part of wide characters) */
            if (new_cell->codepoint == 0) {
                continue;
            }

            /* Skip if unchanged */
            if (new_cell->codepoint == old_cell->codepoint &&
                styles_equal(&new_cell->style, &old_cell->style)) {
                continue;
            }

            /* Move cursor if not sequential */
            if (x != last_x + 1 || y != last_y) {
                tui_ansi_cursor_move(ansi, &ansi_len, x, y);
                /* Flush buffer if it would overflow */
                if (output_len + ansi_len >= OUTPUT_BUFFER_SIZE) {
                    write_all(STDOUT_FILENO, output, output_len);
                    output_len = 0;
                }
                memcpy(output + output_len, ansi, ansi_len);
                output_len += ansi_len;
            }

            /* Apply style if changed */
            if (!styles_equal(&new_cell->style, &current_style)) {
                ansi_len = apply_style_diff(ansi, &current_style, &new_cell->style);
                /* Flush buffer if it would overflow */
                if (output_len + ansi_len >= OUTPUT_BUFFER_SIZE) {
                    write_all(STDOUT_FILENO, output, output_len);
                    output_len = 0;
                }
                memcpy(output + output_len, ansi, ansi_len);
                output_len += ansi_len;
                current_style = new_cell->style;
            }

            /* Write character with proper UTF-8 encoding */
            char utf8_buf[4];
            int utf8_len = tui_utf8_encode(new_cell->codepoint, utf8_buf);
            /* Flush buffer if it would overflow */
            if (output_len + (size_t)utf8_len >= OUTPUT_BUFFER_SIZE) {
                write_all(STDOUT_FILENO, output, output_len);
                output_len = 0;
            }
            memcpy(output + output_len, utf8_buf, (size_t)utf8_len);
            output_len += (size_t)utf8_len;

            /* Update front buffer */
            *old_cell = *new_cell;

            /* Track cursor position accounting for wide chars */
            int char_width = tui_char_width(new_cell->codepoint);
            last_x = x + char_width - 1;
            last_y = y;
        }
    }

    /* Reset style at end */
    if (output_len > 0) {
        tui_ansi_reset(ansi, &ansi_len);
        /* Flush buffer if it would overflow */
        if (output_len + ansi_len >= OUTPUT_BUFFER_SIZE) {
            write_all(STDOUT_FILENO, output, output_len);
            output_len = 0;
        }
        memcpy(output + output_len, ansi, ansi_len);
        output_len += ansi_len;
    }

    /* End synchronized output (DEC mode 2026) - terminal renders atomically */
    tui_ansi_sync_end(ansi, &ansi_len);
    if (output_len + ansi_len >= OUTPUT_BUFFER_SIZE) {
        write_all(STDOUT_FILENO, output, output_len);
        output_len = 0;
    }
    memcpy(output + output_len, ansi, ansi_len);
    output_len += ansi_len;

    /* Write all output at once */
    if (output_len > 0) {
        write_all(STDOUT_FILENO, output, output_len);
    }
}

void tui_output_flush(tui_output *out)
{
    /* Force full redraw */
    if (out && out->front) {
        tui_buffer_mark_all_dirty(out->front);
    }
}

void tui_output_show_cursor(tui_output *out)
{
    if (!out) return;

    char buf[32];
    size_t len;
    tui_ansi_cursor_show(buf, &len);
    write_all(STDOUT_FILENO, buf, len);
    out->cursor_visible = 1;
}

void tui_output_hide_cursor(tui_output *out)
{
    if (!out) return;

    char buf[32];
    size_t len;
    tui_ansi_cursor_hide(buf, &len);
    write_all(STDOUT_FILENO, buf, len);
    out->cursor_visible = 0;
}

void tui_output_move_cursor(tui_output *out, int x, int y)
{
    if (!out) return;

    char buf[32];
    size_t len;
    tui_ansi_cursor_move(buf, &len, x, y);
    write_all(STDOUT_FILENO, buf, len);
    out->cursor_x = x;
    out->cursor_y = y;
}
