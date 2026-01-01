/*
  +----------------------------------------------------------------------+
  | ext-tui: Application state management                               |
  +----------------------------------------------------------------------+
*/

#include "app.h"
#include "../terminal/terminal.h"
#include "../terminal/ansi.h"
#include "../event/input.h"
#include "php.h"
#include "php_tui.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <termios.h>

/* External class entries for event objects (defined in tui.c) */
extern zend_class_entry *tui_key_ce;
extern zend_class_entry *tui_focus_event_ce;

/* Forward declaration for rendering a node tree to buffer */
static void render_node_to_buffer(tui_app *app, tui_node *node, int offset_x, int offset_y);

tui_app* tui_app_create(void)
{
    tui_app *app = calloc(1, sizeof(tui_app));
    if (!app) return NULL;

    app->fullscreen = 1;
    app->exit_on_ctrl_c = 1;
    app->min_render_interval_ms = 16;  /* 60fps */

    /* Initialize all zvals to UNDEF for safe cleanup.
     * This ensures zval_ptr_dtor checks in tui_app_destroy() work correctly
     * even if callbacks were never set. calloc zeros memory but that's not
     * the same as ZVAL_UNDEF which has IS_UNDEF type. */
    ZVAL_UNDEF(&app->component_fci.function_name);
    ZVAL_UNDEF(&app->input_fci.function_name);
    ZVAL_UNDEF(&app->focus_fci.function_name);
    ZVAL_UNDEF(&app->resize_fci.function_name);
    ZVAL_UNDEF(&app->tick_fci.function_name);
    ZVAL_UNDEF(&app->instance_zval);

    /* Allocate initial state array */
    app->states = calloc(INITIAL_STATE_CAPACITY, sizeof(tui_state_slot));
    if (!app->states) {
        free(app);
        return NULL;
    }
    app->state_capacity = INITIAL_STATE_CAPACITY;

    /* Allocate initial timer array */
    app->timer_callbacks = calloc(INITIAL_TIMER_CAPACITY, sizeof(struct tui_timer_callback));
    if (!app->timer_callbacks) {
        free(app->states);
        free(app);
        return NULL;
    }
    app->timer_capacity = INITIAL_TIMER_CAPACITY;

    /* Get terminal size */
    tui_terminal_get_size(&app->width, &app->height);

    /* Create output system with proper cleanup on failure */
    app->output = tui_output_create(app->width, app->height);
    if (!app->output) {
        free(app->timer_callbacks);
        free(app->states);
        free(app);
        return NULL;
    }

    app->buffer = tui_buffer_create(app->width, app->height);
    if (!app->buffer) {
        tui_output_destroy(app->output);
        free(app->timer_callbacks);
        free(app->states);
        free(app);
        return NULL;
    }

    /* Create event loop */
    app->loop = tui_loop_create();
    if (!app->loop) {
        tui_buffer_destroy(app->buffer);
        tui_output_destroy(app->output);
        free(app->timer_callbacks);
        free(app->states);
        free(app);
        return NULL;
    }

    return app;
}

void tui_app_destroy(tui_app *app)
{
    if (!app) return;

    /* Prevent double-free */
    if (app->destroyed) return;
    app->destroyed = 1;

    /* Stop if running */
    if (app->running) {
        tui_app_stop(app);
    }

    /* Free callback references (both function_name and object)
     * Only free if we're still in a valid PHP execution context.
     * Check that the zval is valid before attempting to free. */
    if (app->component_fci.size && !Z_ISUNDEF(app->component_fci.function_name)) {
        zval_ptr_dtor(&app->component_fci.function_name);
        ZVAL_UNDEF(&app->component_fci.function_name);
        if (app->component_fcc.object) {
            OBJ_RELEASE(app->component_fcc.object);
            app->component_fcc.object = NULL;
        }
    }
    if (app->has_input_handler && app->input_fci.size && !Z_ISUNDEF(app->input_fci.function_name)) {
        zval_ptr_dtor(&app->input_fci.function_name);
        ZVAL_UNDEF(&app->input_fci.function_name);
        if (app->input_fcc.object) {
            OBJ_RELEASE(app->input_fcc.object);
            app->input_fcc.object = NULL;
        }
    }
    if (app->has_focus_handler && app->focus_fci.size && !Z_ISUNDEF(app->focus_fci.function_name)) {
        zval_ptr_dtor(&app->focus_fci.function_name);
        ZVAL_UNDEF(&app->focus_fci.function_name);
        if (app->focus_fcc.object) {
            OBJ_RELEASE(app->focus_fcc.object);
            app->focus_fcc.object = NULL;
        }
    }
    if (app->has_resize_handler && app->resize_fci.size && !Z_ISUNDEF(app->resize_fci.function_name)) {
        zval_ptr_dtor(&app->resize_fci.function_name);
        ZVAL_UNDEF(&app->resize_fci.function_name);
        if (app->resize_fcc.object) {
            OBJ_RELEASE(app->resize_fcc.object);
            app->resize_fcc.object = NULL;
        }
    }
    if (app->has_tick_handler && app->tick_fci.size && !Z_ISUNDEF(app->tick_fci.function_name)) {
        zval_ptr_dtor(&app->tick_fci.function_name);
        ZVAL_UNDEF(&app->tick_fci.function_name);
        if (app->tick_fcc.object) {
            OBJ_RELEASE(app->tick_fcc.object);
            app->tick_fcc.object = NULL;
        }
    }

    /* Clean up timer callbacks */
    for (int i = 0; i < app->timer_callback_count; i++) {
        if (app->timer_callbacks[i].active) {
            app->timer_callbacks[i].active = 0;
            if (!Z_ISUNDEF(app->timer_callbacks[i].fci.function_name)) {
                zval_ptr_dtor(&app->timer_callbacks[i].fci.function_name);
                ZVAL_UNDEF(&app->timer_callbacks[i].fci.function_name);
            }
            if (app->timer_callbacks[i].fcc.object) {
                OBJ_RELEASE(app->timer_callbacks[i].fcc.object);
                app->timer_callbacks[i].fcc.object = NULL;
            }
        }
    }
    /* Free dynamic timer array */
    if (app->timer_callbacks) {
        free(app->timer_callbacks);
        app->timer_callbacks = NULL;
    }

    /* Clean up useState slots */
    tui_app_cleanup_states(app);

    /* Free dynamic state array */
    if (app->states) {
        free(app->states);
        app->states = NULL;
    }

    /* Clean up instance_zval if set (added refcount in tui_render.c) */
    if (app->instance_zval_set) {
        zval_ptr_dtor(&app->instance_zval);
        app->instance_zval_set = 0;
    }

    /* Free captured output buffer - allocated with estrndup() in tui_render.c */
    if (app->captured_output) {
        efree(app->captured_output);
        app->captured_output = NULL;
        app->captured_output_len = 0;
    }

    /* Free resources */
    if (app->root_node) {
        tui_node_destroy(app->root_node);
        app->root_node = NULL;
    }
    if (app->buffer) {
        tui_buffer_destroy(app->buffer);
        app->buffer = NULL;
    }
    if (app->output) {
        tui_output_destroy(app->output);
        app->output = NULL;
    }
    if (app->loop) {
        tui_loop_destroy(app->loop);
        app->loop = NULL;
    }

    free(app);
}

