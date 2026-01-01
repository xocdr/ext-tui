/*
  +----------------------------------------------------------------------+
  | ext-tui: Function argument info definitions                         |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
  |                                                                      |
  | This header contains arginfo definitions for all global TUI          |
  | functions. These define function signatures for PHP reflection and   |
  | type checking.                                                       |
  |                                                                      |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_ARGINFO_H
#define TUI_ARGINFO_H

/* Terminal functions arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_terminal_size, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_is_interactive, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_is_ci, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

/* Cursor shape functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_cursor_shape, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, shape, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_cursor_show, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_cursor_hide, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

/* Window title functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_set_title, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, title, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_reset_title, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

/* Capability functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_capabilities, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_has_capability, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

/* Notification functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_bell, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_flash, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_notify, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, title, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, body, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, priority, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

/* Mouse functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_mouse_enable, 0, 0, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "2")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_mouse_disable, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_mouse_get_mode, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

/* Bracketed paste functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_bracketed_paste_enable, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_bracketed_paste_disable, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_bracketed_paste_is_enabled, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

/* Clipboard functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_clipboard_copy, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, target, IS_STRING, 0, "\"clipboard\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_clipboard_request, 0, 0, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, target, IS_STRING, 0, "\"clipboard\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_clipboard_clear, 0, 0, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, target, IS_STRING, 0, "\"clipboard\"")
ZEND_END_ARG_INFO()

/* History functions */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_history_create, 0, 0, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, maxEntries, IS_LONG, 0, "1000")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_history_destroy, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, history)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_history_add, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, history)
    ZEND_ARG_TYPE_INFO(0, entry, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_history_prev, 0, 1, IS_STRING, 1)
    ZEND_ARG_INFO(0, history)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_history_next, 0, 1, IS_STRING, 1)
    ZEND_ARG_INFO(0, history)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_history_reset, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, history)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_history_save_temp, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, history)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_history_get_temp, 0, 1, IS_STRING, 1)
    ZEND_ARG_INFO(0, history)
ZEND_END_ARG_INFO()

/* Input parsing functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_parse_mouse, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_parse_key, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()

/* Text utility functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_string_width, 0, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_wrap_text, 0, 2, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_truncate, 0, 2, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, ellipsis, IS_STRING, 0, "\"...\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, position, IS_STRING, 0, "\"end\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_pad, 0, 2, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, align, IS_STRING, 0, "\"l\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pad_char, IS_STRING, 0, "\" \"")
ZEND_END_ARG_INFO()

/* ANSI utility functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_strip_ansi, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_string_width_ansi, 0, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_slice_ansi, 0, 3, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, start, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, end, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_grapheme_count, 0, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

/* Render/lifecycle functions */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_render, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, component, IS_CALLABLE, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_rerender, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_unmount, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_wait_until_exit, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_set_input_handler, 0, 2, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_set_focus_handler, 0, 2, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_focus_next, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_focus_prev, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_set_resize_handler, 0, 2, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_size, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_focused_node, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
ZEND_END_ARG_INFO()

/* Timer functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_add_timer, 0, 3, IS_LONG, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
    ZEND_ARG_TYPE_INFO(0, interval_ms, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_remove_timer, 0, 2, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
    ZEND_ARG_TYPE_INFO(0, timer_id, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_set_tick_handler, 0, 2, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

/* Buffer functions */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_buffer_create, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_buffer_clear, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_buffer_render, 0, 1, IS_STRING, 0)
    ZEND_ARG_INFO(0, buffer)
ZEND_END_ARG_INFO()

/* Drawing primitives functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_draw_line, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, x2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_draw_rect, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, border, IS_STRING, 1, "\"single\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_fill_rect, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "\" \"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_draw_circle, 0, 4, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, cx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, cy, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, radius, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_fill_circle, 0, 4, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, cx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, cy, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, radius, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_draw_ellipse, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, cx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, cy, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, rx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, ry, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_fill_ellipse, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, cx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, cy, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, rx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, ry, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_draw_triangle, 0, 7, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, x2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, x3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_fill_triangle, 0, 7, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, x2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, x3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

/* Canvas functions */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_canvas_create, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_STRING, 1, "\"braille\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_set, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_unset, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_toggle, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_get, 0, 3, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_clear, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_line, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, x1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, x2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y2, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_rect, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_fill_rect, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_circle, 0, 4, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, cx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, cy, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, radius, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_fill_circle, 0, 4, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, cx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, cy, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, radius, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_set_color, 0, 4, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, r, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, g, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, b, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_get_resolution, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_INFO(0, canvas)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_render, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_INFO(0, canvas)
ZEND_END_ARG_INFO()

