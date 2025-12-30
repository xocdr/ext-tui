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

#include "capabilities.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

/* Global cached capabilities */
static tui_capabilities global_caps;
static int global_caps_initialized = 0;

/* Helper to safely copy string */
static void safe_strcpy(char *dst, size_t dst_size, const char *src)
{
    if (!src) {
        dst[0] = '\0';
        return;
    }
    size_t len = strlen(src);
    if (len >= dst_size) {
        len = dst_size - 1;
    }
    memcpy(dst, src, len);
    dst[len] = '\0';
}

/* Check if string starts with prefix (case-insensitive) */
static int starts_with_ci(const char *str, const char *prefix)
{
    if (!str || !prefix) return 0;
    return strncasecmp(str, prefix, strlen(prefix)) == 0;
}

/* Check if string contains substring (case-insensitive) */
static int contains_ci(const char *str, const char *substr)
{
    if (!str || !substr) return 0;
    return strcasestr(str, substr) != NULL;
}

/* Detect terminal type from environment */
static tui_terminal_type detect_terminal_type(const char *term, const char *term_program)
{
    /* Check TERM_PROGRAM first (more specific) */
    if (term_program) {
        if (strcasecmp(term_program, "iTerm.app") == 0) return TUI_TERM_ITERM2;
        if (strcasecmp(term_program, "Apple_Terminal") == 0) return TUI_TERM_APPLE_TERMINAL;
        if (strcasecmp(term_program, "WezTerm") == 0) return TUI_TERM_WEZTERM;
        if (strcasecmp(term_program, "Hyper") == 0) return TUI_TERM_XTERM; /* Hyper uses xterm */
    }

    /* Check specific environment variables */
    if (getenv("KITTY_WINDOW_ID")) return TUI_TERM_KITTY;
    if (getenv("ALACRITTY_LOG")) return TUI_TERM_ALACRITTY;
    if (getenv("ALACRITTY_SOCKET")) return TUI_TERM_ALACRITTY;
    if (getenv("WT_SESSION")) return TUI_TERM_WINDOWS_TERMINAL;
    if (getenv("KONSOLE_VERSION")) return TUI_TERM_KONSOLE;
    if (getenv("WEZTERM_PANE")) return TUI_TERM_WEZTERM;

    /* Check VTE version (GNOME Terminal, Tilix, etc.) */
    if (getenv("VTE_VERSION")) return TUI_TERM_VTE;

    /* Check TERM variable */
    if (term) {
        if (starts_with_ci(term, "screen")) return TUI_TERM_SCREEN;
        if (starts_with_ci(term, "tmux")) return TUI_TERM_TMUX;
        if (contains_ci(term, "kitty")) return TUI_TERM_KITTY;
        if (contains_ci(term, "alacritty")) return TUI_TERM_ALACRITTY;
        if (starts_with_ci(term, "xterm")) return TUI_TERM_XTERM;
        if (starts_with_ci(term, "rxvt")) return TUI_TERM_XTERM;
        if (starts_with_ci(term, "foot")) return TUI_TERM_FOOT;
    }

    return TUI_TERM_UNKNOWN;
}

/* Detect color depth from environment */
static int detect_color_depth(const char *term, const char *colorterm)
{
    /* COLORTERM=truecolor or 24bit indicates 24-bit color */
    if (colorterm) {
        if (strcasecmp(colorterm, "truecolor") == 0 ||
            strcasecmp(colorterm, "24bit") == 0) {
            return 16777216;
        }
    }

    /* Check TERM for color hints */
    if (term) {
        if (contains_ci(term, "256color") || contains_ci(term, "256-color")) {
            return 256;
        }
        if (contains_ci(term, "truecolor") || contains_ci(term, "24bit") ||
            contains_ci(term, "direct")) {
            return 16777216;
        }
    }

    /* Most modern terminals support at least 256 colors */
    if (term && (starts_with_ci(term, "xterm") || starts_with_ci(term, "screen") ||
                 starts_with_ci(term, "tmux") || starts_with_ci(term, "rxvt"))) {
        return 256;
    }

    /* Default to 8 colors for unknown terminals */
    return 8;
}

