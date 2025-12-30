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

/* Synchronized output (DEC mode 2026) */

void tui_ansi_sync_start(char *buf, size_t *len)
{
    /* Begin synchronized update - terminal buffers all output until sync_end */
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "?2026h"));
}

void tui_ansi_sync_end(char *buf, size_t *len)
{
    /* End synchronized update - terminal renders buffered content atomically */
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "?2026l"));
}

/* Hyperlinks (OSC 8) */

void tui_ansi_hyperlink_start(char *buf, size_t buf_size, size_t *len, const char *url, const char *id)
{
    /*
     * OSC 8 hyperlink format: ESC ] 8 ; params ; URI ST
     * Where ST is ESC \ (String Terminator)
     * params can include id=<id> for grouping links
     */
    if (!url) {
        *len = 0;
        return;
    }

    int result;
    if (id && id[0] != '\0') {
        result = snprintf(buf, buf_size, "\x1b]8;id=%s;%s\x1b\\", id, url);
    } else {
        result = snprintf(buf, buf_size, "\x1b]8;;%s\x1b\\", url);
    }

    if (result < 0) {
        *len = 0;
        if (buf_size > 0) buf[0] = '\0';
    } else if ((size_t)result >= buf_size) {
        *len = buf_size - 1;
    } else {
        *len = (size_t)result;
    }
}

void tui_ansi_hyperlink_end(char *buf, size_t *len)
{
    /* End hyperlink: ESC ] 8 ; ; ST */
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, "\x1b]8;;\x1b\\"));
}

/* Bracketed paste mode */

void tui_ansi_bracketed_paste_enable(char *buf, size_t *len)
{
    /* Enable bracketed paste: ESC [ ? 2004 h */
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "?2004h"));
}

void tui_ansi_bracketed_paste_disable(char *buf, size_t *len)
{
    /* Disable bracketed paste: ESC [ ? 2004 l */
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "?2004l"));
}

/* Mouse tracking modes */

void tui_ansi_mouse_enable(char *buf, size_t *len, tui_mouse_mode mode)
{
    int result;
    switch (mode) {
        case TUI_MOUSE_MODE_CLICK:
            /* Basic mouse tracking (1000) - clicks only */
            result = snprintf(buf, ANSI_BUF_SIZE, ESC "?1000h");
            break;
        case TUI_MOUSE_MODE_BUTTON:
            /* Button event tracking (1002) - press/release */
            result = snprintf(buf, ANSI_BUF_SIZE, ESC "?1002h");
            break;
        case TUI_MOUSE_MODE_ALL:
            /* All motion tracking (1003) - includes hover/movement */
            result = snprintf(buf, ANSI_BUF_SIZE, ESC "?1003h");
            break;
        default:
            *len = 0;
            return;
    }
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, result);
}

void tui_ansi_mouse_disable(char *buf, size_t *len)
{
    /* Disable all mouse modes */
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len,
        snprintf(buf, ANSI_BUF_SIZE, ESC "?1000l" ESC "?1002l" ESC "?1003l"));
}

void tui_ansi_mouse_sgr_enable(char *buf, size_t *len)
{
    /* Enable SGR extended mouse mode (1006) - better for coordinates > 223 */
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "?1006h"));
}

void tui_ansi_mouse_sgr_disable(char *buf, size_t *len)
{
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, snprintf(buf, ANSI_BUF_SIZE, ESC "?1006l"));
}

/* Clipboard (OSC 52) */

/* Base64 encoding table */
static const char b64_encode_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* Base64 decoding table (-1 = invalid, -2 = whitespace) */
static const signed char b64_decode_table[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-1,-1,-2,-1,-1,  /* 0-15 */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 16-31 */
    -2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 32-47: space, +, / */
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 48-63: 0-9 */
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 64-79: A-O */
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 80-95: P-Z */
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 96-111: a-o */
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 112-127: p-z */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 128-143 */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 144-159 */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 160-175 */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 176-191 */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 192-207 */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 208-223 */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 224-239 */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* 240-255 */
};

size_t tui_base64_encode_len(size_t src_len)
{
    /* Base64 produces 4 bytes for every 3 input bytes, rounded up */
    return ((src_len + 2) / 3) * 4;
}

