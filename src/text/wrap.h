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

/* Truncation position modes */
typedef enum {
    TUI_TRUNCATE_END,      /* "Hello Wo..." - truncate at end (default) */
    TUI_TRUNCATE_START,    /* "...lo World" - truncate at start */
    TUI_TRUNCATE_MIDDLE    /* "Hello...orld" - truncate in middle */
} tui_truncate_position;

typedef struct {
    char **lines;
    int count;
    int capacity;
} tui_wrapped_text;

/* Wrap text to fit width */
tui_wrapped_text* tui_wrap_text(const char *text, int width, tui_wrap_mode mode);

/* Free wrapped text */
void tui_wrapped_text_free(tui_wrapped_text *wrapped);

/* Truncate text to width with ellipsis (truncates at end) */
char* tui_truncate_text(const char *text, int width, const char *ellipsis);

/* Truncate text with position control */
char* tui_truncate_text_ex(const char *text, int width, const char *ellipsis, tui_truncate_position position);

#endif /* TUI_WRAP_H */