/* Detect capabilities based on terminal type */
static unsigned int detect_capabilities(tui_terminal_type type, int color_depth,
                                         const char *term)
{
    unsigned int caps = 0;

    /* Color capabilities */
    if (color_depth >= 16777216) {
        caps |= TUI_CAP_TRUE_COLOR | TUI_CAP_256_COLOR;
    } else if (color_depth >= 256) {
        caps |= TUI_CAP_256_COLOR;
    }

    /* Most modern terminals support these basics */
    caps |= TUI_CAP_CURSOR_SHAPE;
    caps |= TUI_CAP_TITLE;
    caps |= TUI_CAP_ALTERNATE_SCREEN;

    /* Check for UTF-8 locale */
    const char *lang = getenv("LANG");
    const char *lc_all = getenv("LC_ALL");
    const char *lc_ctype = getenv("LC_CTYPE");
    if ((lang && contains_ci(lang, "utf")) ||
        (lc_all && contains_ci(lc_all, "utf")) ||
        (lc_ctype && contains_ci(lc_ctype, "utf"))) {
        caps |= TUI_CAP_UNICODE;
    }

    /* Terminal-specific capabilities */
    switch (type) {
        case TUI_TERM_KITTY:
            caps |= TUI_CAP_TRUE_COLOR | TUI_CAP_256_COLOR;
            caps |= TUI_CAP_MOUSE | TUI_CAP_MOUSE_SGR;
            caps |= TUI_CAP_BRACKETED_PASTE;
            caps |= TUI_CAP_CLIPBOARD_OSC52;
            caps |= TUI_CAP_HYPERLINKS_OSC8;
            caps |= TUI_CAP_SYNC_OUTPUT;
            caps |= TUI_CAP_KITTY_KEYBOARD;
            caps |= TUI_CAP_KITTY_GRAPHICS;
            caps |= TUI_CAP_FOCUS_EVENTS;
            caps |= TUI_CAP_UNICODE;
            break;

        case TUI_TERM_ITERM2:
            caps |= TUI_CAP_TRUE_COLOR | TUI_CAP_256_COLOR;
            caps |= TUI_CAP_MOUSE | TUI_CAP_MOUSE_SGR;
            caps |= TUI_CAP_BRACKETED_PASTE;
            caps |= TUI_CAP_CLIPBOARD_OSC52;
            caps |= TUI_CAP_HYPERLINKS_OSC8;
            caps |= TUI_CAP_SYNC_OUTPUT;
            caps |= TUI_CAP_FOCUS_EVENTS;
            caps |= TUI_CAP_SIXEL;
            break;

        case TUI_TERM_WEZTERM:
            caps |= TUI_CAP_TRUE_COLOR | TUI_CAP_256_COLOR;
            caps |= TUI_CAP_MOUSE | TUI_CAP_MOUSE_SGR;
            caps |= TUI_CAP_BRACKETED_PASTE;
            caps |= TUI_CAP_CLIPBOARD_OSC52;
            caps |= TUI_CAP_HYPERLINKS_OSC8;
            caps |= TUI_CAP_SYNC_OUTPUT;
            caps |= TUI_CAP_KITTY_KEYBOARD;
            caps |= TUI_CAP_KITTY_GRAPHICS;
            caps |= TUI_CAP_SIXEL;
            caps |= TUI_CAP_FOCUS_EVENTS;
            break;

        case TUI_TERM_ALACRITTY:
            caps |= TUI_CAP_TRUE_COLOR | TUI_CAP_256_COLOR;
            caps |= TUI_CAP_MOUSE | TUI_CAP_MOUSE_SGR;
            caps |= TUI_CAP_BRACKETED_PASTE;
            caps |= TUI_CAP_HYPERLINKS_OSC8;
            caps |= TUI_CAP_SYNC_OUTPUT;
            caps |= TUI_CAP_FOCUS_EVENTS;
            break;

        case TUI_TERM_VTE:
            caps |= TUI_CAP_TRUE_COLOR | TUI_CAP_256_COLOR;
            caps |= TUI_CAP_MOUSE | TUI_CAP_MOUSE_SGR;
            caps |= TUI_CAP_BRACKETED_PASTE;
            caps |= TUI_CAP_HYPERLINKS_OSC8;
            caps |= TUI_CAP_SYNC_OUTPUT;
            break;

        case TUI_TERM_WINDOWS_TERMINAL:
            caps |= TUI_CAP_TRUE_COLOR | TUI_CAP_256_COLOR;
            caps |= TUI_CAP_MOUSE | TUI_CAP_MOUSE_SGR;
            caps |= TUI_CAP_BRACKETED_PASTE;
            caps |= TUI_CAP_HYPERLINKS_OSC8;
            caps |= TUI_CAP_FOCUS_EVENTS;
            break;

        case TUI_TERM_KONSOLE:
            caps |= TUI_CAP_TRUE_COLOR | TUI_CAP_256_COLOR;
            caps |= TUI_CAP_MOUSE | TUI_CAP_MOUSE_SGR;
            caps |= TUI_CAP_BRACKETED_PASTE;
            caps |= TUI_CAP_CLIPBOARD_OSC52;
            caps |= TUI_CAP_HYPERLINKS_OSC8;
            break;

        case TUI_TERM_FOOT:
            caps |= TUI_CAP_TRUE_COLOR | TUI_CAP_256_COLOR;
            caps |= TUI_CAP_MOUSE | TUI_CAP_MOUSE_SGR;
            caps |= TUI_CAP_BRACKETED_PASTE;
            caps |= TUI_CAP_CLIPBOARD_OSC52;
            caps |= TUI_CAP_HYPERLINKS_OSC8;
            caps |= TUI_CAP_SYNC_OUTPUT;
            caps |= TUI_CAP_KITTY_KEYBOARD;
            caps |= TUI_CAP_SIXEL;
            break;

        case TUI_TERM_XTERM:
            caps |= TUI_CAP_MOUSE | TUI_CAP_MOUSE_SGR;
            caps |= TUI_CAP_BRACKETED_PASTE;
            caps |= TUI_CAP_CLIPBOARD_OSC52;
            caps |= TUI_CAP_FOCUS_EVENTS;
            break;

        case TUI_TERM_SCREEN:
        case TUI_TERM_TMUX:
            caps |= TUI_CAP_MOUSE;
            caps |= TUI_CAP_BRACKETED_PASTE;
            /* Pass through terminal may have more capabilities */
            break;

        case TUI_TERM_APPLE_TERMINAL:
            caps |= TUI_CAP_256_COLOR;
            caps |= TUI_CAP_MOUSE;
            caps |= TUI_CAP_BRACKETED_PASTE;
            /* Apple Terminal has limited feature support */
            break;

        default:
            /* Conservative defaults for unknown terminals */
            if (isatty(STDOUT_FILENO)) {
                caps |= TUI_CAP_MOUSE;
            }
            break;
    }

    return caps;
}

