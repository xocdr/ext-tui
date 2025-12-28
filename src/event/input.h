/*
  +----------------------------------------------------------------------+
  | ext-tui: Input parsing                                              |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_INPUT_H
#define TUI_INPUT_H

#include <stdbool.h>

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

#endif /* TUI_INPUT_H */
