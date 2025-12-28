/*
  +----------------------------------------------------------------------+
  | ext-tui: Event loop                                                 |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_LOOP_H
#define TUI_LOOP_H

typedef void (*tui_input_callback)(const char *input, int len, void *userdata);
typedef void (*tui_timer_callback)(void *userdata);
typedef void (*tui_resize_callback)(int width, int height, void *userdata);
typedef void (*tui_tick_callback)(void *userdata);

typedef struct tui_loop tui_loop;

/* Lifecycle */
tui_loop* tui_loop_create(void);
void tui_loop_destroy(tui_loop *loop);

/* Run loop */
int tui_loop_run(tui_loop *loop);
void tui_loop_stop(tui_loop *loop);

/* Callbacks */
void tui_loop_on_input(tui_loop *loop, tui_input_callback cb, void *userdata);
void tui_loop_on_resize(tui_loop *loop, tui_resize_callback cb, void *userdata);
void tui_loop_on_tick(tui_loop *loop, tui_tick_callback cb, void *userdata);

/* Timers */
int tui_loop_add_timer(tui_loop *loop, int interval_ms, tui_timer_callback cb, void *userdata);
void tui_loop_remove_timer(tui_loop *loop, int timer_id);

#endif /* TUI_LOOP_H */
