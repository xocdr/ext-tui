/*
  +----------------------------------------------------------------------+
  | ext-tui: Render and lifecycle functions                              |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"

/* ------------------------------------------------------------------
 * Forward declarations
 * ------------------------------------------------------------------ */

static void render_component_callback(tui_app *app);
static tui_app* get_app_from_instance(zval *instance);

/* ------------------------------------------------------------------
 * Render Component Callback (called by C library on rerender)
 * ------------------------------------------------------------------ */

static void render_component_callback(tui_app *app)
{
    if (!app || !app->instance_zval_set) {
        return;
    }

    /* Re-entrancy protection: if we're already rendering, just mark that a
     * rerender was requested. The outer render will pick this up and re-render
     * again after the current render completes. This prevents double-free of
     * the node tree when a nested rerender is triggered (e.g., by a hook). */
    if (app->is_rendering) {
        app->rerender_requested = 1;
        return;
    }

    /* Mark that we're now rendering */
    app->is_rendering = 1;
    app->rerender_requested = 0;

    /* Reset state index before each render (for useState hook ordering) */
    tui_app_reset_state_index(app);

    /* Set up parameters - pass Instance as first argument */
    zval retval;
    zval params[1];
    ZVAL_COPY(&params[0], &app->instance_zval);

    /* Save original param count and set up for our call */
    zend_fcall_info fci_copy = app->component_fci;
    fci_copy.param_count = 1;
    fci_copy.params = params;
    fci_copy.retval = &retval;

    /* Capture output during component render to prevent leakage */
    php_output_start_default();

    /* Call the PHP component function with Instance parameter */
    if (zend_call_function(&fci_copy, &app->component_fcc) == SUCCESS) {
        /* Convert PHP object tree to C node tree */
        if (Z_TYPE(retval) == IS_OBJECT) {
            /* Free old tree AFTER building new tree to avoid dangling pointer.
             * This ensures app->root_node is never pointing to freed memory. */
            tui_node *old_tree = app->root_node;

            /* Build new tree */
            app->root_node = php_to_tui_node(&retval, 0);

            /* Now safe to free old tree */
            if (old_tree) {
                tui_node_destroy(old_tree);
            }
        }

        zval_ptr_dtor(&retval);
    }

    /* Capture any output that occurred during render */
    {
        zval captured;
        ZVAL_UNDEF(&captured);
        php_output_get_contents(&captured);

        /* Free previous captured output */
        if (app->captured_output) {
            efree(app->captured_output);
            app->captured_output = NULL;
            app->captured_output_len = 0;
        }

        /* Store new captured output (if any) */
        if (Z_TYPE(captured) == IS_STRING && Z_STRLEN(captured) > 0) {
            app->captured_output = estrndup(Z_STRVAL(captured), Z_STRLEN(captured));
            app->captured_output_len = Z_STRLEN(captured);
        }

        zval_ptr_dtor(&captured);
    }

    /* Discard the buffer (don't flush to stdout) */
    php_output_discard();

    /* Clean up the copied zval reference */
    zval_ptr_dtor(&params[0]);

    /* Mark render complete */
    app->is_rendering = 0;

    /* If a rerender was requested during render, do it now */
    if (app->rerender_requested) {
        app->rerender_requested = 0;
        render_component_callback(app);
    }
}

/* Helper to get app from TuiInstance */
static tui_app* get_app_from_instance(zval *instance)
{
    tui_instance_object *obj = Z_TUI_INSTANCE_P(instance);
    return obj->app;
}

/* ------------------------------------------------------------------
 * Core Render Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_render(callable $component, array $options = []): TuiInstance */
