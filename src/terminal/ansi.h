/*
  +----------------------------------------------------------------------+
  | ext-tui: ANSI escape code generation                                |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_ANSI_H
#define TUI_ANSI_H

#include <stddef.h>
#include <stdint.h>

/* Cursor control */
void tui_ansi_cursor_hide(char *buf, size_t *len);
void tui_ansi_cursor_show(char *buf, size_t *len);
void tui_ansi_cursor_move(char *buf, size_t *len, int x, int y);
void tui_ansi_cursor_save(char *buf, size_t *len);
void tui_ansi_cursor_restore(char *buf, size_t *len);

/* Screen control */
void tui_ansi_clear_screen(char *buf, size_t *len);
void tui_ansi_clear_line(char *buf, size_t *len);
void tui_ansi_alternate_screen_enter(char *buf, size_t *len);
void tui_ansi_alternate_screen_exit(char *buf, size_t *len);

/* Colors (256-color and RGB) */
void tui_ansi_fg_256(char *buf, size_t *len, uint8_t color);
void tui_ansi_bg_256(char *buf, size_t *len, uint8_t color);
void tui_ansi_fg_rgb(char *buf, size_t *len, uint8_t r, uint8_t g, uint8_t b);
void tui_ansi_bg_rgb(char *buf, size_t *len, uint8_t r, uint8_t g, uint8_t b);
void tui_ansi_reset(char *buf, size_t *len);

/* Text styles */
void tui_ansi_bold(char *buf, size_t *len);
void tui_ansi_dim(char *buf, size_t *len);
void tui_ansi_italic(char *buf, size_t *len);
void tui_ansi_underline(char *buf, size_t *len);
void tui_ansi_inverse(char *buf, size_t *len);
void tui_ansi_strikethrough(char *buf, size_t *len);

/* Additional cursor and screen operations */
void tui_ansi_erase_line_start(char *buf, size_t *len);
void tui_ansi_erase_line_end(char *buf, size_t *len);
void tui_ansi_scroll_up(char *buf, size_t *len, int lines);
void tui_ansi_scroll_down(char *buf, size_t *len, int lines);
void tui_ansi_cursor_next_line(char *buf, size_t *len, int lines);
void tui_ansi_cursor_prev_line(char *buf, size_t *len, int lines);
void tui_ansi_cursor_column(char *buf, size_t *len, int col);
void tui_ansi_erase_screen_end(char *buf, size_t *len);
void tui_ansi_erase_screen_start(char *buf, size_t *len);

/* Color conversion */
int tui_rgb_to_ansi256(uint8_t r, uint8_t g, uint8_t b);

/* Synchronized output (DEC mode 2026) - eliminates flicker */
void tui_ansi_sync_start(char *buf, size_t *len);
void tui_ansi_sync_end(char *buf, size_t *len);

/* Hyperlinks (OSC 8) */
void tui_ansi_hyperlink_start(char *buf, size_t buf_size, size_t *len, const char *url, const char *id);
void tui_ansi_hyperlink_end(char *buf, size_t *len);

/* Bracketed paste mode */
void tui_ansi_bracketed_paste_enable(char *buf, size_t *len);
void tui_ansi_bracketed_paste_disable(char *buf, size_t *len);

/* Clipboard (OSC 52) */
typedef enum {
    TUI_CLIPBOARD_CLIPBOARD,   /* System clipboard 'c' */
    TUI_CLIPBOARD_PRIMARY,     /* Primary selection 'p' (X11) */
    TUI_CLIPBOARD_SECONDARY    /* Secondary selection 's' (X11) */
} tui_clipboard_target;

/**
 * Write to clipboard using OSC 52.
 * Generates: \033]52;c;<base64-data>\007
 * Caller must provide buffer large enough for base64 encoding.
 * Returns bytes written to buf, or -1 on error.
 */
int tui_ansi_clipboard_write(char *buf, size_t buf_size, const char *text, size_t text_len,
                              tui_clipboard_target target);

/**
 * Request clipboard contents using OSC 52.
 * Generates: \033]52;c;?\007
 */
void tui_ansi_clipboard_request(char *buf, size_t *len, tui_clipboard_target target);

/**
 * Clear clipboard using OSC 52.
 * Generates: \033]52;c;!\007
 */
void tui_ansi_clipboard_clear(char *buf, size_t *len, tui_clipboard_target target);

/* Base64 utilities for clipboard */
size_t tui_base64_encode_len(size_t src_len);
int tui_base64_encode(const char *src, size_t src_len, char *dst, size_t dst_size);
size_t tui_base64_decode_len(size_t src_len);
int tui_base64_decode(const char *src, size_t src_len, char *dst, size_t dst_size);

#endif /* TUI_ANSI_H */
