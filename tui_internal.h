/*
  +----------------------------------------------------------------------+
  | ext-tui: Internal header for split modules                          |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_INTERNAL_H
#define TUI_INTERNAL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_exceptions.h"
#include "zend_enum.h"
#include "php_tui.h"

#include "src/text/measure.h"
#include "src/text/wrap.h"
#include "src/app/app.h"
#include "src/node/node.h"
#include "src/terminal/terminal.h"
#include "src/terminal/ansi.h"
#include "src/event/input.h"
#include "src/drawing/primitives.h"
#include "src/drawing/canvas.h"
#include "src/drawing/animation.h"
#include "src/drawing/table.h"
#include "src/drawing/progress.h"
#include "src/drawing/sprite.h"
#include "src/render/buffer.h"
#include "src/testing/renderer.h"
#include "src/testing/query.h"
#include "src/pool/pool.h"

#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <math.h>

/* Maximum depth for recursive tree operations */
#define MAX_TREE_DEPTH 256

/* Resource names */
#define TUI_CANVAS_RES_NAME "TuiCanvas"
#define TUI_TABLE_RES_NAME "TuiTable"
#define TUI_SPRITE_RES_NAME "TuiSprite"
#define TUI_BUFFER_RES_NAME "TuiBuffer"
#define TUI_TEST_RENDERER_RES_NAME "TuiTestRenderer"
#define TUI_HISTORY_RES_NAME "TuiHistory"
#define TUI_VIRTUAL_LIST_RES_NAME "TuiVirtualList"
#define TUI_SCROLL_ANIM_RES_NAME "TuiScrollAnimation"
#define TUI_IMAGE_RES_NAME "TuiImage"

/* ----------------------------------------------------------------
 * Shared class entries
 * ---------------------------------------------------------------- */
extern zend_class_entry *tui_box_ce;
extern zend_class_entry *tui_text_ce;
extern zend_class_entry *tui_instance_ce;
extern zend_class_entry *tui_key_ce;
extern zend_class_entry *tui_focus_event_ce;
extern zend_class_entry *tui_focus_ce;
extern zend_class_entry *tui_focus_manager_ce;
extern zend_class_entry *tui_stdin_context_ce;
extern zend_class_entry *tui_stdout_context_ce;
extern zend_class_entry *tui_stderr_context_ce;
extern zend_class_entry *tui_newline_ce;
extern zend_class_entry *tui_spacer_ce;
extern zend_class_entry *tui_transform_ce;
extern zend_class_entry *tui_static_ce;
extern zend_class_entry *tui_color_ce;

/* ----------------------------------------------------------------
 * Shared resource type IDs
 * ---------------------------------------------------------------- */
extern int le_tui_canvas;
extern int le_tui_table;
extern int le_tui_sprite;
extern int le_tui_buffer;
extern int le_tui_test_renderer;
extern int le_tui_history;
extern int le_tui_virtual_list;
extern int le_tui_scroll_animation;
extern int le_tui_image;

/* ----------------------------------------------------------------
 * TuiInstance custom object structure
 * ---------------------------------------------------------------- */
typedef struct {
    tui_app *app;
    zend_object std;
} tui_instance_object;

/* Helper to get instance from zend_object */
static inline tui_instance_object *tui_instance_from_obj(zend_object *obj) {
    return (tui_instance_object *)((char *)(obj) - XtOffsetOf(tui_instance_object, std));
}

#define Z_TUI_INSTANCE_P(zv) tui_instance_from_obj(Z_OBJ_P(zv))

/* ----------------------------------------------------------------
 * TuiFocus custom object structure
 * ---------------------------------------------------------------- */
typedef struct {
    tui_app *app;
    zend_object std;
} tui_focus_object;

/* Helper to get focus from zend_object */
static inline tui_focus_object *tui_focus_from_obj(zend_object *obj) {
    return (tui_focus_object *)((char *)(obj) - XtOffsetOf(tui_focus_object, std));
}

#define Z_TUI_FOCUS_P(zv) tui_focus_from_obj(Z_OBJ_P(zv))

/* ----------------------------------------------------------------
 * TuiFocusManager custom object structure
 * ---------------------------------------------------------------- */
typedef struct {
    tui_app *app;
    zend_object std;
} tui_focus_manager_object;

/* Helper to get focus manager from zend_object */
static inline tui_focus_manager_object *tui_focus_manager_from_obj(zend_object *obj) {
    return (tui_focus_manager_object *)((char *)(obj) - XtOffsetOf(tui_focus_manager_object, std));
}

#define Z_TUI_FOCUS_MANAGER_P(zv) tui_focus_manager_from_obj(Z_OBJ_P(zv))

