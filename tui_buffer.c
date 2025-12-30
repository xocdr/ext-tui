/*
  +----------------------------------------------------------------------+
  | ext-tui: Buffer and drawing primitive functions                      |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"

/* ------------------------------------------------------------------
 * Shared Style Array Parser (used by drawing functions)
 * ------------------------------------------------------------------ */

void parse_style_array(zval *arr, tui_style *style)
{
    if (!arr || Z_TYPE_P(arr) != IS_ARRAY) return;

    HashTable *ht = Z_ARRVAL_P(arr);
    zval *val;

    memset(style, 0, sizeof(tui_style));

    if ((val = zend_hash_str_find(ht, "fg", 2)) != NULL ||
        (val = zend_hash_str_find(ht, "color", 5)) != NULL) {
        if (Z_TYPE_P(val) == IS_STRING) {
            style->fg = tui_color_from_hex(Z_STRVAL_P(val));
        } else if (Z_TYPE_P(val) == IS_ARRAY) {
            HashTable *color_ht = Z_ARRVAL_P(val);
            zval *r = zend_hash_index_find(color_ht, 0);
            zval *g = zend_hash_index_find(color_ht, 1);
            zval *b = zend_hash_index_find(color_ht, 2);
            if (r && g && b) {
                style->fg.r = (uint8_t)zval_get_long(r);
                style->fg.g = (uint8_t)zval_get_long(g);
                style->fg.b = (uint8_t)zval_get_long(b);
                style->fg.is_set = 1;
            }
        }
    }

    if ((val = zend_hash_str_find(ht, "bg", 2)) != NULL ||
        (val = zend_hash_str_find(ht, "backgroundColor", 15)) != NULL) {
        if (Z_TYPE_P(val) == IS_STRING) {
            style->bg = tui_color_from_hex(Z_STRVAL_P(val));
        } else if (Z_TYPE_P(val) == IS_ARRAY) {
            HashTable *color_ht = Z_ARRVAL_P(val);
            zval *r = zend_hash_index_find(color_ht, 0);
            zval *g = zend_hash_index_find(color_ht, 1);
            zval *b = zend_hash_index_find(color_ht, 2);
            if (r && g && b) {
                style->bg.r = (uint8_t)zval_get_long(r);
                style->bg.g = (uint8_t)zval_get_long(g);
                style->bg.b = (uint8_t)zval_get_long(b);
                style->bg.is_set = 1;
            }
        }
    }

    if ((val = zend_hash_str_find(ht, "bold", 4)) != NULL) {
        style->bold = zend_is_true(val);
    }
    if ((val = zend_hash_str_find(ht, "dim", 3)) != NULL) {
        style->dim = zend_is_true(val);
    }
    if ((val = zend_hash_str_find(ht, "italic", 6)) != NULL) {
        style->italic = zend_is_true(val);
    }
    if ((val = zend_hash_str_find(ht, "underline", 9)) != NULL) {
        style->underline = zend_is_true(val);
    }
    if ((val = zend_hash_str_find(ht, "inverse", 7)) != NULL) {
        style->inverse = zend_is_true(val);
    }
    if ((val = zend_hash_str_find(ht, "strikethrough", 13)) != NULL) {
        style->strikethrough = zend_is_true(val);
    }
}

/* ------------------------------------------------------------------
 * Buffer Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_buffer_create(int $width, int $height): resource */
PHP_FUNCTION(tui_buffer_create)
{
    zend_long width, height;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = tui_buffer_create((int)width, (int)height);
    if (!buffer) {
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(buffer, le_tui_buffer));
}
/* }}} */

/* {{{ tui_buffer_clear(resource $buffer): void */
PHP_FUNCTION(tui_buffer_clear)
{
    zval *zbuffer;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zbuffer)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    tui_buffer_clear(buffer);
}
/* }}} */

/* {{{ tui_buffer_render(resource $buffer): string */
PHP_FUNCTION(tui_buffer_render)
{
    zval *zbuffer;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zbuffer)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    char *output = tui_buffer_to_string(buffer);
    if (output) {
        RETVAL_STRING(output);
        free(output);
    } else {
        RETURN_EMPTY_STRING();
    }
}
/* }}} */

/* ------------------------------------------------------------------
 * Drawing Primitive Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_draw_line(resource $buffer, int $x1, int $y1, int $x2, int $y2, string $char = '─', ?array $style = null): void */
PHP_FUNCTION(tui_draw_line)
{
    zval *zbuffer;
    zend_long x1, y1, x2, y2;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 7)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x1)
        Z_PARAM_LONG(y1)
        Z_PARAM_LONG(x2)
        Z_PARAM_LONG(y2)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = 0x2500; /* ─ */
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_draw_line(buffer, (int)x1, (int)y1, (int)x2, (int)y2, codepoint, &style);
}
/* }}} */