void tui_app_set_fullscreen(tui_app *app, int fullscreen)
{
    if (app) {
        app->fullscreen = fullscreen;
    }
}

void tui_app_set_exit_on_ctrl_c(tui_app *app, int enabled)
{
    if (app) {
        app->exit_on_ctrl_c = enabled;
    }
}

void tui_app_set_component(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc)
{
    if (app && fci && fcc) {
        /* Release previous callback references if set */
        if (app->component_fci.size) {
            zval_ptr_dtor(&app->component_fci.function_name);
            if (app->component_fcc.object) {
                OBJ_RELEASE(app->component_fcc.object);
            }
        }
        app->component_fci = *fci;
        app->component_fcc = *fcc;
        /* Add references to prevent garbage collection */
        Z_TRY_ADDREF(app->component_fci.function_name);
        if (app->component_fcc.object) {
            GC_ADDREF(app->component_fcc.object);
        }
    }
}

void tui_app_set_input_handler(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc)
{
    if (app && fci && fcc) {
        /* Release previous callback references if set */
        if (app->has_input_handler && app->input_fci.size) {
            zval_ptr_dtor(&app->input_fci.function_name);
            if (app->input_fcc.object) {
                OBJ_RELEASE(app->input_fcc.object);
            }
        }
        app->input_fci = *fci;
        app->input_fcc = *fcc;
        app->has_input_handler = 1;
        /* Add references to prevent garbage collection */
        Z_TRY_ADDREF(app->input_fci.function_name);
        if (app->input_fcc.object) {
            GC_ADDREF(app->input_fcc.object);
        }
    }
}

void tui_app_set_focus_handler(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc)
{
    if (app && fci && fcc) {
        /* Release previous callback references if set */
        if (app->has_focus_handler && app->focus_fci.size) {
            zval_ptr_dtor(&app->focus_fci.function_name);
            if (app->focus_fcc.object) {
                OBJ_RELEASE(app->focus_fcc.object);
            }
        }
        app->focus_fci = *fci;
        app->focus_fcc = *fcc;
        app->has_focus_handler = 1;
        /* Add references to prevent garbage collection */
        Z_TRY_ADDREF(app->focus_fci.function_name);
        if (app->focus_fcc.object) {
            GC_ADDREF(app->focus_fcc.object);
        }
    }
}

void tui_app_set_resize_handler(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc)
{
    if (app && fci && fcc) {
        /* Release previous callback references if set */
        if (app->has_resize_handler && app->resize_fci.size) {
            zval_ptr_dtor(&app->resize_fci.function_name);
            if (app->resize_fcc.object) {
                OBJ_RELEASE(app->resize_fcc.object);
            }
        }
        app->resize_fci = *fci;
        app->resize_fcc = *fcc;
        app->has_resize_handler = 1;
        /* Add references to prevent garbage collection */
        Z_TRY_ADDREF(app->resize_fci.function_name);
        if (app->resize_fcc.object) {
            GC_ADDREF(app->resize_fcc.object);
        }
    }
}

/* Tick callback wrapper - called from event loop on each iteration */
static void tick_callback_wrapper(void *userdata)
{
    tui_app *app = (tui_app *)userdata;
    if (!app || !app->has_tick_handler) return;

    /* Capture any output during callback to prevent terminal corruption */
    php_output_start_default();

    zval retval;
    app->tick_fci.retval = &retval;
    app->tick_fci.param_count = 0;
    app->tick_fci.params = NULL;

    if (zend_call_function(&app->tick_fci, &app->tick_fcc) == SUCCESS) {
        zval_ptr_dtor(&retval);
    }

    /* Discard any captured output */
    php_output_discard();
}

void tui_app_set_tick_handler(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc)
{
    if (app && fci && fcc) {
        /* Release previous callback references if set */
        if (app->has_tick_handler && app->tick_fci.size) {
            zval_ptr_dtor(&app->tick_fci.function_name);
            if (app->tick_fcc.object) {
                OBJ_RELEASE(app->tick_fcc.object);
            }
        }
        app->tick_fci = *fci;
        app->tick_fcc = *fcc;
        app->has_tick_handler = 1;
        /* Add references to prevent garbage collection */
        Z_TRY_ADDREF(app->tick_fci.function_name);
        if (app->tick_fcc.object) {
            GC_ADDREF(app->tick_fcc.object);
        }

        /* Register the tick callback with the event loop */
        if (app->loop) {
            tui_loop_on_tick(app->loop, tick_callback_wrapper, app);
        }
    }
}

