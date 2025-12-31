/*
  +----------------------------------------------------------------------+
  | ext-tui: Metrics functions                                           |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"

/* ------------------------------------------------------------------
 * Metrics Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_metrics_enable(): void */
PHP_FUNCTION(tui_metrics_enable)
{
    ZEND_PARSE_PARAMETERS_NONE();
    TUI_G(metrics_enabled) = 1;
}
/* }}} */

/* {{{ tui_metrics_disable(): void */
PHP_FUNCTION(tui_metrics_disable)
{
    ZEND_PARSE_PARAMETERS_NONE();
    TUI_G(metrics_enabled) = 0;
}
/* }}} */

/* {{{ tui_metrics_enabled(): bool */
PHP_FUNCTION(tui_metrics_enabled)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_BOOL(TUI_G(metrics_enabled));
}
/* }}} */

/* {{{ tui_metrics_reset(): void */
PHP_FUNCTION(tui_metrics_reset)
{
    ZEND_PARSE_PARAMETERS_NONE();
    memset(&TUI_G(metrics), 0, sizeof(tui_metrics));
}
/* }}} */

/* {{{ tui_get_metrics(): array */
PHP_FUNCTION(tui_get_metrics)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_metrics *m = &TUI_G(metrics);

    array_init(return_value);

    /* Node metrics */
    add_assoc_long(return_value, "node_count", (zend_long)m->node_count);
    add_assoc_long(return_value, "box_count", (zend_long)m->box_count);
    add_assoc_long(return_value, "text_count", (zend_long)m->text_count);
    add_assoc_long(return_value, "static_count", (zend_long)m->static_count);
    add_assoc_long(return_value, "max_depth", (zend_long)m->max_depth);

    /* Reconciler metrics */
    add_assoc_long(return_value, "diff_runs", (zend_long)m->diff_runs);
    add_assoc_long(return_value, "create_ops", (zend_long)m->create_ops);
    add_assoc_long(return_value, "update_ops", (zend_long)m->update_ops);
    add_assoc_long(return_value, "delete_ops", (zend_long)m->delete_ops);
    add_assoc_long(return_value, "replace_ops", (zend_long)m->replace_ops);
    add_assoc_long(return_value, "reorder_ops", (zend_long)m->reorder_ops);
    add_assoc_long(return_value, "total_ops", (zend_long)(m->create_ops + m->update_ops +
        m->delete_ops + m->replace_ops + m->reorder_ops));

    /* Render timing (convert ns to ms) */
    add_assoc_long(return_value, "render_count", (zend_long)m->render_count);
    add_assoc_double(return_value, "layout_time_ms", (double)m->layout_time_ns / 1000000.0);
    add_assoc_double(return_value, "buffer_time_ms", (double)m->buffer_time_ns / 1000000.0);
    add_assoc_double(return_value, "output_time_ms", (double)m->output_time_ns / 1000000.0);
    add_assoc_double(return_value, "total_render_time_ms",
        (double)(m->layout_time_ns + m->buffer_time_ns + m->output_time_ns) / 1000000.0);
    if (m->render_count > 0) {
        add_assoc_double(return_value, "avg_render_ms",
            (double)(m->layout_time_ns + m->buffer_time_ns + m->output_time_ns) /
            (double)m->render_count / 1000000.0);
    } else {
        add_assoc_double(return_value, "avg_render_ms", 0.0);
    }
    add_assoc_double(return_value, "max_render_ms", (double)m->max_render_ns / 1000000.0);
    add_assoc_double(return_value, "min_render_ms", (double)m->min_render_ns / 1000000.0);

    /* Layout metrics */
    add_assoc_long(return_value, "layout_runs", (zend_long)m->layout_runs);
    add_assoc_long(return_value, "measure_calls", (zend_long)m->measure_calls);
    add_assoc_long(return_value, "baseline_calls", (zend_long)m->baseline_calls);

    /* Event loop metrics */
    add_assoc_long(return_value, "loop_iterations", (zend_long)m->loop_iterations);
    add_assoc_long(return_value, "input_events", (zend_long)m->input_events);
    add_assoc_long(return_value, "resize_events", (zend_long)m->resize_events);
    add_assoc_long(return_value, "timer_fires", (zend_long)m->timer_fires);
    add_assoc_long(return_value, "poll_errors", (zend_long)m->poll_errors);

    /* Pool metrics */
    if (TUI_G(pools)) {
        tui_pools *p = TUI_G(pools);
        add_assoc_long(return_value, "pool_children_hits", (zend_long)p->children_hits);
        add_assoc_long(return_value, "pool_children_misses", (zend_long)p->children_misses);
        add_assoc_long(return_value, "pool_children_returns", (zend_long)p->children_returns);
        add_assoc_long(return_value, "pool_keymap_reuses", (zend_long)p->key_map_reuses);
        add_assoc_long(return_value, "pool_keymap_misses", (zend_long)p->key_map_misses);
    }
}
/* }}} */

/* {{{ tui_get_node_metrics(): array */
PHP_FUNCTION(tui_get_node_metrics)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_metrics *m = &TUI_G(metrics);

    array_init(return_value);
    add_assoc_long(return_value, "node_count", (zend_long)m->node_count);
    add_assoc_long(return_value, "box_count", (zend_long)m->box_count);
    add_assoc_long(return_value, "text_count", (zend_long)m->text_count);
    add_assoc_long(return_value, "static_count", (zend_long)m->static_count);
    add_assoc_long(return_value, "max_depth", (zend_long)m->max_depth);
}
/* }}} */

