/*
  +----------------------------------------------------------------------+
  | ext-tui: Terminal handling                                          |
  +----------------------------------------------------------------------+
*/

#include "terminal.h"
#include <unistd.h>
#include <sys/ioctl.h>

struct termios tui_original_termios;
static int raw_mode_enabled = 0;

int tui_terminal_enable_raw_mode(void)
{
    struct termios raw;

    if (raw_mode_enabled) {
        return 0;
    }

    if (!isatty(STDIN_FILENO)) {
        return -1;
    }

    if (tcgetattr(STDIN_FILENO, &tui_original_termios) == -1) {
        return -1;
    }

    raw = tui_original_termios;

    /* Input modes: no break, no CR to NL, no parity check, no strip char,
       no start/stop output control. */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    /* Output modes: disable post processing */
    raw.c_oflag &= ~(OPOST);

    /* Control modes: set 8 bit chars */
    raw.c_cflag |= (CS8);

    /* Local modes: echo off, canonical off, no extended functions,
       no signal chars (^Z, ^C) */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    /* Control chars: set return condition: min number of bytes and timer. */
    raw.c_cc[VMIN] = 0;  /* Return each byte, or zero for timeout. */
    raw.c_cc[VTIME] = 1; /* 100 ms timeout (unit is tenths of a second). */

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        return -1;
    }

    raw_mode_enabled = 1;
    return 0;
}

int tui_terminal_disable_raw_mode(void)
{
    if (!raw_mode_enabled) {
        return 0;
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tui_original_termios) == -1) {
        return -1;
    }

    raw_mode_enabled = 0;
    return 0;
}

int tui_terminal_is_raw_mode(void)
{
    return raw_mode_enabled;
}

int tui_terminal_get_size(int *width, int *height)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        *width = 80;
        *height = 24;
        return -1;
    }

    *width = ws.ws_col;
    *height = ws.ws_row;
    return 0;
}

int tui_terminal_is_tty(void)
{
    return isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
}

/* Bracketed paste mode */

#include "ansi.h"
#include <stdio.h>

static int bracketed_paste_enabled = 0;

int tui_terminal_enable_bracketed_paste(void)
{
    if (bracketed_paste_enabled) return 0;
    if (!tui_terminal_is_tty()) return -1;

    char buf[32];
    size_t len;
    tui_ansi_bracketed_paste_enable(buf, &len);
    if (write(STDOUT_FILENO, buf, len) != (ssize_t)len) {
        return -1;
    }

    bracketed_paste_enabled = 1;
    return 0;
}

int tui_terminal_disable_bracketed_paste(void)
{
    if (!bracketed_paste_enabled) return 0;

    char buf[32];
    size_t len;
    tui_ansi_bracketed_paste_disable(buf, &len);
    if (write(STDOUT_FILENO, buf, len) != (ssize_t)len) {
        return -1;
    }

    bracketed_paste_enabled = 0;
    return 0;
}

int tui_terminal_is_bracketed_paste_enabled(void)
{
    return bracketed_paste_enabled;
}

/* Mouse tracking */

static tui_mouse_mode current_mouse_mode = TUI_MOUSE_MODE_OFF;

int tui_terminal_enable_mouse(tui_mouse_mode mode)
{
    if (mode == TUI_MOUSE_MODE_OFF) {
        return tui_terminal_disable_mouse();
    }

    if (current_mouse_mode == mode) return 0;
    if (!tui_terminal_is_tty()) return -1;

    char buf[64];
    size_t len;
    size_t total_len = 0;

    /* First disable any existing mode */
    if (current_mouse_mode != TUI_MOUSE_MODE_OFF) {
        tui_ansi_mouse_disable(buf, &len);
        total_len = len;
        tui_ansi_mouse_sgr_disable(buf + total_len, &len);
        total_len += len;
        if (write(STDOUT_FILENO, buf, total_len) != (ssize_t)total_len) {
            return -1;
        }
        total_len = 0;
    }

    /* Enable new mode */
    tui_ansi_mouse_enable(buf, &len, mode);
    total_len = len;

    /* Always enable SGR mode for extended coordinate support */
    tui_ansi_mouse_sgr_enable(buf + total_len, &len);
    total_len += len;

    if (write(STDOUT_FILENO, buf, total_len) != (ssize_t)total_len) {
        return -1;
    }

    current_mouse_mode = mode;
    return 0;
}

int tui_terminal_disable_mouse(void)
{
    if (current_mouse_mode == TUI_MOUSE_MODE_OFF) return 0;

    char buf[64];
    size_t len;
    size_t total_len = 0;

    /* Disable SGR mode */
    tui_ansi_mouse_sgr_disable(buf, &len);
    total_len = len;

    /* Disable mouse tracking */
    tui_ansi_mouse_disable(buf + total_len, &len);
    total_len += len;

    if (write(STDOUT_FILENO, buf, total_len) != (ssize_t)total_len) {
        return -1;
    }

    current_mouse_mode = TUI_MOUSE_MODE_OFF;
    return 0;
}

tui_mouse_mode tui_terminal_get_mouse_mode(void)
{
    return current_mouse_mode;
}