/* Maximum tree depth to prevent stack overflow from circular references or very deep trees */
#define MAX_TREE_DEPTH 256

/* ==========================================================================
 * FOCUS TRAVERSAL ALGORITHM
 *
 * Focus management follows a depth-first tree order:
 *
 * 1. collect_focusable_nodes(): DFS traversal collecting all nodes where
 *    node->focusable == true into a flat array. Order matches tree order
 *    (parent before children, left-to-right siblings).
 *
 * 2. tui_app_focus_next(): Move to next focusable node in array (wrapping)
 * 3. tui_app_focus_prev(): Move to previous focusable node (wrapping)
 * 4. tui_app_focus_by_id(): Focus specific node by ID string
 *
 * Tab key navigation (in tui_app_on_input):
 * - Tab: focus_next
 * - Shift+Tab: focus_prev
 *
 * Focus changes trigger:
 * - Old node: focused = false
 * - New node: focused = true
 * - Focus handler callback (if registered)
 * - render_pending = 1 (triggers re-render)
 *
 * This gives predictable, accessibility-friendly focus order matching
 * visual layout (top-to-bottom, left-to-right for LTR).
 * ========================================================================== */

/**
 * Recursively collect all focusable nodes from the tree in DFS order.
 *
 * @param node     Current node to process
 * @param nodes    Pointer to array of node pointers (may be reallocated)
 * @param count    Pointer to current count of collected nodes
 * @param capacity Pointer to current array capacity
 * @param depth    Current recursion depth (for overflow protection)
 * @return         0 on success, -1 on error
 */
static int collect_focusable_nodes_impl(tui_node *node, tui_node ***nodes, int *count, int *capacity, int depth)
{
    if (!node) return 0;

    /* Prevent stack overflow from very deep trees or circular references */
    if (depth > MAX_TREE_DEPTH) return -1;

    if (node->focusable) {
        if (*count >= *capacity) {
            /* Check for overflow before doubling */
            if (*capacity > INT_MAX / 2) return -1;
            int new_capacity = *capacity * 2;
            tui_node **new_nodes = realloc(*nodes, new_capacity * sizeof(tui_node*));
            if (!new_nodes) {
                /* Realloc failed - return error but don't lose original pointer */
                return -1;
            }
            *nodes = new_nodes;
            *capacity = new_capacity;
        }
        (*nodes)[(*count)++] = node;
    }

    for (int i = 0; i < node->child_count; i++) {
        if (collect_focusable_nodes_impl(node->children[i], nodes, count, capacity, depth + 1) < 0) {
            return -1;
        }
    }

    return 0;
}

/* Helper: collect all focusable nodes */
static int collect_focusable_nodes(tui_node *node, tui_node ***nodes, int *count, int *capacity)
{
    return collect_focusable_nodes_impl(node, nodes, count, capacity, 0);
}

/* Helper: create array with node info */
static void create_node_info_array(zval *arr, tui_node *node)
{
    array_init(arr);
    add_assoc_bool(arr, "focusable", node->focusable);
    add_assoc_bool(arr, "focused", node->focused);
    add_assoc_long(arr, "x", (zend_long)node->x);
    add_assoc_long(arr, "y", (zend_long)node->y);
    add_assoc_long(arr, "width", (zend_long)node->width);
    add_assoc_long(arr, "height", (zend_long)node->height);
    if (node->type == TUI_NODE_TEXT && node->text) {
        add_assoc_string(arr, "type", "text");
        add_assoc_string(arr, "content", node->text);
    } else {
        add_assoc_string(arr, "type", "box");
    }
}

/* Call focus change handler with TuiFocusEvent object */
static void call_focus_handler(tui_app *app, tui_node *old_node, tui_node *new_node, const char *direction)
{
    if (!app || !app->has_focus_handler || !tui_focus_event_ce) return;

    /* Capture any output during callback to prevent terminal corruption */
    php_output_start_default();

    zval retval;
    zval event;
    zval previous, current;

    /* Create TuiFocusEvent object */
    object_init_ex(&event, tui_focus_event_ce);

    /* Set previous node info (or null) */
    if (old_node) {
        create_node_info_array(&previous, old_node);
        zend_update_property(tui_focus_event_ce, Z_OBJ(event), "previous", sizeof("previous")-1, &previous);
        zval_ptr_dtor(&previous);
    }

    /* Set current node info (or null) */
    if (new_node) {
        create_node_info_array(&current, new_node);
        zend_update_property(tui_focus_event_ce, Z_OBJ(event), "current", sizeof("current")-1, &current);
        zval_ptr_dtor(&current);
    }

    /* Set direction */
    zend_update_property_string(tui_focus_event_ce, Z_OBJ(event), "direction", sizeof("direction")-1, direction ? direction : "");

    /* Call the PHP handler with TuiFocusEvent */
    zval params[1];
    ZVAL_COPY_VALUE(&params[0], &event);

    app->focus_fci.param_count = 1;
    app->focus_fci.params = params;
    app->focus_fci.retval = &retval;

    if (zend_call_function(&app->focus_fci, &app->focus_fcc) == SUCCESS) {
        zval_ptr_dtor(&retval);
    }

    zval_ptr_dtor(&event);

    /* Discard any captured output */
    php_output_discard();
}

