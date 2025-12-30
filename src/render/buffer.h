/*
  +----------------------------------------------------------------------+
  | ext-tui: Character buffer                                           |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_BUFFER_H
#define TUI_BUFFER_H

#include <stdint.h>
#include "../node/node.h"

typedef struct {
    uint32_t codepoint;  /* Unicode codepoint */
    tui_style style;
    uint8_t dirty;       /* Needs redraw */
} tui_cell;

typedef struct {
    tui_cell *cells;
    int width;
    int height;
} tui_buffer;

/* Lifecycle */
tui_buffer* tui_buffer_create(int width, int height);
void tui_buffer_destroy(tui_buffer *buf);

/**
 * Resize buffer to new dimensions.
 * @param buf Buffer to resize
 * @param width New width (1-10000)
 * @param height New height (1-10000)
 * @return 0 on success, -1 on failure (buffer unchanged on failure)
 */
int tui_buffer_resize(tui_buffer *buf, int width, int height);

/* Drawing */
void tui_buffer_clear(tui_buffer *buf);
void tui_buffer_set_cell(tui_buffer *buf, int x, int y, uint32_t ch, const tui_style *style);
void tui_buffer_write_text(tui_buffer *buf, int x, int y, const char *text, const tui_style *style);
void tui_buffer_fill_rect(tui_buffer *buf, int x, int y, int w, int h, uint32_t ch, const tui_style *style);

/* Cell access */
tui_cell* tui_buffer_get_cell(tui_buffer *buf, int x, int y);

/* Diffing */
void tui_buffer_mark_all_dirty(tui_buffer *buf);
void tui_buffer_mark_clean(tui_buffer *buf);

/* Rendering to string */
char* tui_buffer_to_string(tui_buffer *buf);

/* Configuration - set by tui.c from INI values */
void tui_buffer_set_max_dimensions(int max_width, int max_height);
int tui_buffer_get_max_width(void);
int tui_buffer_get_max_height(void);

#endif /* TUI_BUFFER_H */