int tui_base64_encode(const char *src, size_t src_len, char *dst, size_t dst_size)
{
    size_t needed = tui_base64_encode_len(src_len);
    if (dst_size < needed + 1) return -1;  /* +1 for null terminator */

    const unsigned char *in = (const unsigned char *)src;
    char *out = dst;

    while (src_len >= 3) {
        *out++ = b64_encode_table[in[0] >> 2];
        *out++ = b64_encode_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *out++ = b64_encode_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *out++ = b64_encode_table[in[2] & 0x3f];
        in += 3;
        src_len -= 3;
    }

    if (src_len > 0) {
        *out++ = b64_encode_table[in[0] >> 2];
        if (src_len == 1) {
            *out++ = b64_encode_table[(in[0] & 0x03) << 4];
            *out++ = '=';
        } else {
            *out++ = b64_encode_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
            *out++ = b64_encode_table[(in[1] & 0x0f) << 2];
        }
        *out++ = '=';
    }

    *out = '\0';
    return (int)(out - dst);
}

size_t tui_base64_decode_len(size_t src_len)
{
    /* Decoded length is at most 3/4 of encoded length */
    return (src_len * 3) / 4;
}

int tui_base64_decode(const char *src, size_t src_len, char *dst, size_t dst_size)
{
    /* Validate input parameters */
    if (!src || !dst || dst_size == 0) {
        return 0;
    }

    const unsigned char *in = (const unsigned char *)src;
    unsigned char *out = (unsigned char *)dst;
    unsigned char *out_end = out + dst_size;
    int val = 0, valb = -8;

    for (size_t i = 0; i < src_len; i++) {
        signed char c = b64_decode_table[in[i]];
        if (c == -2) continue;  /* Skip whitespace */
        if (c == -1) break;     /* Invalid or padding */

        val = (val << 6) + c;
        valb += 6;
        if (valb >= 0) {
            /* Check bounds BEFORE writing to prevent buffer overflow */
            if (out >= out_end) {
                break;  /* Buffer full, stop decoding */
            }
            *out++ = (unsigned char)((val >> valb) & 0xFF);
            valb -= 8;
        }
    }

    return (int)(out - (unsigned char *)dst);
}

static char clipboard_target_char(tui_clipboard_target target)
{
    switch (target) {
        case TUI_CLIPBOARD_PRIMARY: return 'p';
        case TUI_CLIPBOARD_SECONDARY: return 's';
        default: return 'c';
    }
}

int tui_ansi_clipboard_write(char *buf, size_t buf_size, const char *text, size_t text_len,
                              tui_clipboard_target target)
{
    /* Validate inputs */
    if (!buf || buf_size == 0 || !text || text_len == 0) return -1;

    /* Limit text size to prevent integer overflow in base64 calculation.
     * Base64 produces 4 bytes for every 3 input bytes, so we need text_len * 4/3.
     * Limit to ~1GB to be safe (SIZE_MAX / 4 prevents overflow in multiplication). */
    if (text_len > SIZE_MAX / 4) return -1;

    /* Calculate required size: \033]52;X;<base64>\007 */
    size_t b64_len = tui_base64_encode_len(text_len);
    size_t prefix_len = 6;  /* \033]52;X; */
    size_t suffix_len = 1;  /* \007 */
    size_t total = prefix_len + b64_len + suffix_len + 1;

    /* Check for overflow in total calculation */
    if (total < b64_len) return -1;  /* Overflow occurred */
    if (buf_size < total) return -1;

    /* Build the sequence */
    char tgt = clipboard_target_char(target);
    int prefix = snprintf(buf, buf_size, "\x1b]52;%c;", tgt);
    if (prefix < 0 || (size_t)prefix >= buf_size) return -1;

    int b64_result = tui_base64_encode(text, text_len, buf + prefix, buf_size - prefix);
    if (b64_result < 0) return -1;

    size_t pos = prefix + b64_result;
    if (pos + 2 > buf_size) return -1;

    buf[pos++] = '\x07';  /* BEL terminator */
    buf[pos] = '\0';

    return (int)pos;
}

void tui_ansi_clipboard_request(char *buf, size_t *len, tui_clipboard_target target)
{
    char tgt = clipboard_target_char(target);
    int result = snprintf(buf, ANSI_BUF_SIZE, "\x1b]52;%c;?\x07", tgt);
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, result);
}

void tui_ansi_clipboard_clear(char *buf, size_t *len, tui_clipboard_target target)
{
    char tgt = clipboard_target_char(target);
    int result = snprintf(buf, ANSI_BUF_SIZE, "\x1b]52;%c;!\x07", tgt);
    safe_snprintf_len(buf, ANSI_BUF_SIZE, len, result);
}