/* {{{ tui_draw_rect(resource $buffer, int $x, int $y, int $width, int $height, string $border = 'single', ?array $style = null): void */
PHP_FUNCTION(tui_draw_rect)
{
    zval *zbuffer;
    zend_long x, y, width, height;
    zend_string *border = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 7)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(border)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    tui_border_style border_style = TUI_BORDER_SINGLE;
    if (border) {
        const char *bs = ZSTR_VAL(border);
        if (strcmp(bs, "double") == 0) border_style = TUI_BORDER_DOUBLE;
        else if (strcmp(bs, "round") == 0) border_style = TUI_BORDER_ROUND;
        else if (strcmp(bs, "bold") == 0) border_style = TUI_BORDER_BOLD;
        else if (strcmp(bs, "dashed") == 0) border_style = TUI_BORDER_DASHED;
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_draw_rect(buffer, (int)x, (int)y, (int)width, (int)height, border_style, &style);
}
/* }}} */

/* {{{ tui_fill_rect(resource $buffer, int $x, int $y, int $width, int $height, string $char = ' ', ?array $style = null): void */
PHP_FUNCTION(tui_fill_rect)
{
    zval *zbuffer;
    zend_long x, y, width, height;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 7)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = ' ';
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_fill_rect(buffer, (int)x, (int)y, (int)width, (int)height, codepoint, &style);
}
/* }}} */

/* {{{ tui_draw_circle(resource $buffer, int $cx, int $cy, int $radius, string $char = '●', ?array $style = null): void */
PHP_FUNCTION(tui_draw_circle)
{
    zval *zbuffer;
    zend_long cx, cy, radius;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(4, 6)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(cx)
        Z_PARAM_LONG(cy)
        Z_PARAM_LONG(radius)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = 0x25CF; /* ● */
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_draw_circle(buffer, (int)cx, (int)cy, (int)radius, codepoint, &style);
}
/* }}} */

/* {{{ tui_fill_circle(resource $buffer, int $cx, int $cy, int $radius, string $char = '█', ?array $style = null): void */
PHP_FUNCTION(tui_fill_circle)
{
    zval *zbuffer;
    zend_long cx, cy, radius;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(4, 6)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(cx)
        Z_PARAM_LONG(cy)
        Z_PARAM_LONG(radius)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = 0x2588; /* █ */
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_fill_circle(buffer, (int)cx, (int)cy, (int)radius, codepoint, &style);
}
/* }}} */

/* {{{ tui_draw_ellipse(resource $buffer, int $cx, int $cy, int $rx, int $ry, string $char = '●', ?array $style = null): void */
PHP_FUNCTION(tui_draw_ellipse)
{
    zval *zbuffer;
    zend_long cx, cy, rx, ry;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 7)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(cx)
        Z_PARAM_LONG(cy)
        Z_PARAM_LONG(rx)
        Z_PARAM_LONG(ry)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = 0x25CF;
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_draw_ellipse(buffer, (int)cx, (int)cy, (int)rx, (int)ry, codepoint, &style);
}
/* }}} */

/* {{{ tui_fill_ellipse(resource $buffer, int $cx, int $cy, int $rx, int $ry, string $char = '█', ?array $style = null): void */
PHP_FUNCTION(tui_fill_ellipse)
{
    zval *zbuffer;
    zend_long cx, cy, rx, ry;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 7)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(cx)
        Z_PARAM_LONG(cy)
        Z_PARAM_LONG(rx)
        Z_PARAM_LONG(ry)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = 0x2588;
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_fill_ellipse(buffer, (int)cx, (int)cy, (int)rx, (int)ry, codepoint, &style);
}
/* }}} */

/* {{{ tui_draw_triangle(resource $buffer, int $x1, int $y1, int $x2, int $y2, int $x3, int $y3, string $char = '●', ?array $style = null): void */
PHP_FUNCTION(tui_draw_triangle)
{
    zval *zbuffer;
    zend_long x1, y1, x2, y2, x3, y3;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(7, 9)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x1)
        Z_PARAM_LONG(y1)
        Z_PARAM_LONG(x2)
        Z_PARAM_LONG(y2)
        Z_PARAM_LONG(x3)
        Z_PARAM_LONG(y3)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = 0x25CF;
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_draw_triangle(buffer, (int)x1, (int)y1, (int)x2, (int)y2, (int)x3, (int)y3, codepoint, &style);
}
/* }}} */

/* {{{ tui_fill_triangle(resource $buffer, int $x1, int $y1, int $x2, int $y2, int $x3, int $y3, string $char = '█', ?array $style = null): void */
PHP_FUNCTION(tui_fill_triangle)
{
    zval *zbuffer;
    zend_long x1, y1, x2, y2, x3, y3;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(7, 9)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x1)
        Z_PARAM_LONG(y1)
        Z_PARAM_LONG(x2)
        Z_PARAM_LONG(y2)
        Z_PARAM_LONG(x3)
        Z_PARAM_LONG(y3)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = 0x2588;
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_fill_triangle(buffer, (int)x1, (int)y1, (int)x2, (int)y2, (int)x3, (int)y3, codepoint, &style);
}
/* }}} */