void tui_app_focus_next(tui_app *app)
{
    if (!app || !app->root_node) return;

    int capacity = 16;
    int count = 0;
    tui_node **nodes = malloc(capacity * sizeof(tui_node*));
    if (!nodes) return;

    if (collect_focusable_nodes(app->root_node, &nodes, &count, &capacity) < 0) {
        free(nodes);
        return;
    }

    if (count == 0) {
        free(nodes);
        return;
    }

    /* Find current focused index */
    int current = -1;
    for (int i = 0; i < count; i++) {
        if (nodes[i] == app->focused_node) {
            current = i;
            break;
        }
    }

    /* Move to next (or first if none focused) */
    int next = (current + 1) % count;
    tui_node *old_node = app->focused_node;

    if (old_node) {
        old_node->focused = 0;
    }

    app->focused_node = nodes[next];
    app->focused_node->focused = 1;

    /* Call focus change handler */
    call_focus_handler(app, old_node, app->focused_node, "next");

    free(nodes);

    /* Request re-render */
    app->render_pending = 1;
}

void tui_app_focus_prev(tui_app *app)
{
    if (!app || !app->root_node) return;

    int capacity = 16;
    int count = 0;
    tui_node **nodes = malloc(capacity * sizeof(tui_node*));
    if (!nodes) return;

    if (collect_focusable_nodes(app->root_node, &nodes, &count, &capacity) < 0) {
        free(nodes);
        return;
    }

    if (count == 0) {
        free(nodes);
        return;
    }

    /* Find current focused index */
    int current = -1;
    for (int i = 0; i < count; i++) {
        if (nodes[i] == app->focused_node) {
            current = i;
            break;
        }
    }

    /* Move to previous (or last if none focused) */
    int prev = current <= 0 ? count - 1 : current - 1;
    tui_node *old_node = app->focused_node;

    if (old_node) {
        old_node->focused = 0;
    }

    app->focused_node = nodes[prev];
    app->focused_node->focused = 1;

    /* Call focus change handler */
    call_focus_handler(app, old_node, app->focused_node, "prev");

    free(nodes);

    /* Request re-render */
    app->render_pending = 1;
}

void tui_app_set_focus(tui_app *app, tui_node *node)
{
    if (!app) return;

    tui_node *old_node = app->focused_node;

    if (old_node) {
        old_node->focused = 0;
    }

    if (node && node->focusable) {
        app->focused_node = node;
        node->focused = 1;
    } else {
        app->focused_node = NULL;
    }

    /* Call focus change handler */
    call_focus_handler(app, old_node, app->focused_node, "programmatic");

    /* Request re-render */
    app->render_pending = 1;
}

int tui_app_start(tui_app *app)
{
    if (!app) return -1;

    /* Enable raw mode */
    if (tui_terminal_enable_raw_mode() != 0) {
        return -1;
    }

    /* Enter alternate screen if fullscreen */
    if (app->fullscreen) {
        tui_output_enter_alternate(app->output);
    }

    /* Hide cursor */
    tui_output_hide_cursor(app->output);

    /* Set up event callbacks */
    tui_loop_on_input(app->loop, tui_app_on_input, app);
    tui_loop_on_resize(app->loop, tui_app_on_resize, app);

    app->running = 1;

    /* Initial render - tree is already built by tui_render(), just render to screen */
    tui_app_render_tree(app);

    return 0;
}

/* Helper to get monotonic time in nanoseconds */
static int64_t get_time_ns(void)
{
#ifdef __APPLE__
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
#endif
}

/**
 * Render the current node tree to the terminal buffer and output.
 * Does NOT call the component callback - use tui_app_render() for full render.
 */
void tui_app_render_tree(tui_app *app)
{
    if (!app || !app->running) return;

    int64_t start_ns = 0, layout_end_ns = 0, buffer_end_ns = 0, output_end_ns = 0;

    if (TUI_G(metrics_enabled)) {
        start_ns = get_time_ns();
    }

    /* Clear the buffer */
    tui_buffer_clear(app->buffer);

    /* If we have a root node, render it */
    if (app->root_node) {
        /* Calculate layout */
        tui_node_calculate_layout(app->root_node, app->width, app->height);

        if (TUI_G(metrics_enabled)) {
            layout_end_ns = get_time_ns();
        }

        /* Render to buffer */
        render_node_to_buffer(app, app->root_node, 0, 0);

        if (TUI_G(metrics_enabled)) {
            buffer_end_ns = get_time_ns();
        }
    } else if (TUI_G(metrics_enabled)) {
        layout_end_ns = start_ns;
        buffer_end_ns = start_ns;
    }

    /* Determine if cursor should be shown based on focused node's showCursor property */
    int show_cursor = (app->focused_node && app->focused_node->show_cursor) ? 1 : 0;

    /* Output to terminal */
    tui_output_render_with_cursor(app->output, app->buffer, show_cursor);

    if (TUI_G(metrics_enabled)) {
        output_end_ns = get_time_ns();

        /* Update timing metrics */
        int64_t layout_time = layout_end_ns - start_ns;
        int64_t buffer_time = buffer_end_ns - layout_end_ns;
        int64_t output_time = output_end_ns - buffer_end_ns;
        int64_t total_time = output_end_ns - start_ns;

        TUI_METRIC_ADD(layout_time_ns, layout_time);
        TUI_METRIC_ADD(buffer_time_ns, buffer_time);
        TUI_METRIC_ADD(output_time_ns, output_time);
        TUI_METRIC_INC(render_count);
        TUI_METRIC_MAX(max_render_ns, total_time);
        TUI_METRIC_MIN(min_render_ns, total_time);
    }

    app->render_pending = 0;
}

void tui_app_render(tui_app *app)
{
    if (!app || !app->running) return;

    /* Use rerender_callback to properly call component with Instance parameter */
    if (app->rerender_callback) {
        app->rerender_callback(app);
    }

    /* Render the tree to screen */
    tui_app_render_tree(app);
}

