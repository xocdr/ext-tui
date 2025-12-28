/*
  +----------------------------------------------------------------------+
  | ext-tui: Terminal output                                            |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_OUTPUT_H
#define TUI_OUTPUT_H

#include "buffer.h"

/* Output modes */
typedef enum {
    TUI_OUTPUT_NORMAL,      /* Normal screen */
    TUI_OUTPUT_ALTERNATE    /* Alternate screen buffer */
} tui_output_mode;

/* Renderer state */
typedef struct {
    tui_buffer *front;      /* Current display */
    tui_buffer *back;       /* Next frame */
    tui_output_mode mode;
    int cursor_x;
    int cursor_y;
    int cursor_visible;
} tui_output;

/* Lifecycle */
tui_output* tui_output_create(int width, int height);
void tui_output_destroy(tui_output *out);

/* Screen modes */
void tui_output_enter_alternate(tui_output *out);
void tui_output_exit_alternate(tui_output *out);

/* Rendering */
void tui_output_render(tui_output *out, tui_buffer *buf);
void tui_output_flush(tui_output *out);

/* Cursor */
void tui_output_show_cursor(tui_output *out);
void tui_output_hide_cursor(tui_output *out);
void tui_output_move_cursor(tui_output *out, int x, int y);

#endif /* TUI_OUTPUT_H */
