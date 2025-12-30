/*
  +----------------------------------------------------------------------+
  | ext-tui: Progress bar and spinner functions                          |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"

/* ------------------------------------------------------------------
 * Progress/Spinner Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_render_progress_bar(resource $buffer, int $x, int $y, int $width, float $progress, ?array $style = null): void */
PHP_FUNCTION(tui_render_progress_bar)
{
    zval *zbuffer;
    zend_long x, y, width;
    double progress;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 6)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_LONG(width)
        Z_PARAM_DOUBLE(progress)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_render_progress(buffer, (int)x, (int)y, (int)width, progress, 0, 0, &style, NULL);
}
/* }}} */

/* {{{ tui_render_busy_bar(resource $buffer, int $x, int $y, int $width, int $frame, string $style_name = 'pulse', ?array $style = null): void */
PHP_FUNCTION(tui_render_busy_bar)
{
    zval *zbuffer;
    zend_long x, y, width, frame;
    zend_string *style_name = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 7)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(frame)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(style_name)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    tui_busy_style busy_style = TUI_BUSY_PULSE;
    if (style_name) {
        const char *sn = ZSTR_VAL(style_name);
        if (strcmp(sn, "snake") == 0) busy_style = TUI_BUSY_SNAKE;
        else if (strcmp(sn, "gradient") == 0) busy_style = TUI_BUSY_GRADIENT;
        else if (strcmp(sn, "wave") == 0) busy_style = TUI_BUSY_WAVE;
        else if (strcmp(sn, "shimmer") == 0) busy_style = TUI_BUSY_SHIMMER;
        else if (strcmp(sn, "rainbow") == 0) busy_style = TUI_BUSY_RAINBOW;
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_render_busy(buffer, (int)x, (int)y, (int)width, (int)frame, busy_style, &style);
}
/* }}} */

/* {{{ tui_spinner_frame(string $type, int $frame): string */
PHP_FUNCTION(tui_spinner_frame)
{
    zend_string *type;
    zend_long frame;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(type)
        Z_PARAM_LONG(frame)
    ZEND_PARSE_PARAMETERS_END();

    tui_spinner_type spinner_type = TUI_SPINNER_DOTS;
    const char *t = ZSTR_VAL(type);
    if (strcmp(t, "line") == 0) spinner_type = TUI_SPINNER_LINE;
    else if (strcmp(t, "circle") == 0) spinner_type = TUI_SPINNER_CIRCLE;
    else if (strcmp(t, "arrow") == 0) spinner_type = TUI_SPINNER_ARROW;
    else if (strcmp(t, "box") == 0) spinner_type = TUI_SPINNER_BOX;
    else if (strcmp(t, "bounce") == 0) spinner_type = TUI_SPINNER_BOUNCE;
    else if (strcmp(t, "clock") == 0) spinner_type = TUI_SPINNER_CLOCK;
    else if (strcmp(t, "moon") == 0) spinner_type = TUI_SPINNER_MOON;
    else if (strcmp(t, "earth") == 0) spinner_type = TUI_SPINNER_EARTH;

    char output[16];
    int len = tui_spinner_frame(spinner_type, (int)frame, output);
    RETURN_STRINGL(output, len);
}
/* }}} */

/* {{{ tui_spinner_frame_count(string $type): int */
PHP_FUNCTION(tui_spinner_frame_count)
{
    zend_string *type;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(type)
    ZEND_PARSE_PARAMETERS_END();

    tui_spinner_type spinner_type = TUI_SPINNER_DOTS;
    const char *t = ZSTR_VAL(type);
    if (strcmp(t, "line") == 0) spinner_type = TUI_SPINNER_LINE;
    else if (strcmp(t, "circle") == 0) spinner_type = TUI_SPINNER_CIRCLE;
    else if (strcmp(t, "arrow") == 0) spinner_type = TUI_SPINNER_ARROW;
    else if (strcmp(t, "box") == 0) spinner_type = TUI_SPINNER_BOX;
    else if (strcmp(t, "bounce") == 0) spinner_type = TUI_SPINNER_BOUNCE;
    else if (strcmp(t, "clock") == 0) spinner_type = TUI_SPINNER_CLOCK;
    else if (strcmp(t, "moon") == 0) spinner_type = TUI_SPINNER_MOON;
    else if (strcmp(t, "earth") == 0) spinner_type = TUI_SPINNER_EARTH;

    RETURN_LONG(tui_spinner_frame_count(spinner_type));
}
/* }}} */

/* {{{ tui_render_spinner(resource $buffer, int $x, int $y, string $type, int $frame, ?array $style = null): void */
PHP_FUNCTION(tui_render_spinner)
{
    zval *zbuffer;
    zend_long x, y, frame;
    zend_string *type;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 6)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_STR(type)
        Z_PARAM_LONG(frame)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    tui_spinner_type spinner_type = TUI_SPINNER_DOTS;
    const char *t = ZSTR_VAL(type);
    if (strcmp(t, "line") == 0) spinner_type = TUI_SPINNER_LINE;
    else if (strcmp(t, "circle") == 0) spinner_type = TUI_SPINNER_CIRCLE;
    else if (strcmp(t, "arrow") == 0) spinner_type = TUI_SPINNER_ARROW;
    else if (strcmp(t, "box") == 0) spinner_type = TUI_SPINNER_BOX;
    else if (strcmp(t, "bounce") == 0) spinner_type = TUI_SPINNER_BOUNCE;
    else if (strcmp(t, "clock") == 0) spinner_type = TUI_SPINNER_CLOCK;
    else if (strcmp(t, "moon") == 0) spinner_type = TUI_SPINNER_MOON;
    else if (strcmp(t, "earth") == 0) spinner_type = TUI_SPINNER_EARTH;

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_render_spinner(buffer, (int)x, (int)y, spinner_type, (int)frame, &style);
}
/* }}} */