/* ----------------------------------------------------------------
 * Named Color lookup (used by Color enum and php_to_tui_node)
 * ---------------------------------------------------------------- */
typedef struct {
    const char *name;
    uint8_t r, g, b;
} named_color;

extern const named_color named_colors[];

/* Convert CSS color name to PascalCase for enum lookup */
void named_color_to_camel(const char *name, char *out, size_t out_size);

/* Lookup color by name in named_colors table */
int lookup_named_color(const char *name, tui_color *color);

/* ----------------------------------------------------------------
 * Class method tables (defined in tui_classes.c)
 * ---------------------------------------------------------------- */
extern const zend_function_entry tui_color_methods[];
extern const zend_function_entry tui_node_interface_methods[];
extern const zend_function_entry tui_box_methods[];
extern const zend_function_entry tui_text_methods[];
extern const zend_function_entry tui_instance_methods[];
extern const zend_function_entry tui_focus_methods[];
extern const zend_function_entry tui_focus_manager_methods[];
extern const zend_function_entry tui_stdin_context_methods[];
extern const zend_function_entry tui_stdout_context_methods[];
extern const zend_function_entry tui_stderr_context_methods[];

/* ----------------------------------------------------------------
 * Object handlers (defined in tui.c)
 * ---------------------------------------------------------------- */
extern zend_object_handlers tui_instance_handlers;
extern zend_object_handlers tui_focus_handlers;
extern zend_object_handlers tui_focus_manager_handlers;

/* Object creation functions */
zend_object *tui_instance_create_object(zend_class_entry *ce);
zend_object *tui_focus_create_object(zend_class_entry *ce);
zend_object *tui_focus_manager_create_object(zend_class_entry *ce);

/* ----------------------------------------------------------------
 * Shared utility functions
 * ---------------------------------------------------------------- */

/* Parse PHP array/object to tui_node tree */
tui_node* php_to_tui_node(zval *znode, int depth);

/* Parse style array to tui_style */
void parse_style_array(zval *style_arr, tui_style *style);

/* Register all resources, classes, and constants */
void tui_register_resources(int module_number);
void tui_register_classes(void);
void tui_register_constants(int module_number);

/* ----------------------------------------------------------------
 * PHP Function declarations (for function table in tui.c)
 * ---------------------------------------------------------------- */

/* Terminal functions (tui_terminal.c) */
PHP_FUNCTION(tui_get_terminal_size);
PHP_FUNCTION(tui_is_interactive);
PHP_FUNCTION(tui_is_ci);
PHP_FUNCTION(tui_cursor_shape);
PHP_FUNCTION(tui_cursor_show);
PHP_FUNCTION(tui_cursor_hide);
PHP_FUNCTION(tui_set_title);
PHP_FUNCTION(tui_reset_title);
PHP_FUNCTION(tui_get_capabilities);
PHP_FUNCTION(tui_has_capability);
PHP_FUNCTION(tui_bell);
PHP_FUNCTION(tui_flash);
PHP_FUNCTION(tui_notify);
PHP_FUNCTION(tui_mouse_enable);
PHP_FUNCTION(tui_mouse_disable);
PHP_FUNCTION(tui_mouse_get_mode);
PHP_FUNCTION(tui_bracketed_paste_enable);
PHP_FUNCTION(tui_bracketed_paste_disable);
PHP_FUNCTION(tui_bracketed_paste_is_enabled);
PHP_FUNCTION(tui_clipboard_copy);
PHP_FUNCTION(tui_clipboard_request);
PHP_FUNCTION(tui_clipboard_clear);
PHP_FUNCTION(tui_history_create);
PHP_FUNCTION(tui_history_destroy);
PHP_FUNCTION(tui_history_add);
PHP_FUNCTION(tui_history_prev);
PHP_FUNCTION(tui_history_next);
PHP_FUNCTION(tui_history_reset);
PHP_FUNCTION(tui_history_save_temp);
PHP_FUNCTION(tui_history_get_temp);
PHP_FUNCTION(tui_history_search);
PHP_FUNCTION(tui_history_get_all);
PHP_FUNCTION(tui_history_clear);
PHP_FUNCTION(tui_history_save);

/* Input parsing functions (tui_terminal.c) */
PHP_FUNCTION(tui_parse_mouse);
PHP_FUNCTION(tui_parse_key);

/* Text functions (tui_text.c) */
PHP_FUNCTION(tui_string_width);
PHP_FUNCTION(tui_wrap_text);
PHP_FUNCTION(tui_truncate);
PHP_FUNCTION(tui_pad);
PHP_FUNCTION(tui_strip_ansi);
PHP_FUNCTION(tui_string_width_ansi);
PHP_FUNCTION(tui_slice_ansi);
PHP_FUNCTION(tui_grapheme_count);

