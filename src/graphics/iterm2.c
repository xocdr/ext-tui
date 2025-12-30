/*
  +----------------------------------------------------------------------+
  | ext-tui: iTerm2 Inline Images Protocol support                       |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "iterm2.h"
#include "../terminal/ansi.h"
#include "../terminal/capabilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Write all bytes to stdout.
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

int tui_iterm2_is_supported(void)
{
    const char *term_program = getenv("TERM_PROGRAM");
    const char *lc_terminal = getenv("LC_TERMINAL");

    /* Check TERM_PROGRAM for iTerm.app */
    if (term_program && strcmp(term_program, "iTerm.app") == 0) {
        return 1;
    }

    /* Check LC_TERMINAL for iTerm2 (used in SSH sessions) */
    if (lc_terminal && strcmp(lc_terminal, "iTerm2") == 0) {
        return 1;
    }

    /* Also check terminal type from capabilities */
    const tui_capabilities *caps = tui_get_capabilities();
    if (caps->terminal == TUI_TERM_ITERM2) {
        return 1;
    }

    return 0;
}

int tui_iterm2_build_escape(char *buf, size_t buf_size,
                            const tui_image *img, int cols, int rows)
{
    if (!buf || !img || !img->data || img->data_len == 0) {
        return -1;
    }

    /* Calculate base64 encoded size */
    size_t b64_len = tui_base64_encode_len(img->data_len);

    /* Build parameter string */
    char params[256];
    int params_len;

    if (cols > 0 && rows > 0) {
        params_len = snprintf(params, sizeof(params),
            "inline=1;width=%d;height=%d;preserveAspectRatio=0",
            cols, rows);
    } else if (cols > 0) {
        params_len = snprintf(params, sizeof(params),
            "inline=1;width=%d;preserveAspectRatio=1", cols);
    } else if (rows > 0) {
        params_len = snprintf(params, sizeof(params),
            "inline=1;height=%d;preserveAspectRatio=1", rows);
    } else {
        params_len = snprintf(params, sizeof(params),
            "inline=1;preserveAspectRatio=1");
    }

    if (params_len < 0 || (size_t)params_len >= sizeof(params)) {
        return -1;
    }

    /* Check if buffer is large enough:
     * ESC ] 1337 ; File= <params> : <base64> BEL
     * = 7 + 6 + params_len + 1 + b64_len + 1 = 15 + params_len + b64_len
     */
    size_t needed = 15 + (size_t)params_len + b64_len + 1;  /* +1 for null */
    if (buf_size < needed) {
        return -1;
    }

    /* Build escape sequence header */
    int header_len = snprintf(buf, buf_size, "\x1b]1337;File=%s:", params);
    if (header_len < 0) {
        return -1;
    }

    /* Base64 encode the image data */
    int b64_written = tui_base64_encode(
        (const char *)img->data, img->data_len,
        buf + header_len, buf_size - (size_t)header_len - 1);

    if (b64_written < 0) {
        return -1;
    }

    /* Add BEL terminator */
    size_t total = (size_t)header_len + (size_t)b64_written;
    buf[total] = '\x07';
    total++;
    buf[total] = '\0';

    return (int)total;
}

int tui_iterm2_display_inline(tui_image *img, int cols, int rows)
{
    if (!img || !img->data || img->data_len == 0) {
        return -1;
    }

    if (img->state == TUI_IMAGE_STATE_EMPTY) {
        return -1;
    }

    /* Calculate buffer size needed */
    size_t b64_len = tui_base64_encode_len(img->data_len);
    size_t buf_size = b64_len + 512;  /* Extra space for escape sequence */

    char *buf = malloc(buf_size);
    if (!buf) {
        return -1;
    }

    int len = tui_iterm2_build_escape(buf, buf_size, img, cols, rows);
    if (len < 0) {
        free(buf);
        return -1;
    }

    write_stdout(buf, (size_t)len);
    free(buf);

    img->state = TUI_IMAGE_STATE_DISPLAYED;
    img->display_cols = cols;
    img->display_rows = rows;

    return 0;
}

int tui_iterm2_display(tui_image *img, int x, int y, int cols, int rows)
{
    if (!img) {
        return -1;
    }

    /* Move cursor to position first */
    char cursor_buf[32];
    size_t cursor_len;
    tui_ansi_cursor_move(cursor_buf, &cursor_len, x, y);
    write_stdout(cursor_buf, cursor_len);

    /* Display the image */
    int result = tui_iterm2_display_inline(img, cols, rows);

    if (result == 0) {
        img->display_x = x;
        img->display_y = y;
    }

    return result;
}
