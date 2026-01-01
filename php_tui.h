/*
  +----------------------------------------------------------------------+
  | ext-tui: Terminal UI extension for PHP                              |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
  | Thread Safety (ZTS):                                                 |
  | This extension uses ZEND_BEGIN_MODULE_GLOBALS for per-thread state. |
  | Terminal operations (raw mode, output) are NOT thread-safe and      |
  | should only be used from the main thread. The TUI application model |
  | assumes single-threaded terminal access. In ZTS builds:             |
  | - Module globals are thread-local via TUI_G() macro                 |
  | - Terminal state (termios) is process-global (use main thread only) |
  | - Yoga layout nodes are per-request, not shared between threads     |
  | - Signal handlers (SIGWINCH) affect the entire process              |
  +----------------------------------------------------------------------+
  | Error Return Conventions:                                            |
  | - Functions returning int: return -1 on error, 0 or positive on ok  |
  | - Functions returning pointers: return NULL on error                 |
  | - Append/add functions: return -1 on error, 0 on success            |
  | - Create functions: return NULL on error, valid pointer on success  |
  | - Width/size functions: return 0 for empty input, -1 never          |
  +----------------------------------------------------------------------+
*/

#ifndef PHP_TUI_H
#define PHP_TUI_H

extern zend_module_entry tui_module_entry;
#define phpext_tui_ptr &tui_module_entry

#define PHP_TUI_VERSION "0.4.0"
#define PHP_TUI_VERSION_ID 400  /* major * 10000 + minor * 100 + patch */

/* Security limits for string properties */
#define TUI_MAX_KEY_LENGTH 256      /* Maximum key string length */
#define TUI_MAX_ID_LENGTH 256       /* Maximum id string length */
#define TUI_MAX_TEXT_LENGTH 1048576 /* Maximum text content (1MB) */

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
#include <stdint.h>

/* Forward declaration for pools */
struct tui_pools;

/* Telemetry/Metrics structure */
typedef struct {
    int enabled;

    /* Node metrics */
    int64_t node_count;
    int64_t box_count;
    int64_t text_count;
    int64_t static_count;
    int64_t max_depth;

    /* Reconciler metrics */
    int64_t diff_runs;
    int64_t create_ops;
    int64_t update_ops;
    int64_t delete_ops;
    int64_t replace_ops;
    int64_t reorder_ops;

    /* Render timing (nanoseconds) */
    int64_t render_count;
    int64_t layout_time_ns;
    int64_t buffer_time_ns;
    int64_t output_time_ns;
    int64_t max_render_ns;
    int64_t min_render_ns;

    /* Layout metrics */
    int64_t layout_runs;
    int64_t measure_calls;
    int64_t baseline_calls;

    /* Event loop metrics */
    int64_t loop_iterations;
    int64_t input_events;
    int64_t resize_events;
    int64_t timer_fires;
    int64_t poll_errors;

    /* Pool metrics */
    int64_t pool_diff_allocs;
    int64_t pool_diff_fallbacks;
    int64_t pool_diff_reuses;
    int64_t pool_children_allocs;
    int64_t pool_children_fallbacks;
    int64_t pool_children_reuses;
    int64_t pool_keymap_reuses;
} tui_metrics;

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

    /* Telemetry */
    zend_bool metrics_enabled;
    tui_metrics metrics;

    /* Object pools (pointer to avoid header dependency) */
    struct tui_pools *pools;
ZEND_END_MODULE_GLOBALS(tui)

ZEND_EXTERN_MODULE_GLOBALS(tui)

#ifdef ZTS
#define TUI_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(tui, v)
#else
#define TUI_G(v) (tui_globals.v)
#endif

/* Metrics collection macros
 *
 * When TUI_DISABLE_METRICS is defined at compile time, all metrics
 * macros expand to nothing, eliminating any runtime overhead.
 *
 * To build without metrics: add -DTUI_DISABLE_METRICS to CFLAGS
 *   CFLAGS="-DTUI_DISABLE_METRICS" ./configure --enable-tui && make
 *
 * Without this flag, metrics are collected when tui.metrics_enabled=1
 * in php.ini, with minimal overhead (a single branch) when disabled.
 */
#ifdef TUI_DISABLE_METRICS

/* Compile-time disabled: zero overhead */
#define TUI_METRIC_INC(field)           ((void)0)
#define TUI_METRIC_DEC(field)           ((void)0)
#define TUI_METRIC_ADD(field, val)      ((void)0)
#define TUI_METRIC_MAX(field, val)      ((void)0)
#define TUI_METRIC_MIN(field, val)      ((void)0)

#else /* !TUI_DISABLE_METRICS */

/* Runtime-controlled: minimal overhead when disabled (single branch) */
#define TUI_METRIC_INC(field) \
    do { if (TUI_G(metrics_enabled)) TUI_G(metrics).field++; } while(0)

#define TUI_METRIC_DEC(field) \
    do { if (TUI_G(metrics_enabled)) TUI_G(metrics).field--; } while(0)

#define TUI_METRIC_ADD(field, val) \
    do { if (TUI_G(metrics_enabled)) TUI_G(metrics).field += (val); } while(0)

#define TUI_METRIC_MAX(field, val) \
    do { if (TUI_G(metrics_enabled) && (val) > TUI_G(metrics).field) \
        TUI_G(metrics).field = (val); } while(0)

#define TUI_METRIC_MIN(field, val) \
    do { if (TUI_G(metrics_enabled) && \
        (TUI_G(metrics).field == 0 || (val) < TUI_G(metrics).field)) \
        TUI_G(metrics).field = (val); } while(0)

#endif /* TUI_DISABLE_METRICS */

/* Class entries */
extern zend_class_entry *tui_box_ce;
extern zend_class_entry *tui_text_ce;
extern zend_class_entry *tui_instance_ce;
extern zend_class_entry *tui_key_ce;
extern zend_class_entry *tui_color_ce;

/* Exception class entries */
extern zend_class_entry *tui_exception_ce;                    /* Base: Xocdr\Tui\TuiException */
extern zend_class_entry *tui_instance_destroyed_exception_ce; /* Xocdr\Tui\InstanceDestroyedException */
extern zend_class_entry *tui_validation_exception_ce;         /* Xocdr\Tui\ValidationException */
extern zend_class_entry *tui_invalid_dimension_exception_ce;  /* Xocdr\Tui\InvalidDimensionException */
extern zend_class_entry *tui_resource_exception_ce;           /* Xocdr\Tui\ResourceException */
extern zend_class_entry *tui_state_limit_exception_ce;        /* Xocdr\Tui\StateLimitException */
extern zend_class_entry *tui_timer_limit_exception_ce;        /* Xocdr\Tui\TimerLimitException */

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