int tui_capabilities_detect(tui_capabilities *caps)
{
    if (!caps) return -1;

    memset(caps, 0, sizeof(*caps));

    /* Get environment variables */
    const char *term = getenv("TERM");
    const char *term_program = getenv("TERM_PROGRAM");
    const char *colorterm = getenv("COLORTERM");

    /* Detect terminal type */
    caps->terminal = detect_terminal_type(term, term_program);

    /* Set terminal name */
    if (term_program && term_program[0]) {
        safe_strcpy(caps->name, sizeof(caps->name), term_program);
    } else if (term && term[0]) {
        safe_strcpy(caps->name, sizeof(caps->name), term);
    } else {
        safe_strcpy(caps->name, sizeof(caps->name), "unknown");
    }

    /* Try to get version */
    const char *vte_version = getenv("VTE_VERSION");
    const char *konsole_version = getenv("KONSOLE_VERSION");
    if (vte_version) {
        safe_strcpy(caps->version, sizeof(caps->version), vte_version);
    } else if (konsole_version) {
        safe_strcpy(caps->version, sizeof(caps->version), konsole_version);
    }

    /* Detect color depth */
    caps->color_depth = detect_color_depth(term, colorterm);

    /* Detect capabilities */
    caps->capabilities = detect_capabilities(caps->terminal, caps->color_depth, term);

    return 0;
}

int tui_has_capability(const tui_capabilities *caps, unsigned int capability)
{
    if (!caps) {
        caps = tui_get_capabilities();
    }
    return (caps->capabilities & capability) != 0;
}

const tui_capabilities *tui_get_capabilities(void)
{
    if (!global_caps_initialized) {
        tui_capabilities_detect(&global_caps);
        global_caps_initialized = 1;
    }
    return &global_caps;
}

const char *tui_terminal_type_name(tui_terminal_type type)
{
    switch (type) {
        case TUI_TERM_XTERM:            return "xterm";
        case TUI_TERM_VTE:              return "vte";
        case TUI_TERM_KITTY:            return "kitty";
        case TUI_TERM_ALACRITTY:        return "alacritty";
        case TUI_TERM_ITERM2:           return "iterm2";
        case TUI_TERM_WINDOWS_TERMINAL: return "windows_terminal";
        case TUI_TERM_APPLE_TERMINAL:   return "apple_terminal";
        case TUI_TERM_SCREEN:           return "screen";
        case TUI_TERM_TMUX:             return "tmux";
        case TUI_TERM_MINTTY:           return "mintty";
        case TUI_TERM_KONSOLE:          return "konsole";
        case TUI_TERM_WEZTERM:          return "wezterm";
        case TUI_TERM_FOOT:             return "foot";
        default:                        return "unknown";
    }
}
