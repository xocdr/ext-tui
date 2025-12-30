/*
  +----------------------------------------------------------------------+
  | ext-tui: Input parsing                                              |
  +----------------------------------------------------------------------+
*/

#include "input.h"
#include "../text/measure.h"  /* For tui_utf8_decode_n() */
#include <string.h>
#include <stdlib.h>

/* Maximum mouse coordinate value (prevents overflow).
   Terminal coordinates rarely exceed 10000 even on very large displays. */
#define MAX_MOUSE_COORD_VALUE 65535

/* ----------------------------------------------------------------
 * Mouse Event Parsing
 * ---------------------------------------------------------------- */

/**
 * Parse SGR mouse sequence: ESC [ < button ; x ; y M/m
 * Where M = press, m = release
 * Returns bytes consumed if valid, 0 otherwise.
 */
int tui_input_parse_mouse(const char *buf, int len, tui_mouse_event *event)
{
    if (!buf || len < 9 || !event) return 0;

    memset(event, 0, sizeof(tui_mouse_event));

    /* Check for SGR mouse prefix: ESC [ < */
    if (buf[0] != '\x1b' || buf[1] != '[' || buf[2] != '<') {
        return 0;
    }

    /* Parse: <button>;<x>;<y>M or <button>;<x>;<y>m */
    int button = 0, x = 0, y = 0;
    int i = 3;
    char terminator = 0;

    /* Parse button code (with overflow protection) */
    while (i < len && buf[i] >= '0' && buf[i] <= '9') {
        if (button > MAX_MOUSE_COORD_VALUE / 10) {
            return 0;  /* Would overflow, invalid sequence */
        }
        button = button * 10 + (buf[i] - '0');
        i++;
    }
    if (i >= len || buf[i] != ';') return 0;
    i++;

    /* Parse x coordinate (with overflow protection) */
    while (i < len && buf[i] >= '0' && buf[i] <= '9') {
        if (x > MAX_MOUSE_COORD_VALUE / 10) {
            return 0;  /* Would overflow, invalid sequence */
        }
        x = x * 10 + (buf[i] - '0');
        i++;
    }
    if (i >= len || buf[i] != ';') return 0;
    i++;

    /* Parse y coordinate (with overflow protection) */
    while (i < len && buf[i] >= '0' && buf[i] <= '9') {
        if (y > MAX_MOUSE_COORD_VALUE / 10) {
            return 0;  /* Would overflow, invalid sequence */
        }
        y = y * 10 + (buf[i] - '0');
        i++;
    }
    if (i >= len) return 0;

    terminator = buf[i];
    if (terminator != 'M' && terminator != 'm') return 0;
    i++;

    /* Convert to 0-based coordinates */
    event->x = x - 1;
    event->y = y - 1;

    /* Decode button and modifiers from button code */
    /* Button code format: (modifiers << 2) | button_code */
    /* Modifiers: 4=Shift, 8=Meta, 16=Ctrl */
    /* Button: 0=left, 1=middle, 2=right, 32=motion, 64=scroll */
    event->shift = (button & 4) != 0;
    event->meta = (button & 8) != 0;
    event->ctrl = (button & 16) != 0;

    int btn = button & 3;
    int motion = (button & 32) != 0;
    int scroll = (button & 64) != 0;

    if (scroll) {
        /* Scroll events */
        event->action = TUI_MOUSE_PRESS;  /* Scroll is always a "press" event */
        if (btn == 0) {
            event->button = TUI_MOUSE_SCROLL_UP;
        } else if (btn == 1) {
            event->button = TUI_MOUSE_SCROLL_DOWN;
        } else if (btn == 2) {
            event->button = TUI_MOUSE_SCROLL_LEFT;
        } else {
            event->button = TUI_MOUSE_SCROLL_RIGHT;
        }
    } else if (motion && btn == 3) {
        /* Motion without button (btn==3 means no button) */
        event->action = TUI_MOUSE_MOVE;
        event->button = TUI_MOUSE_NONE;
    } else if (motion) {
        /* Drag with button held */
        event->action = TUI_MOUSE_DRAG;
        event->button = (btn == 0) ? TUI_MOUSE_LEFT :
                        (btn == 1) ? TUI_MOUSE_MIDDLE :
                        (btn == 2) ? TUI_MOUSE_RIGHT : TUI_MOUSE_NONE;
    } else {
        /* Click/release */
        event->action = (terminator == 'M') ? TUI_MOUSE_PRESS : TUI_MOUSE_RELEASE;
        event->button = (btn == 0) ? TUI_MOUSE_LEFT :
                        (btn == 1) ? TUI_MOUSE_MIDDLE :
                        (btn == 2) ? TUI_MOUSE_RIGHT : TUI_MOUSE_NONE;
    }

    return i;  /* Bytes consumed */
}

/* ----------------------------------------------------------------
 * Keyboard Event Parsing
 * ---------------------------------------------------------------- */

/* Maximum ANSI numeric parameter value to parse (prevents overflow).
   Valid key codes are all <= 24, so any higher value is invalid. */
#define MAX_KEY_CODE_VALUE 9999

