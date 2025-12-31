/*
  +----------------------------------------------------------------------+
  | ext-tui: Application state management                               |
  +----------------------------------------------------------------------+
  | Central application state for TUI applications. Manages the render  |
  | loop, event handling, state hooks, focus system, and timers.        |
  |                                                                      |
  | Thread Safety: NOT thread-safe. All calls must be from the main     |
  | thread. Signal handlers update volatile flags only.                 |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_APP_H
#define TUI_APP_H

#include "../node/node.h"
#include "../render/buffer.h"
#include "../render/output.h"
#include "../event/loop.h"
#include "php.h"

/**
 * State slot for useState hook.
 * Each call to useState() allocates or reuses a slot.
 */
typedef struct {
    zval value;           /* Current state value */
    zval setter;          /* Setter closure (for [$state, $setState]) */
    int index;            /* State slot index */
} tui_state_slot;

/* Initial capacity for dynamic state array */
#define INITIAL_STATE_CAPACITY 8

/* Forward declaration for callback pointer */
typedef struct tui_app tui_app;

/**
 * Main application state structure.
 */
struct tui_app {
    /* ---- Terminal state ---- */
    int fullscreen;           /* Use alternate screen buffer */
    int exit_on_ctrl_c;       /* Exit on Ctrl+C */
    int running;              /* Currently in event loop */
    int should_exit;          /* Exit requested */
    int exit_code;            /* Exit code to return */

    /* ---- Layout dimensions ---- */
    int width;                /* Terminal width in columns */
    int height;               /* Terminal height in rows */

    /* ---- Component callback (main render function) ---- */
    zend_fcall_info component_fci;
    zend_fcall_info_cache component_fcc;

    /* ---- Event callbacks ---- */
    zend_fcall_info input_fci;
    zend_fcall_info_cache input_fcc;
    int has_input_handler;

    zend_fcall_info focus_fci;
    zend_fcall_info_cache focus_fcc;
    int has_focus_handler;

    zend_fcall_info resize_fci;
    zend_fcall_info_cache resize_fcc;
    int has_resize_handler;

    zend_fcall_info tick_fci;
    zend_fcall_info_cache tick_fcc;
    int has_tick_handler;

    /* ---- Focus management ---- */
    tui_node *focused_node;   /* Currently focused node (NULL = none) */
    int focus_enabled;        /* Whether focus system is active */

    /* ---- Virtual DOM ---- */
    tui_node *root_node;      /* Root of current node tree */

    /* ---- Render state ---- */
    tui_buffer *buffer;       /* Character buffer */
    tui_output *output;       /* Terminal output */

    /* ---- Event loop ---- */
    tui_loop *loop;           /* Event loop instance */

    /* ---- Render throttling ---- */
    int render_pending;           /* Re-render existing tree */
    int rerender_pending;         /* Full re-render (call component) */
    int min_render_interval_ms;   /* Throttle interval (16ms = 60fps) */

    /* ---- Rerender callback (set by tui.c) ---- */
    void (*rerender_callback)(struct tui_app *app);

    /* ---- Console output capture ---- */
    char *captured_output;        /* Output from echo/print during render */
    size_t captured_output_len;

    /* ---- Timer callbacks ---- */
    #define INITIAL_TIMER_CAPACITY 4
    struct tui_timer_callback {
        int id;                   /* Timer ID from event loop */
        zend_fcall_info fci;
        zend_fcall_info_cache fcc;
        int active;               /* Whether timer is active */
        struct tui_app *app;      /* Back-pointer for safe invocation */
    } *timer_callbacks;           /* Dynamic timer array */
    int timer_capacity;           /* Current allocated capacity */
    int timer_callback_count;     /* Number of used slots */

    /* ---- Cleanup state ---- */
    int destroyed;                /* Prevent double-free */

    /* ---- Re-entrancy protection ---- */
    int is_rendering;             /* Set during render */
    int rerender_requested;       /* Rerender requested during render */

    /* ---- useState hook state ---- */
    tui_state_slot *states;       /* Dynamic state slot array */
    int state_capacity;           /* Current allocated capacity */
    int state_count;              /* Number of allocated state slots */
    int state_index;              /* Current hook index (reset each render) */

    /* ---- PHP Instance reference ---- */
    zval instance_zval;           /* TuiInstance object zval */
    int instance_zval_set;        /* Whether instance_zval is valid */
};

/* ================================================================
 * Lifecycle
 * ================================================================ */

/**
 * Create a new app instance.
 * Initializes output, buffer, and event loop.
 * @return New app instance, or NULL on allocation failure
 */
tui_app* tui_app_create(void);

/**
 * Destroy app and free all resources.
 * Stops event loop if running, cleans up all PHP references.
 * @param app App instance (NULL-safe)
 */
void tui_app_destroy(tui_app *app);

/* ================================================================
 * Configuration
 * ================================================================ */

/**
 * Enable or disable fullscreen mode (alternate screen buffer).
 * @param app        App instance
 * @param fullscreen 1 to enable, 0 to disable
 */
void tui_app_set_fullscreen(tui_app *app, int fullscreen);

/**
 * Enable or disable exit on Ctrl+C.
 * @param app     App instance
 * @param enabled 1 to enable, 0 to disable
 */
void tui_app_set_exit_on_ctrl_c(tui_app *app, int enabled);

/* ================================================================
 * Component and event handlers
 * ================================================================ */

/**
 * Set the main component callback.
 * Called on each rerender to generate the node tree.
 * @param app App instance
 * @param fci Function call info (PHP callable)
 * @param fcc Function call cache
 */
