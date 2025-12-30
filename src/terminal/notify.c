/*
  +----------------------------------------------------------------------+
  | ext-tui: Terminal notifications (bell, flash, desktop)              |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "notify.h"
#include "capabilities.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

/* Buffer sizes */
#define NOTIFY_BUF_SIZE 32
#define NOTIFY_MSG_SIZE 1024

/**
 * Write all bytes to stdout.
 * Handles partial writes.
 */
static void write_stdout(const char *buf, size_t len)
{
    size_t written = 0;
    while (written < len) {
        ssize_t n = write(STDOUT_FILENO, buf + written, len - written);
        if (n <= 0) break;
        written += (size_t)n;
    }
}

/**
 * Sleep for specified milliseconds.
 */
static void sleep_ms(int ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

void tui_ansi_bell(char *buf, size_t *len)
{
    buf[0] = '\x07';  /* BEL character */
    buf[1] = '\0';
    *len = 1;
}

void tui_ansi_flash_start(char *buf, size_t *len)
{
    /* DECSCNM - Set Reverse Video: ESC [ ? 5 h */
    int n = snprintf(buf, NOTIFY_BUF_SIZE, "\x1b[?5h");
    *len = (n > 0 && n < NOTIFY_BUF_SIZE) ? (size_t)n : 0;
}

void tui_ansi_flash_end(char *buf, size_t *len)
{
    /* DECSCNM - Reset Reverse Video: ESC [ ? 5 l */
    int n = snprintf(buf, NOTIFY_BUF_SIZE, "\x1b[?5l");
    *len = (n > 0 && n < NOTIFY_BUF_SIZE) ? (size_t)n : 0;
}

void tui_ansi_notify_osc9(char *buf, size_t buf_size, size_t *len, const char *message)
{
    /* OSC 9 ; message BEL - iTerm2 notification */
    int n = snprintf(buf, buf_size, "\x1b]9;%s\x07", message ? message : "");
    if (n > 0 && (size_t)n < buf_size) {
        *len = (size_t)n;
    } else {
        *len = 0;
    }
}

void tui_ansi_notify_osc99(char *buf, size_t buf_size, size_t *len,
                           const char *title, const char *body)
{
    /*
     * OSC 99 - Kitty notification protocol
     * Format: ESC ] 99 ; i=<id>:d=<done>:p=<payload_type> ; <payload> BEL
     * - i=1 means identifier (can be any unique value)
     * - d=0 means notification is complete
     * - p=body means payload is the body (title in separate field)
     *
     * Simpler form: ESC ] 99 ; d=0 ; title BEL (title only)
     * With body: ESC ] 99 ; d=0:p=body ; body ESC \ ESC ] 99 ; d=1 ; title BEL
     *
     * For simplicity, we use the basic form with just title
     */
    int n;
    if (body && body[0]) {
        /* Include body as part of title for simplicity */
        n = snprintf(buf, buf_size, "\x1b]99;d=0;%s: %s\x07", title ? title : "", body);
    } else {
        n = snprintf(buf, buf_size, "\x1b]99;d=0;%s\x07", title ? title : "");
    }
    if (n > 0 && (size_t)n < buf_size) {
        *len = (size_t)n;
    } else {
        *len = 0;
    }
}

void tui_ansi_notify_osc777(char *buf, size_t buf_size, size_t *len,
                            const char *title, const char *body)
{
    /* OSC 777 ; notify ; title ; body BEL - VTE/libnotify notification */
    int n = snprintf(buf, buf_size, "\x1b]777;notify;%s;%s\x07",
                     title ? title : "",
                     body ? body : "");
    if (n > 0 && (size_t)n < buf_size) {
        *len = (size_t)n;
    } else {
        *len = 0;
    }
}

void tui_bell(void)
{
    char buf[NOTIFY_BUF_SIZE];
    size_t len;

    tui_ansi_bell(buf, &len);
    write_stdout(buf, len);
}

void tui_flash(void)
{
    char buf[NOTIFY_BUF_SIZE];
    size_t len;

    /* Enable reverse video */
    tui_ansi_flash_start(buf, &len);
    write_stdout(buf, len);

    /* Wait 100ms for visible flash */
    sleep_ms(100);

    /* Disable reverse video */
    tui_ansi_flash_end(buf, &len);
    write_stdout(buf, len);
}

int tui_notify(const char *title, const char *body, tui_notify_priority priority)
{
    const tui_capabilities *caps;
    char buf[NOTIFY_MSG_SIZE];
    size_t len;

    (void)priority;  /* TODO: Use priority for urgent notifications */

    if (!title || !title[0]) {
        return -1;  /* Title is required */
    }

    caps = tui_get_capabilities();

    switch (caps->terminal) {
        case TUI_TERM_KITTY:
            tui_ansi_notify_osc99(buf, sizeof(buf), &len, title, body);
            break;

        case TUI_TERM_ITERM2:
            /* iTerm2 uses OSC 9 - combine title and body */
            if (body && body[0]) {
                char combined[NOTIFY_MSG_SIZE];
                snprintf(combined, sizeof(combined), "%s: %s", title, body);
                tui_ansi_notify_osc9(buf, sizeof(buf), &len, combined);
            } else {
                tui_ansi_notify_osc9(buf, sizeof(buf), &len, title);
            }
            break;

        case TUI_TERM_VTE:
        case TUI_TERM_KONSOLE:
            /* VTE-based terminals use OSC 777 */
            tui_ansi_notify_osc777(buf, sizeof(buf), &len, title, body);
            break;

        case TUI_TERM_WEZTERM:
            /* WezTerm supports OSC 9 */
            if (body && body[0]) {
                char combined[NOTIFY_MSG_SIZE];
                snprintf(combined, sizeof(combined), "%s: %s", title, body);
                tui_ansi_notify_osc9(buf, sizeof(buf), &len, combined);
            } else {
                tui_ansi_notify_osc9(buf, sizeof(buf), &len, title);
            }
            break;

        default:
            /* Try OSC 9 as fallback - most widely supported */
            if (body && body[0]) {
                char combined[NOTIFY_MSG_SIZE];
                snprintf(combined, sizeof(combined), "%s: %s", title, body);
                tui_ansi_notify_osc9(buf, sizeof(buf), &len, combined);
            } else {
                tui_ansi_notify_osc9(buf, sizeof(buf), &len, title);
            }
            break;
    }

    if (len > 0) {
        write_stdout(buf, len);
        return 0;
    }

    return -1;
}