PHP_FUNCTION(tui_render)
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    zval *options = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_FUNC(fci, fcc)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(options)
    ZEND_PARSE_PARAMETERS_END();

    /* Create app instance */
    tui_app *app = tui_app_create();
    if (!app) {
        php_error_docref(NULL, E_ERROR,
            "Failed to create TUI application: memory allocation failed");
        RETURN_NULL();
    }

    /* Parse options */
    if (options) {
        zval *opt;
        if ((opt = zend_hash_str_find(Z_ARRVAL_P(options), "fullscreen", sizeof("fullscreen")-1)) != NULL) {
            tui_app_set_fullscreen(app, zend_is_true(opt));
        }
        if ((opt = zend_hash_str_find(Z_ARRVAL_P(options), "exitOnCtrlC", sizeof("exitOnCtrlC")-1)) != NULL) {
            tui_app_set_exit_on_ctrl_c(app, zend_is_true(opt));
        }
    }

    /* Set component callback */
    tui_app_set_component(app, &fci, &fcc);

    /* Set rerender callback for auto-rerender on resize */
    app->rerender_callback = render_component_callback;

    /* Create Instance object FIRST - same object passed to callback and returned */
    object_init_ex(return_value, tui_instance_ce);
    tui_instance_object *instance_obj = Z_TUI_INSTANCE_P(return_value);
    instance_obj->app = app;

    /* Store a COPY of the Instance zval in app for render_component_callback.
     * We must copy because return_value is on the stack and will be invalid
     * after this function returns. The copy adds a reference to the object. */
    ZVAL_COPY(&app->instance_zval, return_value);
    app->instance_zval_set = 1;

    /* Initial render - call component to get tree with Instance parameter */
    zval retval;

    /* Reset state index for initial render */
    tui_app_reset_state_index(app);

    /* Set up parameters - pass the same Instance we're returning */
    zval params[1];
    ZVAL_COPY(&params[0], return_value);

    zend_fcall_info fci_copy = fci;
    fci_copy.param_count = 1;
    fci_copy.params = params;
    fci_copy.retval = &retval;

    /* Capture output during initial render to prevent leakage to terminal */
    php_output_start_default();

    if (zend_call_function(&fci_copy, &fcc) == SUCCESS) {
        if (Z_TYPE(retval) == IS_OBJECT) {
            if (!instanceof_function(Z_OBJCE(retval), tui_box_ce) &&
                !instanceof_function(Z_OBJCE(retval), tui_text_ce)) {
                zval_ptr_dtor(&retval);
                zval_ptr_dtor(&params[0]);
                zval_ptr_dtor(&app->instance_zval);
                app->instance_zval_set = 0;
                instance_obj->app = NULL;
                tui_app_destroy(app);
                zend_throw_exception(zend_ce_exception,
                    "Component must return Box or Text, got other object", 0);
                RETURN_THROWS();
            }
            app->root_node = php_to_tui_node(&retval, 0);
        } else if (Z_TYPE(retval) != IS_NULL) {
            zval_ptr_dtor(&retval);
            zval_ptr_dtor(&params[0]);
            zval_ptr_dtor(&app->instance_zval);
            app->instance_zval_set = 0;
            instance_obj->app = NULL;
            tui_app_destroy(app);
            zend_throw_exception(zend_ce_exception,
                "Component must return Box, Text, or null", 0);
            RETURN_THROWS();
        }
        zval_ptr_dtor(&retval);
    } else {
        zval_ptr_dtor(&params[0]);
        zval_ptr_dtor(&app->instance_zval);
        app->instance_zval_set = 0;
        instance_obj->app = NULL;
        tui_app_destroy(app);
        zend_throw_exception(zend_ce_exception,
            "Component callback failed to execute", 0);
        RETURN_THROWS();
    }

    /* Clean up the copied param reference */
    zval_ptr_dtor(&params[0]);

    /* Discard any captured output from initial render */
    php_output_discard();

    /* Start the app */
    if (tui_app_start(app) != 0) {
        zval_ptr_dtor(&app->instance_zval);
        app->instance_zval_set = 0;
        instance_obj->app = NULL;
        tui_app_destroy(app);
        php_error_docref(NULL, E_ERROR,
            "Failed to start TUI application: could not enable raw terminal mode "
            "(terminal may not be interactive or stdin is not a TTY)");
        RETURN_NULL();
    }

    /* return_value already contains the Instance object */
}
/* }}} */

/* {{{ tui_rerender(TuiInstance $instance): void */
PHP_FUNCTION(tui_rerender)
{
    zval *instance;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        render_component_callback(app);  /* Call PHP component, build node tree */
        tui_app_render_tree(app);        /* Render tree to screen (no double component call) */
    }
}
/* }}} */

/* {{{ tui_unmount(TuiInstance $instance): void */
PHP_FUNCTION(tui_unmount)
{
    zval *instance;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(instance);
    if (obj->app) {
        tui_app_stop(obj->app);
        tui_app_destroy(obj->app);
        /* Clear the app pointer in the C struct */
        obj->app = NULL;
    }
}
/* }}} */