void tui_app_stop(tui_app *app)
{
    if (!app || !app->running) return;

    app->running = 0;

    /* Stop event loop */
    tui_loop_stop(app->loop);

    /* Disable mouse tracking if enabled */
    tui_terminal_disable_mouse();

    /* Disable bracketed paste mode if enabled */
    tui_terminal_disable_bracketed_paste();

    /* Reset text attributes before showing cursor */
    char reset_buf[16];
    size_t reset_len;
    tui_ansi_reset(reset_buf, &reset_len);
    write(STDOUT_FILENO, reset_buf, reset_len);

    /* Show cursor */
    tui_output_show_cursor(app->output);

    /* Exit alternate screen if fullscreen */
    if (app->fullscreen) {
        tui_output_exit_alternate(app->output);
    }

    /* Flush any pending input to prevent stale keystrokes */
    tcflush(STDIN_FILENO, TCIFLUSH);

    /* Disable raw mode - restores original terminal settings */
    tui_terminal_disable_raw_mode();
}

void tui_app_wait_until_exit(tui_app *app)
{
    if (!app || !app->running) return;

    /* Run event loop until stopped */
    while (app->running && !app->should_exit) {
        tui_loop_run(app->loop);

        /* Full re-render if needed (e.g., terminal resize) */
        if (app->rerender_pending && app->rerender_callback) {
            app->rerender_pending = 0;
            app->rerender_callback(app);  /* Rebuild tree */
            tui_app_render_tree(app);     /* Render to screen */
        }
        /* Re-render existing tree if pending (focus changes) */
        else if (app->render_pending) {
            tui_app_render_tree(app);
        }
    }

    /* Clean up terminal state when exiting */
    if (app->running) {
        tui_app_stop(app);
    }
}

void tui_app_exit(tui_app *app, int code)
{
    if (app) {
        app->exit_code = code;
        app->should_exit = 1;
    }
}

/**
 * Ensure timer_callbacks array has room for at least one more timer.
 * Doubles capacity on growth with overflow protection.
 * @return 0 on success, -1 on failure
 */
static int ensure_timer_capacity(tui_app *app)
{
    if (app->timer_callback_count < app->timer_capacity) {
        return 0;  /* Already have room */
    }

    /* Check INI limit */
    zend_long max_timers = TUI_G(max_timers);
    if (app->timer_callback_count >= (int)max_timers) {
        php_error_docref(NULL, E_WARNING,
            "Maximum number of timers (%d) exceeded. "
            "Increase tui.max_timers in php.ini",
            (int)max_timers);
        return -1;
    }

    /* Check for overflow before doubling */
    if (app->timer_capacity > INT_MAX / 2) return -1;
    int new_capacity = app->timer_capacity * 2;

    /* Respect INI limit */
    if (new_capacity > (int)max_timers) {
        new_capacity = (int)max_timers;
    }

    /* Check for size_t overflow */
    if ((size_t)new_capacity > SIZE_MAX / sizeof(struct tui_timer_callback)) return -1;

    struct tui_timer_callback *new_timers = realloc(app->timer_callbacks,
        (size_t)new_capacity * sizeof(struct tui_timer_callback));
    if (!new_timers) return -1;

    /* Zero-initialize new slots */
    memset(&new_timers[app->timer_capacity], 0,
        (new_capacity - app->timer_capacity) * sizeof(struct tui_timer_callback));

    app->timer_callbacks = new_timers;
    app->timer_capacity = new_capacity;
    return 0;
}

/* Timer callback wrapper - called from event loop, invokes PHP callback */
static void timer_callback_wrapper(void *userdata)
{
    /* userdata is a pointer to the timer_callback struct */
    struct tui_timer_callback *timer = (struct tui_timer_callback *)userdata;

    if (!timer || !timer->app || !timer->active) return;

    /* Capture any output during callback to prevent terminal corruption */
    php_output_start_default();

    zval retval;
    timer->fci.retval = &retval;

    if (zend_call_function(&timer->fci, &timer->fcc) == SUCCESS) {
        zval_ptr_dtor(&retval);
    }

    /* Discard any captured output */
    php_output_discard();
}

int tui_app_add_timer(tui_app *app, int interval_ms, zend_fcall_info *fci, zend_fcall_info_cache *fcc)
{
    if (!app || !fci || !fcc) return -1;

    /* Ensure capacity (handles INI limit check internally) */
    if (ensure_timer_capacity(app) < 0) {
        return -1;
    }

    int index = app->timer_callback_count++;

    /* Store PHP callback and back-pointer to app */
    app->timer_callbacks[index].fci = *fci;
    app->timer_callbacks[index].fcc = *fcc;
    app->timer_callbacks[index].active = 1;
    app->timer_callbacks[index].app = app;

    /* Add reference to prevent garbage collection */
    Z_TRY_ADDREF(app->timer_callbacks[index].fci.function_name);
    if (app->timer_callbacks[index].fcc.object) {
        GC_ADDREF(app->timer_callbacks[index].fcc.object);
    }

    /* Pass pointer to timer_callback struct as userdata (safe, no bit-packing) */
    void *userdata = &app->timer_callbacks[index];

    /* Add to event loop */
    int timer_id = tui_loop_add_timer(app->loop, interval_ms, timer_callback_wrapper, userdata);
    app->timer_callbacks[index].id = timer_id;

    return timer_id;
}

void tui_app_remove_timer(tui_app *app, int timer_id)
{
    if (!app) return;

    /* Find and deactivate the timer callback */
    for (int i = 0; i < app->timer_callback_count; i++) {
        if (app->timer_callbacks[i].id == timer_id && app->timer_callbacks[i].active) {
            app->timer_callbacks[i].active = 0;

            /* Release PHP callback references */
            if (!Z_ISUNDEF(app->timer_callbacks[i].fci.function_name)) {
                zval_ptr_dtor(&app->timer_callbacks[i].fci.function_name);
                ZVAL_UNDEF(&app->timer_callbacks[i].fci.function_name);
            }
            if (app->timer_callbacks[i].fcc.object) {
                OBJ_RELEASE(app->timer_callbacks[i].fcc.object);
                app->timer_callbacks[i].fcc.object = NULL;
            }

            /* Remove from event loop */
            tui_loop_remove_timer(app->loop, timer_id);
            break;
        }
    }
}

