/*
  +----------------------------------------------------------------------+
  | ext-tui: Canvas functions (braille/block/ascii)                      |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"

/* ------------------------------------------------------------------
 * Canvas Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_canvas_create(int $width, int $height, string $mode = 'braille'): resource */
PHP_FUNCTION(tui_canvas_create)
{
    zend_long width, height;
    zend_string *mode = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(mode)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas_mode canvas_mode = TUI_CANVAS_BRAILLE;
    if (mode) {
        const char *m = ZSTR_VAL(mode);
        if (strcmp(m, "block") == 0) canvas_mode = TUI_CANVAS_BLOCK;
        else if (strcmp(m, "ascii") == 0) canvas_mode = TUI_CANVAS_ASCII;
    }

    tui_canvas *canvas = tui_canvas_create((int)width, (int)height, canvas_mode);
    if (!canvas) {
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(canvas, le_tui_canvas));
}
/* }}} */

/* {{{ tui_canvas_set(resource $canvas, int $x, int $y): void */
PHP_FUNCTION(tui_canvas_set)
{
    zval *zcanvas;
    zend_long x, y;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_set(canvas, (int)x, (int)y);
}
/* }}} */

/* {{{ tui_canvas_unset(resource $canvas, int $x, int $y): void */
PHP_FUNCTION(tui_canvas_unset)
{
    zval *zcanvas;
    zend_long x, y;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_unset(canvas, (int)x, (int)y);
}
/* }}} */

/* {{{ tui_canvas_toggle(resource $canvas, int $x, int $y): void */
PHP_FUNCTION(tui_canvas_toggle)
{
    zval *zcanvas;
    zend_long x, y;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_toggle(canvas, (int)x, (int)y);
}
/* }}} */

/* {{{ tui_canvas_get(resource $canvas, int $x, int $y): bool */
PHP_FUNCTION(tui_canvas_get)
{
    zval *zcanvas;
    zend_long x, y;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    RETURN_BOOL(tui_canvas_get(canvas, (int)x, (int)y));
}
/* }}} */

/* {{{ tui_canvas_clear(resource $canvas): void */
PHP_FUNCTION(tui_canvas_clear)
{
    zval *zcanvas;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zcanvas)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_clear_all(canvas);
}
/* }}} */

/* {{{ tui_canvas_line(resource $canvas, int $x1, int $y1, int $x2, int $y2): void */
PHP_FUNCTION(tui_canvas_line)
{
    zval *zcanvas;
    zend_long x1, y1, x2, y2;

    ZEND_PARSE_PARAMETERS_START(5, 5)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(x1)
        Z_PARAM_LONG(y1)
        Z_PARAM_LONG(x2)
        Z_PARAM_LONG(y2)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_line(canvas, (int)x1, (int)y1, (int)x2, (int)y2);
}
/* }}} */

/* {{{ tui_canvas_rect(resource $canvas, int $x, int $y, int $width, int $height): void */
PHP_FUNCTION(tui_canvas_rect)
{
    zval *zcanvas;
    zend_long x, y, width, height;

    ZEND_PARSE_PARAMETERS_START(5, 5)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_rect(canvas, (int)x, (int)y, (int)width, (int)height);
}
/* }}} */

/* {{{ tui_canvas_fill_rect(resource $canvas, int $x, int $y, int $width, int $height): void */
PHP_FUNCTION(tui_canvas_fill_rect)
{
    zval *zcanvas;
    zend_long x, y, width, height;

    ZEND_PARSE_PARAMETERS_START(5, 5)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_fill_rect(canvas, (int)x, (int)y, (int)width, (int)height);
}
/* }}} */

/* {{{ tui_canvas_circle(resource $canvas, int $cx, int $cy, int $radius): void */
PHP_FUNCTION(tui_canvas_circle)
{
    zval *zcanvas;
    zend_long cx, cy, radius;

    ZEND_PARSE_PARAMETERS_START(4, 4)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(cx)
        Z_PARAM_LONG(cy)
        Z_PARAM_LONG(radius)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_circle(canvas, (int)cx, (int)cy, (int)radius);
}
/* }}} */

/* {{{ tui_canvas_fill_circle(resource $canvas, int $cx, int $cy, int $radius): void */
PHP_FUNCTION(tui_canvas_fill_circle)
{
    zval *zcanvas;
    zend_long cx, cy, radius;

    ZEND_PARSE_PARAMETERS_START(4, 4)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(cx)
        Z_PARAM_LONG(cy)
        Z_PARAM_LONG(radius)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_fill_circle(canvas, (int)cx, (int)cy, (int)radius);
}
/* }}} */

/* {{{ tui_canvas_set_color(resource $canvas, int $r, int $g, int $b): void */
PHP_FUNCTION(tui_canvas_set_color)
{
    zval *zcanvas;
    zend_long r, g, b;

    ZEND_PARSE_PARAMETERS_START(4, 4)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(r)
        Z_PARAM_LONG(g)
        Z_PARAM_LONG(b)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_set_color(canvas, (uint8_t)r, (uint8_t)g, (uint8_t)b);
}
/* }}} */

/* {{{ tui_canvas_get_resolution(resource $canvas): array */
PHP_FUNCTION(tui_canvas_get_resolution)
{
    zval *zcanvas;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zcanvas)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    int width, height;
    tui_canvas_get_resolution(canvas, &width, &height);

    array_init(return_value);
    add_assoc_long(return_value, "width", width);
    add_assoc_long(return_value, "height", height);
    add_assoc_long(return_value, "char_width", canvas->char_width);
    add_assoc_long(return_value, "char_height", canvas->char_height);
}
/* }}} */

/* {{{ tui_canvas_render(resource $canvas): array */
PHP_FUNCTION(tui_canvas_render)
{
    zval *zcanvas;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zcanvas)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    int line_count;
    char **lines = tui_canvas_render(canvas, &line_count);

    array_init(return_value);
    if (lines) {
        for (int i = 0; i < line_count; i++) {
            add_next_index_string(return_value, lines[i]);
        }
        tui_canvas_render_free(lines, line_count);
    }
}
/* }}} */
