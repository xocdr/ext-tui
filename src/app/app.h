/*
  +----------------------------------------------------------------------+
  | ext-tui: Application state management                               |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_APP_H
#define TUI_APP_H

#include "../node/node.h"
#include "../render/buffer.h"
#include "../render/output.h"
#include "../event/loop.h"
#include "php.h"

/* State slot for useState hook */
typedef struct {
    zval value;           /* Current state value */
    zval setter;          /* Setter closure */
    int index;            /* State slot index */
} tui_state_slot;

/* Maximum state slots per app */
#define TUI_MAX_STATES 64

/* Forward declaration for callback pointer */
typedef struct tui_app tui_app;

struct tui_app {
    /* Terminal state */
    int fullscreen;
    int exit_on_ctrl_c;
    int running;
    int should_exit;
    int exit_code;

    /* Layout dimensions */
    int width;
    int height;

    /* Component callback */
    zend_fcall_info component_fci;
    zend_fcall_info_cache component_fcc;

    /* Input callback */
    zend_fcall_info input_fci;
    zend_fcall_info_cache input_fcc;
    int has_input_handler;

    /* Focus change callback */
    zend_fcall_info focus_fci;
    zend_fcall_info_cache focus_fcc;
    int has_focus_handler;

    /* Resize callback */
    zend_fcall_info resize_fci;
    zend_fcall_info_cache resize_fcc;
    int has_resize_handler;

    /* Tick callback */
    zend_fcall_info tick_fci;
    zend_fcall_info_cache tick_fcc;
    int has_tick_handler;

    /* Current focused node */
    tui_node *focused_node;

    /* Virtual DOM */
    tui_node *root_node;

    /* Render state */
    tui_buffer *buffer;
    tui_output *output;

    /* Event loop */
    tui_loop *loop;

    /* Render throttling */
    int render_pending;           /* Re-render existing tree */
    int rerender_pending;         /* Full re-render (call component callback) */
    int min_render_interval_ms;   /* 16ms = 60fps */

    /* Rerender callback - set by tui.c to call render_component_callback */
    void (*rerender_callback)(struct tui_app *app);

    /* Captured console output from last render (echo/print during component) */
    char *captured_output;
    size_t captured_output_len;

    /* Timer callbacks - store PHP callbacks for timers */
    #define TUI_MAX_TIMERS 32
    struct tui_timer_callback {
        int id;
        zend_fcall_info fci;
        zend_fcall_info_cache fcc;
        int active;
        struct tui_app *app;  /* Back-pointer for safe callback invocation */
    } timer_callbacks[TUI_MAX_TIMERS];
    int timer_callback_count;

    /* Cleanup state */
    int destroyed;  /* Prevent double-free */

    /* useState hook state management */
    tui_state_slot states[TUI_MAX_STATES];
    int state_count;          /* Number of allocated state slots */
    int state_index;          /* Current hook index (reset each render) */

    /* Focus management */
    int focus_enabled;        /* Whether focus system is active */

    /* PHP Instance object reference (for passing to render callback) */
    zval *instance_zval;      /* Pointer to the Instance zval */
};

/* Lifecycle */
tui_app* tui_app_create(void);
void tui_app_destroy(tui_app *app);

/* Configuration */
void tui_app_set_fullscreen(tui_app *app, int fullscreen);
void tui_app_set_exit_on_ctrl_c(tui_app *app, int enabled);

/* Component */
void tui_app_set_component(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc);
void tui_app_set_input_handler(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc);
void tui_app_set_focus_handler(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc);
void tui_app_set_resize_handler(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc);
void tui_app_set_tick_handler(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc);

/* Focus management */
void tui_app_focus_next(tui_app *app);
void tui_app_focus_prev(tui_app *app);
void tui_app_set_focus(tui_app *app, tui_node *node);

/* Rendering */
int tui_app_start(tui_app *app);
void tui_app_render(tui_app *app);
void tui_app_render_tree(tui_app *app);  /* Render existing tree without calling component */
void tui_app_stop(tui_app *app);
void tui_app_wait_until_exit(tui_app *app);
void tui_app_exit(tui_app *app, int code);

/* Timers */
int tui_app_add_timer(tui_app *app, int interval_ms, zend_fcall_info *fci, zend_fcall_info_cache *fcc);
void tui_app_remove_timer(tui_app *app, int timer_id);

/* Internal callbacks */
void tui_app_on_input(const char *input, int len, void *userdata);
void tui_app_on_resize(int width, int height, void *userdata);

/* State management for useState hook */
int tui_app_get_or_create_state_slot(tui_app *app, zval *initial, int *is_new);
void tui_app_reset_state_index(tui_app *app);
void tui_app_cleanup_states(tui_app *app);

/* Focus by ID */
int tui_app_focus_by_id(tui_app *app, const char *id);
void tui_app_enable_focus(tui_app *app);
void tui_app_disable_focus(tui_app *app);

#endif /* TUI_APP_H */
