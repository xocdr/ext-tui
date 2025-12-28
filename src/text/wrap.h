/*
  +----------------------------------------------------------------------+
  | ext-tui: Text wrapping                                              |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_WRAP_H
#define TUI_WRAP_H

typedef enum {
    TUI_WRAP_NONE,      /* No wrapping, truncate */
    TUI_WRAP_CHAR,      /* Wrap at character boundary */
    TUI_WRAP_WORD,      /* Wrap at word boundary */
    TUI_WRAP_WORD_CHAR  /* Word wrap, fallback to char */
} tui_wrap_mode;

typedef struct {
    char **lines;
    int count;
    int capacity;
} tui_wrapped_text;

/* Wrap text to fit width */
tui_wrapped_text* tui_wrap_text(const char *text, int width, tui_wrap_mode mode);

/* Free wrapped text */
void tui_wrapped_text_free(tui_wrapped_text *wrapped);

/* Truncate text to width with ellipsis */
char* tui_truncate_text(const char *text, int width, const char *ellipsis);

#endif /* TUI_WRAP_H */