/* Animation functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_ease, 0, 1, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, t, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, easing, IS_STRING, 1, "\"linear\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_lerp, 0, 3, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, a, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, b, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, t, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_lerp_color, 0, 3, IS_STRING, 0)
    ZEND_ARG_INFO(0, a)
    ZEND_ARG_INFO(0, b)
    ZEND_ARG_TYPE_INFO(0, t, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_color_from_hex, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, hex, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_gradient, 0, 2, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, colors, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, steps, IS_LONG, 0)
ZEND_END_ARG_INFO()

/* Table functions */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_table_create, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, headers, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_table_add_row, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_TYPE_INFO(0, cells, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_table_set_align, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, right_align, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_table_render_to_buffer, 0, 4, IS_LONG, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, border, IS_STRING, 1, "\"single\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, header_style, IS_ARRAY, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cell_style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

/* Progress/Spinner functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_render_progress_bar, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, progress, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_render_busy_bar, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, frame, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style_name, IS_STRING, 1, "\"pulse\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_spinner_frame, 0, 2, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, frame, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_spinner_frame_count, 0, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_render_spinner, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, frame, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

/* Sprite functions */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_sprite_create, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, frames, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, name, IS_STRING, 1, "\"default\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, loop, _IS_BOOL, 0, "true")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_update, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, sprite)
    ZEND_ARG_TYPE_INFO(0, delta_ms, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_set_animation, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, sprite)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_set_position, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, sprite)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_flip, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, sprite)
    ZEND_ARG_TYPE_INFO(0, flipped, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_set_visible, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, sprite)
    ZEND_ARG_TYPE_INFO(0, visible, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_render, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_INFO(0, sprite)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_get_bounds, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_INFO(0, sprite)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_collides, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, a)
    ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()

/* Testing framework functions */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_test_create, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_destroy, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, renderer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_render, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_OBJ_INFO(0, element, TuiBox, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_get_output, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_INFO(0, renderer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_to_string, 0, 1, IS_STRING, 0)
    ZEND_ARG_INFO(0, renderer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_send_input, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_send_key, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_TYPE_INFO(0, keyCode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_advance_frame, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, renderer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_run_timers, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_TYPE_INFO(0, ms, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_test_get_by_id, 0, 0, 2)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_TYPE_INFO(0, id, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_get_by_text, 0, 2, IS_ARRAY, 0)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

/* Metrics functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_metrics_enable, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_metrics_disable, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_metrics_enabled, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_metrics_reset, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_metrics, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_node_metrics, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_reconciler_metrics, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_render_metrics, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_loop_metrics, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_pool_metrics, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

/* Virtual list functions */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_virtual_create, 0, 0, 3)
    ZEND_ARG_TYPE_INFO(0, item_count, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, item_height, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, viewport_height, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, overscan, IS_LONG, 0, "5")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_get_range, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_INFO(0, vlist)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_scroll_to, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_scroll_by, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, delta, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_scroll_items, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, items, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_ensure_visible, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_page_up, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_page_down, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_scroll_top, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_scroll_bottom, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_set_count, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, item_count, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_set_viewport, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, viewport_height, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_item_offset, 0, 2, IS_LONG, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_is_visible, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_destroy, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
ZEND_END_ARG_INFO()

/* Smooth scrolling functions */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_scroll_create, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_set_spring, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, anim)
    ZEND_ARG_TYPE_INFO(0, stiffness, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, damping, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_set_target, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, anim)
    ZEND_ARG_TYPE_INFO(0, x, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_by, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, anim)
    ZEND_ARG_TYPE_INFO(0, dx, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, dy, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_update, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, anim)
    ZEND_ARG_TYPE_INFO(0, dt, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_snap, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, anim)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_get_position, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_INFO(0, anim)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_is_animating, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, anim)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_progress, 0, 1, IS_DOUBLE, 0)
    ZEND_ARG_INFO(0, anim)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_destroy, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, anim)
ZEND_END_ARG_INFO()

/* Graphics functions */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_image_load, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_image_create, 0, 0, 3)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, format, IS_STRING, 0, "\"rgba\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_image_transmit, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, image)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_image_display, 0, 3, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, image)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cols, IS_LONG, 0, "0")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, rows, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_image_delete, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, image)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_image_clear, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_image_destroy, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, image)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_image_get_info, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_INFO(0, image)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_graphics_supported, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_graphics_protocol, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_iterm2_supported, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sixel_supported, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

/* Recording functions */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_record_create, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, title, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_start, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_pause, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_resume, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_stop, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_capture, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, recording)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_duration, 0, 1, IS_DOUBLE, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_frame_count, 0, 1, IS_LONG, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_export, 0, 1, IS_STRING, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_save, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, recording)
    ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_destroy, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

/* Accessibility functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_announce, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, message, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, priority, IS_STRING, 0, "\"polite\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_prefers_reduced_motion, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_prefers_high_contrast, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_accessibility_features, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_aria_role_to_string, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, role, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_aria_role_from_string, 0, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, role, IS_STRING, 0)
ZEND_END_ARG_INFO()

/* Drag and drop functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_start, 0, 4, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_move, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_end, 0, 0, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dropped, _IS_BOOL, 0, "true")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_cancel, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_is_active, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_get_type, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_get_data, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_get_state, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

#endif /* TUI_ARGINFO_H */