/* Buffer functions (tui_buffer.c) */
PHP_FUNCTION(tui_buffer_create);
PHP_FUNCTION(tui_buffer_clear);
PHP_FUNCTION(tui_buffer_render);
PHP_FUNCTION(tui_draw_line);
PHP_FUNCTION(tui_draw_rect);
PHP_FUNCTION(tui_fill_rect);
PHP_FUNCTION(tui_draw_circle);
PHP_FUNCTION(tui_fill_circle);
PHP_FUNCTION(tui_draw_ellipse);
PHP_FUNCTION(tui_fill_ellipse);
PHP_FUNCTION(tui_draw_triangle);
PHP_FUNCTION(tui_fill_triangle);

/* Canvas functions (tui_canvas.c) */
PHP_FUNCTION(tui_canvas_create);
PHP_FUNCTION(tui_canvas_set);
PHP_FUNCTION(tui_canvas_unset);
PHP_FUNCTION(tui_canvas_toggle);
PHP_FUNCTION(tui_canvas_get);
PHP_FUNCTION(tui_canvas_clear);
PHP_FUNCTION(tui_canvas_line);
PHP_FUNCTION(tui_canvas_rect);
PHP_FUNCTION(tui_canvas_fill_rect);
PHP_FUNCTION(tui_canvas_circle);
PHP_FUNCTION(tui_canvas_fill_circle);
PHP_FUNCTION(tui_canvas_set_color);
PHP_FUNCTION(tui_canvas_get_resolution);
PHP_FUNCTION(tui_canvas_render);

/* Animation functions (tui_animation.c) */
PHP_FUNCTION(tui_ease);
PHP_FUNCTION(tui_lerp);
PHP_FUNCTION(tui_lerp_color);
PHP_FUNCTION(tui_color_from_hex);
PHP_FUNCTION(tui_gradient);

/* Table functions (tui_table.c) */
PHP_FUNCTION(tui_table_create);
PHP_FUNCTION(tui_table_add_row);
PHP_FUNCTION(tui_table_set_align);
PHP_FUNCTION(tui_table_render_to_buffer);

/* Progress functions (tui_progress.c) */
PHP_FUNCTION(tui_render_progress_bar);
PHP_FUNCTION(tui_render_busy_bar);
PHP_FUNCTION(tui_spinner_frame);
PHP_FUNCTION(tui_spinner_frame_count);
PHP_FUNCTION(tui_render_spinner);

/* Sprite functions (tui_sprite.c) */
PHP_FUNCTION(tui_sprite_create);
PHP_FUNCTION(tui_sprite_update);
PHP_FUNCTION(tui_sprite_set_animation);
PHP_FUNCTION(tui_sprite_set_position);
PHP_FUNCTION(tui_sprite_flip);
PHP_FUNCTION(tui_sprite_set_visible);
PHP_FUNCTION(tui_sprite_render);
PHP_FUNCTION(tui_sprite_get_bounds);
PHP_FUNCTION(tui_sprite_collides);

/* Render/lifecycle functions (tui_render.c) */
PHP_FUNCTION(tui_render);
PHP_FUNCTION(tui_rerender);
PHP_FUNCTION(tui_unmount);
PHP_FUNCTION(tui_wait_until_exit);
PHP_FUNCTION(tui_set_input_handler);
PHP_FUNCTION(tui_set_focus_handler);
PHP_FUNCTION(tui_focus_next);
PHP_FUNCTION(tui_focus_prev);
PHP_FUNCTION(tui_set_resize_handler);
PHP_FUNCTION(tui_get_size);
PHP_FUNCTION(tui_get_focused_node);
PHP_FUNCTION(tui_add_timer);
PHP_FUNCTION(tui_remove_timer);
PHP_FUNCTION(tui_set_tick_handler);

/* Testing functions (tui_testing.c) */
PHP_FUNCTION(tui_test_create);
PHP_FUNCTION(tui_test_destroy);
PHP_FUNCTION(tui_test_render);
PHP_FUNCTION(tui_test_get_output);
PHP_FUNCTION(tui_test_to_string);
PHP_FUNCTION(tui_test_send_input);
PHP_FUNCTION(tui_test_send_key);
PHP_FUNCTION(tui_test_advance_frame);
PHP_FUNCTION(tui_test_run_timers);
PHP_FUNCTION(tui_test_get_by_id);
PHP_FUNCTION(tui_test_get_by_text);