/* {{{ tui_get_reconciler_metrics(): array */
PHP_FUNCTION(tui_get_reconciler_metrics)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_metrics *m = &TUI_G(metrics);
    int64_t total = m->create_ops + m->update_ops + m->delete_ops +
                    m->replace_ops + m->reorder_ops;

    array_init(return_value);
    add_assoc_long(return_value, "diff_runs", (zend_long)m->diff_runs);
    add_assoc_long(return_value, "total_ops", (zend_long)total);
    add_assoc_long(return_value, "creates", (zend_long)m->create_ops);
    add_assoc_long(return_value, "updates", (zend_long)m->update_ops);
    add_assoc_long(return_value, "deletes", (zend_long)m->delete_ops);
    add_assoc_long(return_value, "replaces", (zend_long)m->replace_ops);
    add_assoc_long(return_value, "reorders", (zend_long)m->reorder_ops);
    if (m->diff_runs > 0) {
        add_assoc_double(return_value, "avg_ops_per_diff", (double)total / (double)m->diff_runs);
    } else {
        add_assoc_double(return_value, "avg_ops_per_diff", 0.0);
    }
}
/* }}} */

/* {{{ tui_get_render_metrics(): array */
PHP_FUNCTION(tui_get_render_metrics)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_metrics *m = &TUI_G(metrics);
    int64_t total_ns = m->layout_time_ns + m->buffer_time_ns + m->output_time_ns;

    array_init(return_value);
    add_assoc_long(return_value, "render_count", (zend_long)m->render_count);
    add_assoc_double(return_value, "layout_time_ms", (double)m->layout_time_ns / 1000000.0);
    add_assoc_double(return_value, "buffer_time_ms", (double)m->buffer_time_ns / 1000000.0);
    add_assoc_double(return_value, "output_time_ms", (double)m->output_time_ns / 1000000.0);
    add_assoc_double(return_value, "total_render_time_ms", (double)total_ns / 1000000.0);
    if (m->render_count > 0) {
        add_assoc_double(return_value, "avg_render_ms", (double)total_ns / (double)m->render_count / 1000000.0);
    } else {
        add_assoc_double(return_value, "avg_render_ms", 0.0);
    }
    add_assoc_double(return_value, "max_render_ms", (double)m->max_render_ns / 1000000.0);
    add_assoc_double(return_value, "min_render_ms", (double)m->min_render_ns / 1000000.0);
}
/* }}} */

/* {{{ tui_get_loop_metrics(): array */
PHP_FUNCTION(tui_get_loop_metrics)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_metrics *m = &TUI_G(metrics);

    array_init(return_value);
    add_assoc_long(return_value, "loop_iterations", (zend_long)m->loop_iterations);
    add_assoc_long(return_value, "input_events", (zend_long)m->input_events);
    add_assoc_long(return_value, "resize_events", (zend_long)m->resize_events);
    add_assoc_long(return_value, "timer_fires", (zend_long)m->timer_fires);
    add_assoc_long(return_value, "poll_errors", (zend_long)m->poll_errors);
}
/* }}} */

/* {{{ tui_get_pool_metrics(): array */
PHP_FUNCTION(tui_get_pool_metrics)
{
    ZEND_PARSE_PARAMETERS_NONE();

    array_init(return_value);

    if (TUI_G(pools)) {
        tui_pools *p = TUI_G(pools);
        add_assoc_long(return_value, "children_allocs", (zend_long)p->children_hits);
        add_assoc_long(return_value, "children_fallbacks", (zend_long)p->children_misses);
        add_assoc_long(return_value, "children_reuses", (zend_long)p->children_returns);
        add_assoc_long(return_value, "keymap_reuses", (zend_long)p->key_map_reuses);
        add_assoc_long(return_value, "keymap_fallbacks", (zend_long)p->key_map_misses);

        /* Pool efficiency percentages */
        int64_t total_children = p->children_hits + p->children_misses;
        if (total_children > 0) {
            add_assoc_double(return_value, "children_hit_rate",
                (double)p->children_hits / (double)total_children * 100.0);
        } else {
            add_assoc_double(return_value, "children_hit_rate", 0.0);
        }

        int64_t total_keymap = p->key_map_reuses + p->key_map_misses;
        if (total_keymap > 0) {
            add_assoc_double(return_value, "keymap_hit_rate",
                (double)p->key_map_reuses / (double)total_keymap * 100.0);
        } else {
            add_assoc_double(return_value, "keymap_hit_rate", 0.0);
        }

        /* Pool state: current slot usage */
        add_assoc_long(return_value, "pool_4_slots_used", (zend_long)p->children.count_4);
        add_assoc_long(return_value, "pool_8_slots_used", (zend_long)p->children.count_8);
        add_assoc_long(return_value, "pool_16_slots_used", (zend_long)p->children.count_16);
        add_assoc_long(return_value, "pool_32_slots_used", (zend_long)p->children.count_32);
        add_assoc_bool(return_value, "keymap_in_use", p->key_map.in_use);
    }
}
/* }}} */
