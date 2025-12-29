/*
  +----------------------------------------------------------------------+
  | ext-tui: Input parsing                                              |
  +----------------------------------------------------------------------+
*/

#include "input.h"
#include "../text/measure.h"  /* For tui_utf8_decode_n() */
#include <string.h>

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
                    if (num > 9999) break;  /* Prevent overflow - no valid key code is > 24 */
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