int tui_input_parse(const char *buf, int len, tui_key_event *event)
{
    memset(event, 0, sizeof(tui_key_event));

    if (len == 0) {
        return -1;
    }

    /* Single character */
    if (len == 1) {
        unsigned char c = buf[0];

        /* Special keys first (before Ctrl check, since Enter=13, Tab=9 overlap) */
        switch (c) {
            case 27:  /* Escape */
                event->escape = true;
                return 0;
            case 13:  /* Enter (CR) */
                event->enter = true;
                return 0;
            case 10:  /* Enter (LF) - some terminals send this */
                event->enter = true;
                return 0;
            case 127: /* Backspace */
                event->backspace = true;
                return 0;
            case 8:   /* Backspace (some terminals) */
                event->backspace = true;
                return 0;
            case 9:   /* Tab */
                event->tab = true;
                return 0;
        }

        /* Ctrl+letter (ASCII 1-26, excluding Tab=9, Enter=10,13) */
        if (c >= 1 && c <= 26 && c != 9 && c != 10 && c != 13) {
            event->ctrl = true;
            event->key[0] = 'a' + c - 1;
            event->key[1] = '\0';
            return 0;
        }

        /* Regular character */
        event->key[0] = c;
        event->key[1] = '\0';
        return 0;
    }

    /* Escape sequences */
    if (buf[0] == 27 && len >= 2) {
        /* Alt+key */
        if (len == 2) {
            event->meta = true;
            event->key[0] = buf[1];
            event->key[1] = '\0';
            return 0;
        }

        /* Arrow keys and special keys: ESC [ ... */
        if (buf[1] == '[' && len >= 3) {
            /* Check for xterm extended sequences: ESC [ 1 ; <mod> <key>
             * Modifier: 2=Shift, 3=Alt, 4=Shift+Alt, 5=Ctrl, etc. */
            if (buf[2] == '1' && len >= 6 && buf[3] == ';') {
                int mod = buf[4] - '0';
                char key = buf[5];

                if (mod == 2) event->shift = true;
                else if (mod == 3) event->meta = true;
                else if (mod == 4) { event->shift = true; event->meta = true; }
                else if (mod == 5) event->ctrl = true;
                else if (mod == 6) { event->shift = true; event->ctrl = true; }
                else if (mod == 7) { event->meta = true; event->ctrl = true; }
                else if (mod == 8) { event->shift = true; event->meta = true; event->ctrl = true; }

                switch (key) {
                    case 'A': event->upArrow = true; return 0;
                    case 'B': event->downArrow = true; return 0;
                    case 'C': event->rightArrow = true; return 0;
                    case 'D': event->leftArrow = true; return 0;
                    case 'H': event->home = true; return 0;
                    case 'F': event->end = true; return 0;
                }
            }

            /* Shift+Tab: ESC [ Z */
            if (buf[2] == 'Z') {
                event->tab = true;
                event->shift = true;
                return 0;
            }

            switch (buf[2]) {
                case 'A': event->upArrow = true; return 0;
                case 'B': event->downArrow = true; return 0;
                case 'C': event->rightArrow = true; return 0;
                case 'D': event->leftArrow = true; return 0;
                case 'H': event->home = true; return 0;
                case 'F': event->end = true; return 0;
                case '3':
                    if (len >= 4 && buf[3] == '~') {
                        event->delete = true;
                        return 0;
                    }
                    break;
                case '5':
                    if (len >= 4 && buf[3] == '~') {
                        event->pageUp = true;
                        return 0;
                    }
                    break;
                case '6':
                    if (len >= 4 && buf[3] == '~') {
                        event->pageDown = true;
                        return 0;
                    }
                    break;
            }
        }

        /* ESC O sequences (alternate arrow keys and F1-F4) */
        if (buf[1] == 'O' && len >= 3) {
            switch (buf[2]) {
                case 'A': event->upArrow = true; return 0;
                case 'B': event->downArrow = true; return 0;
                case 'C': event->rightArrow = true; return 0;
                case 'D': event->leftArrow = true; return 0;
                case 'H': event->home = true; return 0;
                case 'F': event->end = true; return 0;
                /* F1-F4 */
                case 'P': event->functionKey = 1; return 0;
                case 'Q': event->functionKey = 2; return 0;
                case 'R': event->functionKey = 3; return 0;
                case 'S': event->functionKey = 4; return 0;
            }
        }

        /* F5-F12: ESC [ <num> ~ */
        if (buf[1] == '[' && len >= 4 && buf[len-1] == '~') {
            /* Parse the number between [ and ~ (with overflow protection) */
            int num = 0;
            for (int i = 2; i < len - 1; i++) {
                if (buf[i] >= '0' && buf[i] <= '9') {
                    if (num > MAX_KEY_CODE_VALUE) break;
                    num = num * 10 + (buf[i] - '0');
                }
            }
            switch (num) {
                case 15: event->functionKey = 5; return 0;
                case 17: event->functionKey = 6; return 0;
                case 18: event->functionKey = 7; return 0;
                case 19: event->functionKey = 8; return 0;
                case 20: event->functionKey = 9; return 0;
                case 21: event->functionKey = 10; return 0;
                case 23: event->functionKey = 11; return 0;
                case 24: event->functionKey = 12; return 0;
            }
        }
    }

    /* UTF-8 multi-byte character - use shared UTF-8 decoder from measure.h */
    if ((unsigned char)buf[0] >= 0x80) {
        uint32_t codepoint;
        int bytes = tui_utf8_decode_n(buf, len, &codepoint);

        /* If valid UTF-8 and fits in key buffer (leave room for null terminator) */
        if (bytes > 0 && bytes <= (int)sizeof(event->key) - 1) {
            memcpy(event->key, buf, bytes);
            event->key[bytes] = '\0';
            return 0;
        }
        /* Invalid UTF-8 or too long: fall through to raw copy */
    }

    /* Unknown sequence, copy raw with bounds check */
    /* key buffer is 8 bytes, leave room for null terminator */
    int max_copy = (int)sizeof(event->key) - 1;
    int copy = len < max_copy ? len : max_copy;
    memcpy(event->key, buf, copy);
    event->key[copy] = '\0';
    return 0;
}

