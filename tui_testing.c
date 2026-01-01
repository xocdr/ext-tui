/*
  +----------------------------------------------------------------------+
  | ext-tui: Testing framework functions                                 |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"

/* ------------------------------------------------------------------
 * Testing Framework Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_test_create(int $width, int $height): resource */
PHP_FUNCTION(tui_test_create)
{
    zend_long width, height;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
    ZEND_PARSE_PARAMETERS_END();

    if (width <= 0 || width > 1000 || height <= 0 || height > 1000) {
        zend_throw_exception(tui_invalid_dimension_exception_ce,
            "Invalid dimensions: width and height must be 1-1000", 0);
        RETURN_THROWS();
    }

    tui_test_renderer *renderer = tui_test_renderer_create((int)width, (int)height);
    if (!renderer) {
        zend_throw_exception(tui_resource_exception_ce,
            "Failed to create test renderer", 0);
        RETURN_THROWS();
    }

    RETURN_RES(zend_register_resource(renderer, le_tui_test_renderer));
}
/* }}} */

/* {{{ tui_test_destroy(resource $renderer): void */
PHP_FUNCTION(tui_test_destroy)
{
    zval *zrenderer;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrenderer)
    ZEND_PARSE_PARAMETERS_END();

    /* Closing the resource will call the destructor */
    zend_list_close(Z_RES_P(zrenderer));
}
/* }}} */

/* {{{ tui_test_render(resource $renderer, TuiBox|TuiText $element): void */
PHP_FUNCTION(tui_test_render)
{
    zval *zrenderer, *zelement;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zrenderer)
        Z_PARAM_OBJECT(zelement)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    /* Convert PHP element to tui_node tree */
    tui_node *root = php_to_tui_node(zelement, 0);
    if (!root) {
        zend_throw_exception(tui_validation_exception_ce,
            "Failed to convert element to node", 0);
        RETURN_THROWS();
    }

    /* Render to test buffer */
    tui_test_renderer_render(renderer, root);
}
/* }}} */

/* {{{ tui_test_get_output(resource $renderer): array */
PHP_FUNCTION(tui_test_get_output)
{
    zval *zrenderer;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrenderer)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    int line_count;
    char **lines = tui_test_renderer_get_output(renderer, &line_count);
    if (!lines) {
        array_init(return_value);
        return;
    }

    array_init_size(return_value, line_count);
    for (int i = 0; i < line_count; i++) {
        add_next_index_string(return_value, lines[i]);
    }

    tui_test_renderer_free_output(lines, line_count);
}
/* }}} */

/* {{{ tui_test_to_string(resource $renderer): string */
PHP_FUNCTION(tui_test_to_string)
{
    zval *zrenderer;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrenderer)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    char *output = tui_test_renderer_to_string(renderer);
    if (!output) {
        RETURN_EMPTY_STRING();
    }

    RETVAL_STRING(output);
    free(output);
}
/* }}} */

/* {{{ tui_test_send_input(resource $renderer, string $input): void */
PHP_FUNCTION(tui_test_send_input)
{
    zval *zrenderer;
    zend_string *input;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zrenderer)
        Z_PARAM_STR(input)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    tui_test_renderer_send_input(renderer, ZSTR_VAL(input), (int)ZSTR_LEN(input));
}
/* }}} */

/* {{{ tui_test_send_key(resource $renderer, int $keyCode): void */
PHP_FUNCTION(tui_test_send_key)
{
    zval *zrenderer;
    zend_long key_code;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zrenderer)
        Z_PARAM_LONG(key_code)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    tui_test_renderer_send_key(renderer, (int)key_code);
}
/* }}} */

/* {{{ tui_test_advance_frame(resource $renderer): void */
PHP_FUNCTION(tui_test_advance_frame)
{
    zval *zrenderer;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrenderer)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    tui_test_renderer_advance_frame(renderer);
}
/* }}} */

/* {{{ tui_test_run_timers(resource $renderer, int $ms): void */
PHP_FUNCTION(tui_test_run_timers)
{
    zval *zrenderer;
    zend_long ms;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zrenderer)
        Z_PARAM_LONG(ms)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    tui_test_renderer_run_timers(renderer, (int)ms);
}
/* }}} */

/* {{{ tui_test_get_by_id(resource $renderer, string $id): ?array */
PHP_FUNCTION(tui_test_get_by_id)
{
    zval *zrenderer;
    zend_string *id;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zrenderer)
        Z_PARAM_STR(id)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    if (!renderer->root) {
        RETURN_NULL();
    }

    tui_node *node = tui_test_find_by_id(renderer->root, ZSTR_VAL(id));
    if (!node) {
        RETURN_NULL();
    }

    tui_test_node_info info;
    tui_test_get_node_info(node, &info);

    array_init(return_value);
    if (info.id) add_assoc_string(return_value, "id", (char *)info.id);
    add_assoc_string(return_value, "type", (char *)info.type);
    if (info.text) add_assoc_string(return_value, "text", (char *)info.text);
    add_assoc_long(return_value, "x", info.x);
    add_assoc_long(return_value, "y", info.y);
    add_assoc_long(return_value, "width", info.width);
    add_assoc_long(return_value, "height", info.height);
    add_assoc_bool(return_value, "focusable", info.focusable);
    add_assoc_bool(return_value, "focused", info.focused);
}
/* }}} */

/* {{{ tui_test_get_by_text(resource $renderer, string $text): array */
PHP_FUNCTION(tui_test_get_by_text)
{
    zval *zrenderer;
    zend_string *text;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zrenderer)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    array_init(return_value);

    if (!renderer->root) {
        return;
    }

    int count;
    tui_node **nodes = tui_test_find_by_text(renderer->root, ZSTR_VAL(text), &count);
    if (!nodes || count == 0) {
        return;
    }

    for (int i = 0; i < count; i++) {
        zval node_arr;
        tui_test_node_info info;
        tui_test_get_node_info(nodes[i], &info);

        array_init(&node_arr);
        if (info.id) add_assoc_string(&node_arr, "id", (char *)info.id);
        add_assoc_string(&node_arr, "type", (char *)info.type);
        if (info.text) add_assoc_string(&node_arr, "text", (char *)info.text);
        add_assoc_long(&node_arr, "x", info.x);
        add_assoc_long(&node_arr, "y", info.y);
        add_assoc_long(&node_arr, "width", info.width);
        add_assoc_long(&node_arr, "height", info.height);
        add_assoc_bool(&node_arr, "focusable", info.focusable);
        add_assoc_bool(&node_arr, "focused", info.focused);

        add_next_index_zval(return_value, &node_arr);
    }

    free(nodes);
}
/* }}} */
