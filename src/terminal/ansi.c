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

void tui_ansi_cursor_hide(char *buf, size_t *len)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "?25l");
}

void tui_ansi_cursor_show(char *buf, size_t *len)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "?25h");
}

void tui_ansi_cursor_move(char *buf, size_t *len, int x, int y)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "%d;%dH", y + 1, x + 1);
}

void tui_ansi_cursor_save(char *buf, size_t *len)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "s");
}

void tui_ansi_cursor_restore(char *buf, size_t *len)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "u");
}

void tui_ansi_clear_screen(char *buf, size_t *len)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "2J" ESC "H");
}

void tui_ansi_clear_line(char *buf, size_t *len)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "2K");
}

void tui_ansi_alternate_screen_enter(char *buf, size_t *len)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "?1049h");
}

void tui_ansi_alternate_screen_exit(char *buf, size_t *len)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "?1049l");
}

void tui_ansi_fg_256(char *buf, size_t *len, uint8_t color)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "38;5;%dm", color);
}

void tui_ansi_bg_256(char *buf, size_t *len, uint8_t color)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "48;5;%dm", color);
}

void tui_ansi_fg_rgb(char *buf, size_t *len, uint8_t r, uint8_t g, uint8_t b)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "38;2;%d;%d;%dm", r, g, b);
}

void tui_ansi_bg_rgb(char *buf, size_t *len, uint8_t r, uint8_t g, uint8_t b)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "48;2;%d;%d;%dm", r, g, b);
}

void tui_ansi_reset(char *buf, size_t *len)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "0m");
}

void tui_ansi_bold(char *buf, size_t *len)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "1m");
}

void tui_ansi_dim(char *buf, size_t *len)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "2m");
}

void tui_ansi_italic(char *buf, size_t *len)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "3m");
}

void tui_ansi_underline(char *buf, size_t *len)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "4m");
}

void tui_ansi_inverse(char *buf, size_t *len)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "7m");
}

void tui_ansi_strikethrough(char *buf, size_t *len)
{
    *len = snprintf(buf, ANSI_BUF_SIZE, ESC "9m");
}
