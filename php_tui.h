/*
  +----------------------------------------------------------------------+
  | ext-tui: Terminal UI extension for PHP                              |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#ifndef PHP_TUI_H
#define PHP_TUI_H

extern zend_module_entry tui_module_entry;
#define phpext_tui_ptr &tui_module_entry

#define PHP_TUI_VERSION "0.1.0"

#ifdef PHP_WIN32
#   define PHP_TUI_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_TUI_API __attribute__ ((visibility("default")))
#else
#   define PHP_TUI_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#include <yoga/Yoga.h>

/* Module globals */
ZEND_BEGIN_MODULE_GLOBALS(tui)
    zend_bool raw_mode_active;
    int original_termios_saved;
    int terminal_width;
    int terminal_height;
    YGConfigRef yoga_config;

    /* Configurable limits (from INI) */
    zend_long max_buffer_width;
    zend_long max_buffer_height;
    zend_long max_tree_depth;
    zend_long max_states;
    zend_long max_timers;
    zend_long min_render_interval;
ZEND_END_MODULE_GLOBALS(tui)

#ifdef ZTS
#define TUI_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(tui, v)
#else
#define TUI_G(v) (tui_globals.v)
#endif

/* Class entries */
extern zend_class_entry *tui_box_ce;
extern zend_class_entry *tui_text_ce;
extern zend_class_entry *tui_instance_ce;
extern zend_class_entry *tui_key_ce;
extern zend_class_entry *tui_color_ce;

/* Function declarations */
PHP_FUNCTION(tui_render);
PHP_FUNCTION(tui_rerender);
PHP_FUNCTION(tui_unmount);
PHP_FUNCTION(tui_wait_until_exit);
PHP_FUNCTION(tui_get_terminal_size);
PHP_FUNCTION(tui_is_interactive);
PHP_FUNCTION(tui_is_ci);
PHP_FUNCTION(tui_string_width);
PHP_FUNCTION(tui_wrap_text);
PHP_FUNCTION(tui_truncate);

#endif /* PHP_TUI_H */