void tui_app_on_input(const char *input, int len, void *userdata)
{
    tui_app *app = (tui_app *)userdata;
    if (!app) return;

    /* Parse input */
    tui_key_event key;
    tui_input_parse(input, len, &key);

    /* Handle Ctrl+C */
    if (app->exit_on_ctrl_c && key.ctrl && key.key[0] == 'c') {
        tui_app_exit(app, 0);
        return;
    }

    /* Handle Tab for focus navigation */
    if (key.tab && !key.ctrl && !key.meta) {
        if (key.shift) {
            tui_app_focus_prev(app);
        } else {
            tui_app_focus_next(app);
        }
        /* Still call input handler so app can intercept */
    }

    /* Call PHP input handler if set */
    if (app->has_input_handler && tui_key_ce) {
        /* Capture any output during callback to prevent terminal corruption */
        php_output_start_default();

        zval key_obj;
        zval retval;

        /* Create TuiKey object */
        object_init_ex(&key_obj, tui_key_ce);

        /* Set properties */
        zend_update_property_string(tui_key_ce, Z_OBJ(key_obj), "key", sizeof("key")-1, key.key);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "upArrow", sizeof("upArrow")-1, key.upArrow);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "downArrow", sizeof("downArrow")-1, key.downArrow);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "leftArrow", sizeof("leftArrow")-1, key.leftArrow);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "rightArrow", sizeof("rightArrow")-1, key.rightArrow);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "return", sizeof("return")-1, key.enter);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "escape", sizeof("escape")-1, key.escape);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "backspace", sizeof("backspace")-1, key.backspace);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "delete", sizeof("delete")-1, key.delete);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "tab", sizeof("tab")-1, key.tab);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "home", sizeof("home")-1, key.home);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "end", sizeof("end")-1, key.end);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "pageUp", sizeof("pageUp")-1, key.pageUp);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "pageDown", sizeof("pageDown")-1, key.pageDown);
        zend_update_property_long(tui_key_ce, Z_OBJ(key_obj), "functionKey", sizeof("functionKey")-1, key.functionKey);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "ctrl", sizeof("ctrl")-1, key.ctrl);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "meta", sizeof("meta")-1, key.meta);
        zend_update_property_bool(tui_key_ce, Z_OBJ(key_obj), "shift", sizeof("shift")-1, key.shift);

        /* Set name property for special keys */
        const char *name = "";
        char fname[5]; /* Buffer for "F1" through "F12" (max 4 chars + null) */
        if (key.upArrow) name = "upArrow";
        else if (key.downArrow) name = "downArrow";
        else if (key.leftArrow) name = "leftArrow";
        else if (key.rightArrow) name = "rightArrow";
        else if (key.enter) name = "return";
        else if (key.escape) name = "escape";
        else if (key.backspace) name = "backspace";
        else if (key.delete) name = "delete";
        else if (key.tab) name = "tab";
        else if (key.home) name = "home";
        else if (key.end) name = "end";
        else if (key.pageUp) name = "pageUp";
        else if (key.pageDown) name = "pageDown";
        else if (key.functionKey >= 1 && key.functionKey <= 12) {
            snprintf(fname, sizeof(fname), "F%d", key.functionKey);
            name = fname;
        }
        zend_update_property_string(tui_key_ce, Z_OBJ(key_obj), "name", sizeof("name")-1, name);

        /* Call the PHP handler */
        zval params[1];
        ZVAL_COPY_VALUE(&params[0], &key_obj);

        app->input_fci.param_count = 1;
        app->input_fci.params = params;
        app->input_fci.retval = &retval;

        if (zend_call_function(&app->input_fci, &app->input_fcc) == SUCCESS) {
            zval_ptr_dtor(&retval);
        }

        zval_ptr_dtor(&key_obj);

        /* Discard any captured output */
        php_output_discard();
    }

    /* Note: We don't set render_pending here because:
     * 1. If the PHP handler needs a re-render, it calls tui_rerender() directly
     * 2. If focus changed (Tab key), tui_app_focus_next/prev already set render_pending
     * Setting it unconditionally here causes double renders when PHP calls tui_rerender()
     */
}

void tui_app_on_resize(int width, int height, void *userdata)
{
    tui_app *app = (tui_app *)userdata;
    if (!app) return;

    app->width = width;
    app->height = height;

    /* Resize buffers */
    tui_buffer_resize(app->buffer, width, height);

    /* Call PHP resize handler if set */
    if (app->has_resize_handler) {
        /* Capture any output during callback to prevent terminal corruption */
        php_output_start_default();

        zval retval;
        zval params[2];

        ZVAL_LONG(&params[0], width);
        ZVAL_LONG(&params[1], height);

        app->resize_fci.param_count = 2;
        app->resize_fci.params = params;
        app->resize_fci.retval = &retval;

        if (zend_call_function(&app->resize_fci, &app->resize_fcc) == SUCCESS) {
            zval_ptr_dtor(&retval);
        }

        /* Discard any captured output */
        php_output_discard();
    }

    /* Request FULL re-render (call component callback to rebuild tree with new size) */
    app->rerender_pending = 1;
}

/* Border characters for different styles.
 * Index: [style][char] where char is: 0=TL 1=TR 2=BL 3=BR 4=H 5=V
 * Shared by both app and external rendering functions. */
