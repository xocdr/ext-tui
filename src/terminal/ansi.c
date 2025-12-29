/*
  +----------------------------------------------------------------------+
  | ext-tui: ANSI escape code generation                                |
  +----------------------------------------------------------------------+
*/

#include "ansi.h"
#include <stdio.h>
#include <string.h>

/* ESC[ sequences */
#define ESC "\x1b["

/* Maximum buffer size for ANSI sequences - callers must provide at least this much */
#define ANSI_BUF_SIZE 32

/**
 * Safe snprintf wrapper that validates the return value.
 * Sets len to actual bytes written (capped at buffer size - 1).
 * On error, sets len to 0 and null-terminates the buffer.
 */
static inline void safe_snprintf_len(char *buf, size_t buf_size, size_t *len, int result)
{
    if (result < 0) {
        /* Encoding error */
        *len = 0;
        if (buf_size > 0) buf[0] = '\0';
    } else if ((size_t)result >= buf_size) {
        /* Truncation occurred - output was truncated to buf_size-1 chars */
        *len = buf_size - 1;
    } else {
        /* Success */
        *len = (size_t)result;
    }
}

void tui_ansi_cursor_hide(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "?25l"));
}

void tui_ansi_cursor_show(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "?25h"));
}

void tui_ansi_cursor_move(char *buf, size_t *len, int x, int y)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "%d;%dH", y + 1, x + 1));
}

void tui_ansi_cursor_save(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "s"));
}

void tui_ansi_cursor_restore(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "u"));
}

void tui_ansi_clear_screen(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "2J" ESC "H"));
}

void tui_ansi_clear_line(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "2K"));
}

void tui_ansi_alternate_screen_enter(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "?1049h"));
}

void tui_ansi_alternate_screen_exit(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "?1049l"));
}

void tui_ansi_fg_256(char *buf, size_t *len, uint8_t color)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "38;5;%dm", color));
}

void tui_ansi_bg_256(char *buf, size_t *len, uint8_t color)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "48;5;%dm", color));
}

void tui_ansi_fg_rgb(char *buf, size_t *len, uint8_t r, uint8_t g, uint8_t b)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "38;2;%d;%d;%dm", r, g, b));
}

void tui_ansi_bg_rgb(char *buf, size_t *len, uint8_t r, uint8_t g, uint8_t b)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "48;2;%d;%d;%dm", r, g, b));
}

void tui_ansi_reset(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "0m"));
}

void tui_ansi_bold(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "1m"));
}

void tui_ansi_dim(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "2m"));
}

void tui_ansi_italic(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "3m"));
}

void tui_ansi_underline(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "4m"));
}

void tui_ansi_inverse(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "7m"));
}

void tui_ansi_strikethrough(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "9m"));
}

/* Additional cursor and screen operations */

void tui_ansi_erase_line_start(char *buf, size_t *len)
{
    /* Erase from cursor to beginning of line */
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "1K"));
}

void tui_ansi_erase_line_end(char *buf, size_t *len)
{
    /* Erase from cursor to end of line */
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "0K"));
}

void tui_ansi_scroll_up(char *buf, size_t *len, int lines)
{
    /* Scroll screen up by n lines (new lines at bottom) */
    if (lines <= 0) lines = 1;
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "%dS", lines));
}

void tui_ansi_scroll_down(char *buf, size_t *len, int lines)
{
    /* Scroll screen down by n lines (new lines at top) */
    if (lines <= 0) lines = 1;
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "%dT", lines));
}

void tui_ansi_cursor_next_line(char *buf, size_t *len, int lines)
{
    /* Move cursor to beginning of line n lines down */
    if (lines <= 0) lines = 1;
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "%dE", lines));
}

void tui_ansi_cursor_prev_line(char *buf, size_t *len, int lines)
{
    /* Move cursor to beginning of line n lines up */
    if (lines <= 0) lines = 1;
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "%dF", lines));
}

void tui_ansi_cursor_column(char *buf, size_t *len, int col)
{
    /* Move cursor to specific column */
    if (col < 0) col = 0;
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "%dG", col + 1));
}

void tui_ansi_erase_screen_end(char *buf, size_t *len)
{
    /* Erase from cursor to end of screen */
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "0J"));
}

void tui_ansi_erase_screen_start(char *buf, size_t *len)
{
    /* Erase from cursor to beginning of screen */
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "1J"));
}

/* Color conversion utilities */

/**
 * Convert RGB color to ANSI 256-color palette index.
 *
 * The 256-color palette is organized as:
 * - 0-15: Standard colors (system-dependent)
 * - 16-231: 6x6x6 color cube (216 colors)
 * - 232-255: Grayscale ramp (24 shades)
 *
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return ANSI 256-color palette index (0-255)
 */
int tui_rgb_to_ansi256(uint8_t r, uint8_t g, uint8_t b)
{
    /* Check if it's a grayscale color */
    if (r == g && g == b) {
        if (r < 8) {
            return 16;  /* Black in the cube */
        }
        if (r > 248) {
            return 231; /* White in the cube */
        }
        /* Use grayscale ramp (232-255): 24 shades from dark to light */
        /* Each shade covers about 10 units (256/24 ≈ 10.67) */
        return 232 + ((r - 8) / 10);
    }

    /* Map to 6x6x6 color cube (indices 16-231) */
    /* Each RGB component maps to 0-5 */
    int ri = (r < 48) ? 0 : (r < 115) ? 1 : (r - 35) / 40;
    int gi = (g < 48) ? 0 : (g < 115) ? 1 : (g - 35) / 40;
    int bi = (b < 48) ? 0 : (b < 115) ? 1 : (b - 35) / 40;

    /* Clamp to 0-5 range */
    if (ri > 5) ri = 5;
    if (gi > 5) gi = 5;
    if (bi > 5) bi = 5;

    return 16 + (36 * ri) + (6 * gi) + bi;
}
