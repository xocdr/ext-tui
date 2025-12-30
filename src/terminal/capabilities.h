/*
  +----------------------------------------------------------------------+
  | ext-tui: Terminal capability detection                              |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_CAPABILITIES_H
#define TUI_CAPABILITIES_H

#include <stddef.h>

/* Capability flags */
#define TUI_CAP_TRUE_COLOR       (1 << 0)  /* 24-bit RGB color support */
#define TUI_CAP_256_COLOR        (1 << 1)  /* 256 color support */
#define TUI_CAP_MOUSE            (1 << 2)  /* Basic mouse support */
#define TUI_CAP_MOUSE_SGR        (1 << 3)  /* SGR mouse encoding (1006) */
#define TUI_CAP_BRACKETED_PASTE  (1 << 4)  /* Bracketed paste mode */
#define TUI_CAP_CLIPBOARD_OSC52  (1 << 5)  /* OSC 52 clipboard */
#define TUI_CAP_HYPERLINKS_OSC8  (1 << 6)  /* OSC 8 hyperlinks */
#define TUI_CAP_SYNC_OUTPUT      (1 << 7)  /* Synchronized output (2026) */
#define TUI_CAP_UNICODE          (1 << 8)  /* Unicode/UTF-8 support */
#define TUI_CAP_KITTY_KEYBOARD   (1 << 9)  /* Kitty keyboard protocol */
#define TUI_CAP_SIXEL            (1 << 10) /* Sixel graphics */
#define TUI_CAP_KITTY_GRAPHICS   (1 << 11) /* Kitty graphics protocol */
#define TUI_CAP_CURSOR_SHAPE     (1 << 12) /* DECSCUSR cursor shape */
#define TUI_CAP_TITLE            (1 << 13) /* Window title (OSC 2) */
#define TUI_CAP_FOCUS_EVENTS     (1 << 14) /* Focus in/out events */
#define TUI_CAP_ALTERNATE_SCREEN (1 << 15) /* Alternate screen buffer */

/* Terminal identification */
typedef enum {
    TUI_TERM_UNKNOWN = 0,
    TUI_TERM_XTERM,
    TUI_TERM_VTE,           /* GNOME Terminal, Tilix, etc. */
    TUI_TERM_KITTY,
    TUI_TERM_ALACRITTY,
    TUI_TERM_ITERM2,
    TUI_TERM_WINDOWS_TERMINAL,
    TUI_TERM_APPLE_TERMINAL,
    TUI_TERM_SCREEN,
    TUI_TERM_TMUX,
    TUI_TERM_MINTTY,
    TUI_TERM_KONSOLE,
    TUI_TERM_WEZTERM,
    TUI_TERM_FOOT
} tui_terminal_type;

/* Capabilities structure */
typedef struct {
    tui_terminal_type terminal;     /* Detected terminal type */
    unsigned int capabilities;       /* Bitmask of TUI_CAP_* flags */
    int color_depth;                /* 0, 8, 256, or 16777216 (24-bit) */
    char name[64];                  /* Terminal name string */
    char version[32];               /* Version if available */
} tui_capabilities;

/**
 * Detect terminal capabilities.
 * Examines environment variables and terminal characteristics.
 * Returns 0 on success.
 */
int tui_capabilities_detect(tui_capabilities *caps);

/**
 * Check if a specific capability is supported.
 * @param caps Capabilities structure (or NULL to use cached global)
 * @param capability One of the TUI_CAP_* flags
 * @return 1 if supported, 0 if not
 */
int tui_has_capability(const tui_capabilities *caps, unsigned int capability);

/**
 * Get the global capabilities (detected on first call).
 * Thread-safe, cached after first detection.
 */
const tui_capabilities *tui_get_capabilities(void);

/**
 * Get terminal type name as string.
 */
const char *tui_terminal_type_name(tui_terminal_type type);

#endif /* TUI_CAPABILITIES_H */