static const char* border_chars[6][6] = {
    /* NONE: no borders */
    {"", "", "", "", "", ""},
    /* SINGLE: ┌ ┐ └ ┘ ─ │ */
    {"┌", "┐", "└", "┘", "─", "│"},
    /* DOUBLE: ╔ ╗ ╚ ╝ ═ ║ */
    {"╔", "╗", "╚", "╝", "═", "║"},
    /* ROUND: ╭ ╮ ╰ ╯ ─ │ */
    {"╭", "╮", "╰", "╯", "─", "│"},
    /* BOLD: ┏ ┓ ┗ ┛ ━ ┃ */
    {"┏", "┓", "┗", "┛", "━", "┃"},
    /* DASHED: ┌ ┐ └ ┘ ┄ ┆ */
    {"┌", "┐", "└", "┘", "┄", "┆"}
};

/* Render border around a box to any buffer.
 * Single implementation used by both app rendering and external buffer rendering. */
static void render_border_to_buffer(tui_buffer *buffer, tui_node *node, int x, int y, int w, int h)
{
    if (!buffer || !node || node->border_style == TUI_BORDER_NONE || w < 2 || h < 2) return;

    const char **chars = border_chars[node->border_style];
    tui_style border_style = {0};
    if (node->border_color.is_set) {
        border_style.fg = node->border_color;
    }

    /* Corners: TL, TR, BL, BR */
    tui_buffer_write_text(buffer, x, y, chars[0], &border_style);
    tui_buffer_write_text(buffer, x + w - 1, y, chars[1], &border_style);
    tui_buffer_write_text(buffer, x, y + h - 1, chars[2], &border_style);
    tui_buffer_write_text(buffer, x + w - 1, y + h - 1, chars[3], &border_style);

    /* Horizontal edges (top and bottom) */
    for (int i = 1; i < w - 1; i++) {
        tui_buffer_write_text(buffer, x + i, y, chars[4], &border_style);
        tui_buffer_write_text(buffer, x + i, y + h - 1, chars[4], &border_style);
    }

    /* Vertical edges (left and right) */
    for (int i = 1; i < h - 1; i++) {
        tui_buffer_write_text(buffer, x, y + i, chars[5], &border_style);
        tui_buffer_write_text(buffer, x + w - 1, y + i, chars[5], &border_style);
    }
}

/* Forward declaration */
#include "../text/wrap.h"

/* Render wrapped text */
static void render_wrapped_text(tui_app *app, tui_node *node, int x, int y, int max_width, int max_height)
{
    if (!node || !node->text || !node->text[0]) return;

    if (max_width <= 0 || max_height <= 0) {
        return;
    }

    switch (node->wrap_mode) {
        case TUI_WRAP_NONE:
            /* Truncate to fit width */
            {
                char *truncated = tui_truncate_text(node->text, max_width, "…");
                if (truncated) {
                    tui_buffer_write_text(app->buffer, x, y, truncated, &node->style);
                    free(truncated);
                }
            }
            break;

        case TUI_WRAP_CHAR:
        case TUI_WRAP_WORD:
        case TUI_WRAP_WORD_CHAR:
            /* Word or character wrap */
            {
                tui_wrapped_text *wrapped = tui_wrap_text(node->text, max_width, node->wrap_mode);
                if (wrapped) {
                    int lines_to_render = wrapped->count < max_height ? wrapped->count : max_height;
                    for (int i = 0; i < lines_to_render; i++) {
                        tui_buffer_write_text(app->buffer, x, y + i, wrapped->lines[i], &node->style);
                    }
                    tui_wrapped_text_free(wrapped);
                }
            }
            break;
    }
}

/* Render a node tree to the buffer */
static void render_node_to_buffer(tui_app *app, tui_node *node, int offset_x, int offset_y)
{
    if (!app || !node) return;

    /* Calculate absolute position */
    int x = offset_x + (int)node->x;
    int y = offset_y + (int)node->y;
    int w = (int)node->width;
    int h = (int)node->height;

    /* Render based on node type */
    if (node->type == TUI_NODE_TEXT && node->text) {
        /* Render text content with wrapping support */
        render_wrapped_text(app, node, x, y, w, h);
    } else if (node->type == TUI_NODE_BOX) {
        /* Fill background if set */
        if (node->style.bg.is_set) {
            tui_buffer_fill_rect(app->buffer, x, y, w, h, ' ', &node->style);
        }

        /* Render border if set */
        if (node->border_style != TUI_BORDER_NONE) {
            render_border_to_buffer(app->buffer, node, x, y, w, h);
        }
    }

    /* Render children */
    for (int i = 0; i < node->child_count; i++) {
        render_node_to_buffer(app, node->children[i], x, y);
    }
}

/* ------------------------------------------------------------------
 * useState hook state management
 * ------------------------------------------------------------------ */

void tui_app_reset_state_index(tui_app *app)
{
    if (app) {
        app->state_index = 0;
    }
}

void tui_app_cleanup_states(tui_app *app)
{
    if (!app) return;

    for (int i = 0; i < app->state_count; i++) {
        if (!Z_ISUNDEF(app->states[i].value)) {
            zval_ptr_dtor(&app->states[i].value);
            ZVAL_UNDEF(&app->states[i].value);
        }
        if (!Z_ISUNDEF(app->states[i].setter)) {
            zval_ptr_dtor(&app->states[i].setter);
            ZVAL_UNDEF(&app->states[i].setter);
        }
    }
    app->state_count = 0;
    app->state_index = 0;
}

/**
 * Ensure states array has room for at least one more slot.
 * Doubles capacity on growth with overflow protection.
 * @return 0 on success, -1 on failure
 */
