/*
  +----------------------------------------------------------------------+
  | ext-tui: Animation and color functions                               |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"

/* ------------------------------------------------------------------
 * Animation Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_ease(float $t, string $easing = 'linear'): float */
PHP_FUNCTION(tui_ease)
{
    double t;
    zend_string *easing = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_DOUBLE(t)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(easing)
    ZEND_PARSE_PARAMETERS_END();

    tui_easing ease_type = TUI_EASE_LINEAR;
    if (easing) {
        ease_type = tui_ease_from_name(ZSTR_VAL(easing));
    }

    RETURN_DOUBLE(tui_ease(t, ease_type));
}
/* }}} */

/* {{{ tui_lerp(float $a, float $b, float $t): float */
PHP_FUNCTION(tui_lerp)
{
    double a, b, t;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
        Z_PARAM_DOUBLE(t)
    ZEND_PARSE_PARAMETERS_END();

    RETURN_DOUBLE(tui_lerp(a, b, t));
}
/* }}} */

/* {{{ tui_lerp_color(string|array $a, string|array $b, float $t): string */
PHP_FUNCTION(tui_lerp_color)
{
    zval *za, *zb;
    double t;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_ZVAL(za)
        Z_PARAM_ZVAL(zb)
        Z_PARAM_DOUBLE(t)
    ZEND_PARSE_PARAMETERS_END();

    tui_color color_a = {0}, color_b = {0};

    /* Parse color a */
    if (Z_TYPE_P(za) == IS_STRING) {
        color_a = tui_color_from_hex(Z_STRVAL_P(za));
    } else if (Z_TYPE_P(za) == IS_ARRAY) {
        HashTable *ht = Z_ARRVAL_P(za);
        zval *r = zend_hash_index_find(ht, 0);
        zval *g = zend_hash_index_find(ht, 1);
        zval *b = zend_hash_index_find(ht, 2);
        if (r && g && b) {
            color_a.r = (uint8_t)zval_get_long(r);
            color_a.g = (uint8_t)zval_get_long(g);
            color_a.b = (uint8_t)zval_get_long(b);
            color_a.is_set = 1;
        }
    }

    /* Parse color b */
    if (Z_TYPE_P(zb) == IS_STRING) {
        color_b = tui_color_from_hex(Z_STRVAL_P(zb));
    } else if (Z_TYPE_P(zb) == IS_ARRAY) {
        HashTable *ht = Z_ARRVAL_P(zb);
        zval *r = zend_hash_index_find(ht, 0);
        zval *g = zend_hash_index_find(ht, 1);
        zval *b = zend_hash_index_find(ht, 2);
        if (r && g && b) {
            color_b.r = (uint8_t)zval_get_long(r);
            color_b.g = (uint8_t)zval_get_long(g);
            color_b.b = (uint8_t)zval_get_long(b);
            color_b.is_set = 1;
        }
    }

    tui_color result = tui_lerp_color(color_a, color_b, t);

    char hex[8];
    tui_color_to_hex(result, hex, sizeof(hex));
    RETURN_STRING(hex);
}
/* }}} */

/* {{{ tui_color_from_hex(string $hex): array */
PHP_FUNCTION(tui_color_from_hex)
{
    zend_string *hex;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(hex)
    ZEND_PARSE_PARAMETERS_END();

    tui_color color = tui_color_from_hex(ZSTR_VAL(hex));

    array_init(return_value);
    add_next_index_long(return_value, color.r);
    add_next_index_long(return_value, color.g);
    add_next_index_long(return_value, color.b);
}
/* }}} */

/* {{{ tui_gradient(array $colors, int $steps): array */
PHP_FUNCTION(tui_gradient)
{
    zval *zcolors;
    zend_long steps;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(zcolors)
        Z_PARAM_LONG(steps)
    ZEND_PARSE_PARAMETERS_END();

    HashTable *ht = Z_ARRVAL_P(zcolors);
    int color_count = zend_hash_num_elements(ht);

    /* Handle edge cases */
    if (color_count < 1 || steps < 1) {
        array_init(return_value);
        return;
    }

    tui_color *colors = emalloc(sizeof(tui_color) * color_count);
    tui_color *output = emalloc(sizeof(tui_color) * steps);

    int i = 0;
    zval *zcolor;
    ZEND_HASH_FOREACH_VAL(ht, zcolor) {
        if (Z_TYPE_P(zcolor) == IS_STRING) {
            colors[i] = tui_color_from_hex(Z_STRVAL_P(zcolor));
        } else if (Z_TYPE_P(zcolor) == IS_ARRAY) {
            HashTable *color_ht = Z_ARRVAL_P(zcolor);
            zval *r = zend_hash_index_find(color_ht, 0);
            zval *g = zend_hash_index_find(color_ht, 1);
            zval *b = zend_hash_index_find(color_ht, 2);
            if (r && g && b) {
                colors[i].r = (uint8_t)zval_get_long(r);
                colors[i].g = (uint8_t)zval_get_long(g);
                colors[i].b = (uint8_t)zval_get_long(b);
                colors[i].is_set = 1;
            }
        }
        i++;
    } ZEND_HASH_FOREACH_END();

    /* C function handles single-color case (fills output with same color) */
    tui_gradient(colors, color_count, output, (int)steps);

    array_init(return_value);
    for (i = 0; i < (int)steps; i++) {
        char hex[8];
        tui_color_to_hex(output[i], hex, sizeof(hex));
        add_next_index_string(return_value, hex);
    }

    efree(colors);
    efree(output);
}
/* }}} */
