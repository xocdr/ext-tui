/*
  +----------------------------------------------------------------------+
  | ext-tui: Input parsing                                              |
  +----------------------------------------------------------------------+
  | Parses keyboard, mouse, and paste events from terminal input.       |
  |                                                                      |
  | Thread Safety:                                                       |
  | - Parsing functions (tui_input_parse*) are THREAD-SAFE              |
  | - History functions are OBJECT-LOCAL (safe with separate instances) |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_INPUT_H
#define TUI_INPUT_H

#include <stdbool.h>
#include <stddef.h>

/* ----------------------------------------------------------------
 * Mouse Events
 * ---------------------------------------------------------------- */

typedef enum {
    TUI_MOUSE_NONE = 0,
    TUI_MOUSE_LEFT = 1,
    TUI_MOUSE_MIDDLE = 2,
    TUI_MOUSE_RIGHT = 3,
    TUI_MOUSE_SCROLL_UP = 4,
    TUI_MOUSE_SCROLL_DOWN = 5,
    TUI_MOUSE_SCROLL_LEFT = 6,
    TUI_MOUSE_SCROLL_RIGHT = 7
} tui_mouse_button;

typedef enum {
    TUI_MOUSE_PRESS,
    TUI_MOUSE_RELEASE,
    TUI_MOUSE_MOVE,
    TUI_MOUSE_DRAG
} tui_mouse_action;

typedef struct {
    int x;                      /* Column (0-based) */
    int y;                      /* Row (0-based) */
    tui_mouse_button button;
    tui_mouse_action action;
    bool ctrl;
    bool meta;
    bool shift;
} tui_mouse_event;

/**
 * Check if buffer contains a mouse event sequence.
 * Returns number of bytes consumed if mouse event parsed, 0 otherwise.
 */
int tui_input_parse_mouse(const char *buf, int len, tui_mouse_event *event);

/* ----------------------------------------------------------------
 * Keyboard Events
 * ---------------------------------------------------------------- */

typedef struct {
    char key[8];       /* UTF-8 character or key name */
    bool ctrl;
    bool meta;
    bool shift;
    bool upArrow;
    bool downArrow;
    bool leftArrow;
    bool rightArrow;
    bool enter;
    bool escape;
    bool backspace;
    bool delete;
    bool tab;
    bool home;
    bool end;
    bool pageUp;
    bool pageDown;
    int  functionKey;  /* 0 = not a function key, 1-12 = F1-F12 */
} tui_key_event;

/* Parse raw input bytes into key event */
int tui_input_parse(const char *buf, int len, tui_key_event *event);

/* Bracketed paste detection */
#define TUI_PASTE_START_SEQ "\x1b[200~"
#define TUI_PASTE_START_LEN 6
#define TUI_PASTE_END_SEQ "\x1b[201~"
#define TUI_PASTE_END_LEN 6

/**
 * Check if buffer starts with paste start sequence.
 * Returns 1 if paste start detected, 0 otherwise.
 */
int tui_input_is_paste_start(const char *buf, int len);

/**
 * Check if buffer contains paste end sequence.
 * Returns position of paste end sequence if found, -1 otherwise.
 */
int tui_input_find_paste_end(const char *buf, int len);

/* Input history */
typedef struct {
    char **entries;
    int count;
    int capacity;
    int position;       /* Current navigation position (-1 = at end) */
    int max_entries;
    char *temp_input;   /* Saved input when navigating history */
} tui_input_history;

tui_input_history* tui_history_create(int max_entries);
void tui_history_destroy(tui_input_history *history);
int tui_history_add(tui_input_history *history, const char *entry);
const char* tui_history_prev(tui_input_history *history);
const char* tui_history_next(tui_input_history *history);
void tui_history_reset_position(tui_input_history *history);
void tui_history_save_temp(tui_input_history *history, const char *input);
const char* tui_history_get_temp(tui_input_history *history);

#endif /* TUI_INPUT_H */