static int ensure_state_capacity(tui_app *app)
{
    if (app->state_count < app->state_capacity) {
        return 0;  /* Already have room */
    }

    /* Check INI limit */
    zend_long max_states = TUI_G(max_states);
    if (app->state_count >= (int)max_states) {
        php_error_docref(NULL, E_WARNING,
            "Maximum number of useState hooks (%d) exceeded. "
            "Increase tui.max_states in php.ini",
            (int)max_states);
        return -1;
    }

    /* Check for overflow before doubling */
    if (app->state_capacity > INT_MAX / 2) return -1;
    int new_capacity = app->state_capacity * 2;

    /* Respect INI limit */
    if (new_capacity > (int)max_states) {
        new_capacity = (int)max_states;
    }

    /* Check for size_t overflow */
    if ((size_t)new_capacity > SIZE_MAX / sizeof(tui_state_slot)) return -1;

    tui_state_slot *new_states = realloc(app->states,
        (size_t)new_capacity * sizeof(tui_state_slot));
    if (!new_states) return -1;

    /* Zero-initialize new slots */
    memset(&new_states[app->state_capacity], 0,
        (new_capacity - app->state_capacity) * sizeof(tui_state_slot));

    app->states = new_states;
    app->state_capacity = new_capacity;
    return 0;
}

/* Note: The actual useState implementation with closure creation is in tui.c
 * because it requires access to the PHP class entries and closure mechanisms.
 * This function provides the C-level state slot management. */

int tui_app_get_or_create_state_slot(tui_app *app, zval *initial, int *is_new)
{
    if (!app) return -1;

    int index = app->state_index++;

    /* Ensure capacity before accessing (handles INI limit check internally) */
    if (index >= app->state_capacity) {
        if (ensure_state_capacity(app) < 0) {
            return -1;
        }
    }

    if (index >= app->state_count) {
        /* First render - initialize this state slot */
        app->state_count = index + 1;
        ZVAL_COPY(&app->states[index].value, initial);
        ZVAL_UNDEF(&app->states[index].setter);  /* Will be set by caller */
        app->states[index].index = index;
        *is_new = 1;
    } else {
        *is_new = 0;
    }

    return index;
}

/* ------------------------------------------------------------------
 * Focus by ID
 * ------------------------------------------------------------------ */

/* Helper: find node by ID in tree */
static tui_node* find_node_by_id(tui_node *node, const char *id)
{
    if (!node || !id) return NULL;

    /* Check if this node matches */
    if (node->id && strcmp(node->id, id) == 0) {
        return node;
    }

    /* Search children */
    for (int i = 0; i < node->child_count; i++) {
        tui_node *found = find_node_by_id(node->children[i], id);
        if (found) return found;
    }

    return NULL;
}

int tui_app_focus_by_id(tui_app *app, const char *id)
{
    if (!app || !id || !app->root_node) return 0;

    tui_node *node = find_node_by_id(app->root_node, id);
    if (node && node->focusable) {
        tui_app_set_focus(app, node);
        return 1;
    }

    return 0;
}

void tui_app_enable_focus(tui_app *app)
{
    if (app) {
        app->focus_enabled = 1;
    }
}

void tui_app_disable_focus(tui_app *app)
{
    if (app) {
        app->focus_enabled = 0;
        /* Optionally clear current focus */
        if (app->focused_node) {
            app->focused_node->focused = 0;
            app->focused_node = NULL;
        }
    }
}

/* ------------------------------------------------------------------
 * Testing support functions
 * ------------------------------------------------------------------ */

void tui_app_inject_input(tui_app *app, const char *input, int len)
{
    if (!app || !input || len <= 0) return;

    /* Call the input handler directly, simulating input without polling */
    tui_app_on_input(input, len, app);
}

/* Render node tree to external buffer (for headless testing) */
static void render_node_to_buffer_external(tui_buffer *buffer, tui_node *node, int offset_x, int offset_y);

/* Render wrapped text to external buffer */
static void render_wrapped_text_external(tui_buffer *buffer, tui_node *node, int x, int y, int max_width, int max_height)
{
    if (!node || !node->text || !node->text[0]) return;

    if (max_width <= 0 || max_height <= 0) {
        return;
    }

    switch (node->wrap_mode) {
        case TUI_WRAP_NONE:
            {
                char *truncated = tui_truncate_text(node->text, max_width, "…");
                if (truncated) {
                    tui_buffer_write_text(buffer, x, y, truncated, &node->style);
                    free(truncated);
                }
            }
            break;

        case TUI_WRAP_CHAR:
        case TUI_WRAP_WORD:
        case TUI_WRAP_WORD_CHAR:
            {
                tui_wrapped_text *wrapped = tui_wrap_text(node->text, max_width, node->wrap_mode);
                if (wrapped) {
                    int lines_to_render = wrapped->count < max_height ? wrapped->count : max_height;
                    for (int i = 0; i < lines_to_render; i++) {
                        tui_buffer_write_text(buffer, x, y + i, wrapped->lines[i], &node->style);
                    }
                    tui_wrapped_text_free(wrapped);
                }
            }
            break;
    }
}

static void render_node_to_buffer_external(tui_buffer *buffer, tui_node *node, int offset_x, int offset_y)
{
    if (!buffer || !node) return;

    int x = offset_x + (int)node->x;
    int y = offset_y + (int)node->y;
    int w = (int)node->width;
    int h = (int)node->height;

    if (node->type == TUI_NODE_TEXT && node->text) {
        render_wrapped_text_external(buffer, node, x, y, w, h);
    } else if (node->type == TUI_NODE_BOX) {
        if (node->style.bg.is_set) {
            tui_buffer_fill_rect(buffer, x, y, w, h, ' ', &node->style);
        }
        if (node->border_style != TUI_BORDER_NONE) {
            render_border_to_buffer(buffer, node, x, y, w, h);
        }
    }

    for (int i = 0; i < node->child_count; i++) {
        render_node_to_buffer_external(buffer, node->children[i], x, y);
    }
}

void tui_app_render_node_to_buffer(tui_buffer *buffer, tui_node *node,
                                    int offset_x, int offset_y,
                                    int clip_x, int clip_y,
                                    int clip_w, int clip_h)
{
    /* clip parameters reserved for future use */
    (void)clip_x;
    (void)clip_y;
    (void)clip_w;
    (void)clip_h;

    render_node_to_buffer_external(buffer, node, offset_x, offset_y);
}