/* {{{ tui_wait_until_exit(TuiInstance $instance): void */
PHP_FUNCTION(tui_wait_until_exit)
{
    zval *instance;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_wait_until_exit(app);
    }
}
/* }}} */

/* {{{ tui_set_input_handler(TuiInstance $instance, callable $handler): void */
PHP_FUNCTION(tui_set_input_handler)
{
    zval *instance;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_set_input_handler(app, &fci, &fcc);
    }
}
/* }}} */

/* {{{ tui_set_focus_handler(TuiInstance $instance, callable $handler): void */
PHP_FUNCTION(tui_set_focus_handler)
{
    zval *instance;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_set_focus_handler(app, &fci, &fcc);
    }
}
/* }}} */

/* {{{ tui_focus_next(TuiInstance $instance): void */
PHP_FUNCTION(tui_focus_next)
{
    zval *instance;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_focus_next(app);
    }
}
/* }}} */

/* {{{ tui_focus_prev(TuiInstance $instance): void */
PHP_FUNCTION(tui_focus_prev)
{
    zval *instance;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_focus_prev(app);
    }
}
/* }}} */

/* {{{ tui_set_resize_handler(TuiInstance $instance, callable $handler): void */
PHP_FUNCTION(tui_set_resize_handler)
{
    zval *instance;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_set_resize_handler(app, &fci, &fcc);
    }
}
/* }}} */

/* {{{ tui_get_size(TuiInstance $instance): array */
PHP_FUNCTION(tui_get_size)
{
    zval *instance;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        array_init(return_value);
        add_assoc_long(return_value, "width", app->width);
        add_assoc_long(return_value, "height", app->height);
        add_assoc_long(return_value, "columns", app->width);
        add_assoc_long(return_value, "rows", app->height);
    } else {
        php_error_docref(NULL, E_WARNING, "TuiInstance has been destroyed or unmounted");
        RETURN_NULL();
    }
}
/* }}} */

/* {{{ tui_get_focused_node(TuiInstance $instance): ?array */
PHP_FUNCTION(tui_get_focused_node)
{
    zval *instance;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (!app) {
        php_error_docref(NULL, E_WARNING, "TuiInstance has been destroyed or unmounted");
        RETURN_NULL();
    }
    if (app->focused_node) {
        array_init(return_value);
        add_assoc_bool(return_value, "focusable", app->focused_node->focusable);
        add_assoc_bool(return_value, "focused", app->focused_node->focused);
        add_assoc_long(return_value, "x", (zend_long)app->focused_node->x);
        add_assoc_long(return_value, "y", (zend_long)app->focused_node->y);
        add_assoc_long(return_value, "width", (zend_long)app->focused_node->width);
        add_assoc_long(return_value, "height", (zend_long)app->focused_node->height);
        if (app->focused_node->type == TUI_NODE_TEXT && app->focused_node->text) {
            add_assoc_string(return_value, "type", "text");
            add_assoc_string(return_value, "content", app->focused_node->text);
        } else {
            add_assoc_string(return_value, "type", "box");
        }
    } else {
        /* No focused node - this is valid, not an error */
        RETURN_NULL();
    }
}
/* }}} */

/* ------------------------------------------------------------------
 * Timer Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_add_timer(TuiInstance $instance, int $interval_ms, callable $callback): int */
PHP_FUNCTION(tui_add_timer)
{
    zval *instance;
    zend_long interval_ms;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
        Z_PARAM_LONG(interval_ms)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        int timer_id = tui_app_add_timer(app, (int)interval_ms, &fci, &fcc);
        RETURN_LONG(timer_id);
    }

    RETURN_LONG(-1);
}
/* }}} */

/* {{{ tui_remove_timer(TuiInstance $instance, int $timer_id): void */
PHP_FUNCTION(tui_remove_timer)
{
    zval *instance;
    zend_long timer_id;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
        Z_PARAM_LONG(timer_id)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_remove_timer(app, (int)timer_id);
    }
}
/* }}} */

/* {{{ tui_set_tick_handler(TuiInstance $instance, callable $handler): void */
PHP_FUNCTION(tui_set_tick_handler)
{
    zval *instance;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_set_tick_handler(app, &fci, &fcc);
    }
}
/* }}} */