/* Bracketed paste detection */

int tui_input_is_paste_start(const char *buf, int len)
{
    if (len < TUI_PASTE_START_LEN) return 0;
    return memcmp(buf, TUI_PASTE_START_SEQ, TUI_PASTE_START_LEN) == 0;
}

int tui_input_find_paste_end(const char *buf, int len)
{
    if (len < TUI_PASTE_END_LEN) return -1;

    /* Search for paste end sequence */
    for (int i = 0; i <= len - TUI_PASTE_END_LEN; i++) {
        if (memcmp(buf + i, TUI_PASTE_END_SEQ, TUI_PASTE_END_LEN) == 0) {
            return i;
        }
    }
    return -1;
}

/* Input history */

#include <stdlib.h>

#define DEFAULT_HISTORY_CAPACITY 64

tui_input_history* tui_history_create(int max_entries)
{
    if (max_entries <= 0) max_entries = 1000;

    tui_input_history *history = calloc(1, sizeof(tui_input_history));
    if (!history) return NULL;

    int initial_capacity = max_entries < DEFAULT_HISTORY_CAPACITY
                         ? max_entries : DEFAULT_HISTORY_CAPACITY;

    history->entries = calloc(initial_capacity, sizeof(char*));
    if (!history->entries) {
        free(history);
        return NULL;
    }

    history->capacity = initial_capacity;
    history->max_entries = max_entries;
    history->position = -1;  /* -1 = not navigating */
    history->count = 0;
    history->temp_input = NULL;

    return history;
}

void tui_history_destroy(tui_input_history *history)
{
    if (!history) return;

    for (int i = 0; i < history->count; i++) {
        free(history->entries[i]);
    }
    free(history->entries);
    free(history->temp_input);
    free(history);
}

int tui_history_add(tui_input_history *history, const char *entry)
{
    if (!history || !entry || !entry[0]) return -1;

    /* Don't add duplicate of last entry */
    if (history->count > 0 &&
        strcmp(history->entries[history->count - 1], entry) == 0) {
        tui_history_reset_position(history);
        return 0;
    }

    /* Grow if needed */
    if (history->count >= history->capacity) {
        if (history->capacity >= history->max_entries) {
            /* At max capacity - remove oldest entry */
            free(history->entries[0]);
            memmove(history->entries, history->entries + 1,
                   (history->count - 1) * sizeof(char*));
            history->count--;
        } else {
            /* Grow capacity */
            int new_capacity = history->capacity * 2;
            if (new_capacity > history->max_entries) {
                new_capacity = history->max_entries;
            }
            char **new_entries = realloc(history->entries,
                                         new_capacity * sizeof(char*));
            if (!new_entries) return -1;
            history->entries = new_entries;
            history->capacity = new_capacity;
        }
    }

    /* Add new entry */
    history->entries[history->count] = strdup(entry);
    if (!history->entries[history->count]) return -1;
    history->count++;

    tui_history_reset_position(history);
    return 0;
}

const char* tui_history_prev(tui_input_history *history)
{
    if (!history || history->count == 0) return NULL;

    if (history->position < 0) {
        /* Starting navigation from end */
        history->position = history->count - 1;
    } else if (history->position > 0) {
        history->position--;
    } else {
        /* Already at oldest */
        return history->entries[0];
    }

    return history->entries[history->position];
}

const char* tui_history_next(tui_input_history *history)
{
    if (!history || history->position < 0) return NULL;

    if (history->position < history->count - 1) {
        history->position++;
        return history->entries[history->position];
    } else {
        /* Back to current input */
        history->position = -1;
        return history->temp_input;
    }
}

void tui_history_reset_position(tui_input_history *history)
{
    if (history) {
        history->position = -1;
        free(history->temp_input);
        history->temp_input = NULL;
    }
}

void tui_history_save_temp(tui_input_history *history, const char *input)
{
    if (!history) return;
    free(history->temp_input);
    history->temp_input = input ? strdup(input) : NULL;
}

const char* tui_history_get_temp(tui_input_history *history)
{
    return history ? history->temp_input : NULL;
}