void tui_app_set_component(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc);

/**
 * Set the input event handler.
 * @param app App instance
 * @param fci Function call info (PHP callable)
 * @param fcc Function call cache
 */
void tui_app_set_input_handler(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc);

/**
 * Set the focus change handler.
 * @param app App instance
 * @param fci Function call info (PHP callable)
 * @param fcc Function call cache
 */
void tui_app_set_focus_handler(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc);

/**
 * Set the resize event handler.
 * @param app App instance
 * @param fci Function call info (PHP callable)
 * @param fcc Function call cache
 */
void tui_app_set_resize_handler(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc);

/**
 * Set the tick handler (called each event loop iteration).
 * @param app App instance
 * @param fci Function call info (PHP callable)
 * @param fcc Function call cache
 */
void tui_app_set_tick_handler(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc);

/* ================================================================
 * Focus management
 * ================================================================ */

/**
 * Move focus to next focusable node.
 * @param app App instance
 */
void tui_app_focus_next(tui_app *app);

/**
 * Move focus to previous focusable node.
 * @param app App instance
 */
void tui_app_focus_prev(tui_app *app);

/**
 * Set focus to specific node.
 * @param app  App instance
 * @param node Node to focus (NULL to clear focus)
 */
void tui_app_set_focus(tui_app *app, tui_node *node);

/**
 * Focus a node by its ID.
 * @param app App instance
 * @param id  Node ID to focus
 * @return 1 if node found and focused, 0 otherwise
 */
int tui_app_focus_by_id(tui_app *app, const char *id);

/**
 * Enable the focus system.
 * @param app App instance
 */
void tui_app_enable_focus(tui_app *app);

/**
 * Disable the focus system.
 * @param app App instance
 */
void tui_app_disable_focus(tui_app *app);

/* ================================================================
 * Rendering and event loop
 * ================================================================ */

/**
 * Start the event loop (blocking).
 * Returns when exit is requested.
 * @param app App instance
 * @return Exit code
 */
int tui_app_start(tui_app *app);

/**
 * Trigger a full rerender (call component callback).
 * @param app App instance
 */
void tui_app_render(tui_app *app);

/**
 * Render existing node tree without calling component.
 * @param app App instance
 */
void tui_app_render_tree(tui_app *app);

/**
 * Stop the event loop.
 * @param app App instance
 */
void tui_app_stop(tui_app *app);

/**
 * Wait for app to exit (blocking).
 * @param app App instance
 */
void tui_app_wait_until_exit(tui_app *app);

/**
 * Request app exit with code.
 * @param app  App instance
 * @param code Exit code
 */
void tui_app_exit(tui_app *app, int code);

/* ================================================================
 * Timers
 * ================================================================ */

/**
 * Add a repeating timer.
 * @param app         App instance
 * @param interval_ms Interval in milliseconds
 * @param fci         Callback function call info
 * @param fcc         Callback function call cache
 * @return Timer ID (>= 0), or -1 on failure
 */
int tui_app_add_timer(tui_app *app, int interval_ms, zend_fcall_info *fci, zend_fcall_info_cache *fcc);

/**
 * Remove a timer by ID.
 * @param app      App instance
 * @param timer_id Timer ID returned from tui_app_add_timer
 */
void tui_app_remove_timer(tui_app *app, int timer_id);

/* ================================================================
 * Internal callbacks (called by event loop)
 * ================================================================ */

/**
 * Handle keyboard/mouse input.
 * @param input    Input buffer (may contain escape sequences)
 * @param len      Length of input
 * @param userdata App instance pointer
 */
void tui_app_on_input(const char *input, int len, void *userdata);

/**
 * Handle terminal resize.
 * @param width    New width
 * @param height   New height
 * @param userdata App instance pointer
 */
void tui_app_on_resize(int width, int height, void *userdata);

/* ================================================================
 * State management (for useState hook)
 * ================================================================ */

/**
 * Get or create a state slot for useState.
 * @param app     App instance
 * @param initial Initial value (used only if creating new slot)
 * @param is_new  Output: set to 1 if new slot created, 0 if reused
 * @return State slot index (>= 0), or -1 on failure
 */
int tui_app_get_or_create_state_slot(tui_app *app, zval *initial, int *is_new);

/**
 * Reset state index for new render cycle.
 * @param app App instance
 */
void tui_app_reset_state_index(tui_app *app);

/**
 * Clean up all state slots and free PHP references.
 * @param app App instance
 */
void tui_app_cleanup_states(tui_app *app);

/* ================================================================
 * Testing support
 * ================================================================ */

/**
 * Inject input directly without polling (for headless testing).
 * @param app   App instance
 * @param input Input buffer (characters or escape sequences)
 * @param len   Length of input buffer
 */
void tui_app_inject_input(tui_app *app, const char *input, int len);

/**
 * Render a node tree to an external buffer (for headless testing).
 * @param buffer   Target buffer
 * @param node     Root node to render
 * @param offset_x X offset for rendering
 * @param offset_y Y offset for rendering
 * @param clip_x   Clip region X (reserved, unused)
 * @param clip_y   Clip region Y (reserved, unused)
 * @param clip_w   Clip region width (reserved, unused)
 * @param clip_h   Clip region height (reserved, unused)
 */
void tui_app_render_node_to_buffer(tui_buffer *buffer, tui_node *node,
                                    int offset_x, int offset_y,
                                    int clip_x, int clip_y,
                                    int clip_w, int clip_h);

#endif /* TUI_APP_H */