/* Metrics functions (tui_metrics.c) */
PHP_FUNCTION(tui_metrics_enable);
PHP_FUNCTION(tui_metrics_disable);
PHP_FUNCTION(tui_metrics_enabled);
PHP_FUNCTION(tui_metrics_reset);
PHP_FUNCTION(tui_get_metrics);
PHP_FUNCTION(tui_get_node_metrics);
PHP_FUNCTION(tui_get_reconciler_metrics);
PHP_FUNCTION(tui_get_render_metrics);
PHP_FUNCTION(tui_get_loop_metrics);
PHP_FUNCTION(tui_get_pool_metrics);

/* Virtual list functions (tui_virtual.c) */
PHP_FUNCTION(tui_virtual_create);
PHP_FUNCTION(tui_virtual_get_range);
PHP_FUNCTION(tui_virtual_scroll_to);
PHP_FUNCTION(tui_virtual_scroll_by);
PHP_FUNCTION(tui_virtual_scroll_items);
PHP_FUNCTION(tui_virtual_ensure_visible);
PHP_FUNCTION(tui_virtual_page_up);
PHP_FUNCTION(tui_virtual_page_down);
PHP_FUNCTION(tui_virtual_scroll_top);
PHP_FUNCTION(tui_virtual_scroll_bottom);
PHP_FUNCTION(tui_virtual_set_count);
PHP_FUNCTION(tui_virtual_set_viewport);
PHP_FUNCTION(tui_virtual_item_offset);
PHP_FUNCTION(tui_virtual_is_visible);
PHP_FUNCTION(tui_virtual_destroy);

/* Resource destructor for virtual list */
void tui_virtual_list_dtor(zend_resource *res);

/* Smooth scrolling functions (tui_scroll.c) */
PHP_FUNCTION(tui_scroll_create);
PHP_FUNCTION(tui_scroll_set_spring);
PHP_FUNCTION(tui_scroll_set_target);
PHP_FUNCTION(tui_scroll_by);
PHP_FUNCTION(tui_scroll_update);
PHP_FUNCTION(tui_scroll_snap);
PHP_FUNCTION(tui_scroll_get_position);
PHP_FUNCTION(tui_scroll_is_animating);
PHP_FUNCTION(tui_scroll_progress);
PHP_FUNCTION(tui_scroll_destroy);

/* Resource destructor for scroll animation */
void tui_scroll_animation_dtor(zend_resource *res);

/* Graphics functions (tui_graphics.c) */
PHP_FUNCTION(tui_image_load);
PHP_FUNCTION(tui_image_create);
PHP_FUNCTION(tui_image_transmit);
PHP_FUNCTION(tui_image_display);
PHP_FUNCTION(tui_image_delete);
PHP_FUNCTION(tui_image_clear);
PHP_FUNCTION(tui_image_destroy);
PHP_FUNCTION(tui_image_get_info);
PHP_FUNCTION(tui_graphics_supported);
PHP_FUNCTION(tui_graphics_protocol);
PHP_FUNCTION(tui_iterm2_supported);
PHP_FUNCTION(tui_sixel_supported);

/* Resource destructor for image */
void tui_image_dtor(zend_resource *res);

/* Recording functions (tui_recording.c) */
PHP_FUNCTION(tui_record_create);
PHP_FUNCTION(tui_record_start);
PHP_FUNCTION(tui_record_pause);
PHP_FUNCTION(tui_record_resume);
PHP_FUNCTION(tui_record_stop);
PHP_FUNCTION(tui_record_capture);
PHP_FUNCTION(tui_record_duration);
PHP_FUNCTION(tui_record_frame_count);
PHP_FUNCTION(tui_record_export);
PHP_FUNCTION(tui_record_save);
PHP_FUNCTION(tui_record_destroy);

/* Resource destructor for recording */
void tui_recording_dtor(zend_resource *res);
extern int le_tui_recording;

/* Accessibility functions (tui_a11y.c) */
PHP_FUNCTION(tui_announce);
PHP_FUNCTION(tui_prefers_reduced_motion);
PHP_FUNCTION(tui_prefers_high_contrast);
PHP_FUNCTION(tui_get_accessibility_features);
PHP_FUNCTION(tui_aria_role_to_string);
PHP_FUNCTION(tui_aria_role_from_string);

/* Drag and drop functions (tui_dragdrop.c) */
PHP_FUNCTION(tui_drag_start);
PHP_FUNCTION(tui_drag_move);
PHP_FUNCTION(tui_drag_end);
PHP_FUNCTION(tui_drag_cancel);
PHP_FUNCTION(tui_drag_is_active);
PHP_FUNCTION(tui_drag_get_type);
PHP_FUNCTION(tui_drag_get_data);
PHP_FUNCTION(tui_drag_get_state);

#endif /* TUI_INTERNAL_H */
