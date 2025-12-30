/*
  +----------------------------------------------------------------------+
  | ext-tui: Sprite functions                                            |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"

/* ------------------------------------------------------------------
 * Sprite Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_sprite_create(array $frames, string $name = 'default', bool $loop = true): resource */
PHP_FUNCTION(tui_sprite_create)
{
    zval *zframes;
    zend_string *name = NULL;
    zend_bool loop = 1;

    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_ARRAY(zframes)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(name)
        Z_PARAM_BOOL(loop)
    ZEND_PARSE_PARAMETERS_END();

    HashTable *ht = Z_ARRVAL_P(zframes);
    int frame_count = zend_hash_num_elements(ht);

    if (frame_count == 0) {
        RETURN_FALSE;
    }

    tui_sprite_frame *frames = ecalloc(frame_count, sizeof(tui_sprite_frame));
    int i = 0;
    zval *zframe;

    ZEND_HASH_FOREACH_VAL(ht, zframe) {
        if (Z_TYPE_P(zframe) != IS_ARRAY) continue;

        HashTable *frame_ht = Z_ARRVAL_P(zframe);
        zval *zlines = zend_hash_str_find(frame_ht, "lines", 5);
        zval *zduration = zend_hash_str_find(frame_ht, "duration", 8);

        int duration = zduration ? (int)zval_get_long(zduration) : 100;

        if (zlines && Z_TYPE_P(zlines) == IS_ARRAY) {
            HashTable *lines_ht = Z_ARRVAL_P(zlines);
            int line_count = zend_hash_num_elements(lines_ht);
            const char **lines = emalloc(sizeof(char*) * line_count);

            int j = 0;
            zval *zline;
            ZEND_HASH_FOREACH_VAL(lines_ht, zline) {
                convert_to_string(zline);
                lines[j++] = Z_STRVAL_P(zline);
            } ZEND_HASH_FOREACH_END();

            frames[i] = tui_sprite_frame_create(lines, line_count, duration);
            efree(lines);
        }
        i++;
    } ZEND_HASH_FOREACH_END();

    const char *anim_name = name ? ZSTR_VAL(name) : "default";
    tui_sprite *sprite = tui_sprite_create(frames, frame_count, anim_name, loop ? 1 : 0);

    /* Free temporary frame data (sprite makes copies) */
    for (i = 0; i < frame_count; i++) {
        tui_sprite_frame_free(&frames[i]);
    }
    efree(frames);

    if (!sprite) {
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(sprite, le_tui_sprite));
}
/* }}} */

/* {{{ tui_sprite_update(resource $sprite, int $delta_ms): void */
PHP_FUNCTION(tui_sprite_update)
{
    zval *zsprite;
    zend_long delta_ms;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zsprite)
        Z_PARAM_LONG(delta_ms)
    ZEND_PARSE_PARAMETERS_END();

    tui_sprite *sprite = (tui_sprite *)zend_fetch_resource(Z_RES_P(zsprite), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!sprite) {
        RETURN_THROWS();
    }

    tui_sprite_update(sprite, (int)delta_ms);
}
/* }}} */

/* {{{ tui_sprite_set_animation(resource $sprite, string $name): bool */
PHP_FUNCTION(tui_sprite_set_animation)
{
    zval *zsprite;
    zend_string *name;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zsprite)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    tui_sprite *sprite = (tui_sprite *)zend_fetch_resource(Z_RES_P(zsprite), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!sprite) {
        RETURN_THROWS();
    }

    RETURN_BOOL(tui_sprite_set_animation(sprite, ZSTR_VAL(name)) == 0);
}
/* }}} */

/* {{{ tui_sprite_set_position(resource $sprite, int $x, int $y): void */
PHP_FUNCTION(tui_sprite_set_position)
{
    zval *zsprite;
    zend_long x, y;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_RESOURCE(zsprite)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
    ZEND_PARSE_PARAMETERS_END();

    tui_sprite *sprite = (tui_sprite *)zend_fetch_resource(Z_RES_P(zsprite), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!sprite) {
        RETURN_THROWS();
    }

    tui_sprite_set_position(sprite, (int)x, (int)y);
}
/* }}} */

/* {{{ tui_sprite_flip(resource $sprite, bool $flipped): void */
PHP_FUNCTION(tui_sprite_flip)
{
    zval *zsprite;
    zend_bool flipped;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zsprite)
        Z_PARAM_BOOL(flipped)
    ZEND_PARSE_PARAMETERS_END();

    tui_sprite *sprite = (tui_sprite *)zend_fetch_resource(Z_RES_P(zsprite), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!sprite) {
        RETURN_THROWS();
    }

    tui_sprite_flip(sprite, flipped ? 1 : 0);
}
/* }}} */

/* {{{ tui_sprite_set_visible(resource $sprite, bool $visible): void */
PHP_FUNCTION(tui_sprite_set_visible)
{
    zval *zsprite;
    zend_bool visible;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zsprite)
        Z_PARAM_BOOL(visible)
    ZEND_PARSE_PARAMETERS_END();

    tui_sprite *sprite = (tui_sprite *)zend_fetch_resource(Z_RES_P(zsprite), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!sprite) {
        RETURN_THROWS();
    }

    tui_sprite_set_visible(sprite, visible ? 1 : 0);
}
/* }}} */

/* {{{ tui_sprite_render(resource $buffer, resource $sprite): void */
PHP_FUNCTION(tui_sprite_render)
{
    zval *zbuffer, *zsprite;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_RESOURCE(zsprite)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    tui_sprite *sprite = (tui_sprite *)zend_fetch_resource(Z_RES_P(zsprite), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!sprite) {
        RETURN_THROWS();
    }

    tui_sprite_render(buffer, sprite);
}
/* }}} */

/* {{{ tui_sprite_get_bounds(resource $sprite): array */
PHP_FUNCTION(tui_sprite_get_bounds)
{
    zval *zsprite;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zsprite)
    ZEND_PARSE_PARAMETERS_END();

    tui_sprite *sprite = (tui_sprite *)zend_fetch_resource(Z_RES_P(zsprite), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!sprite) {
        RETURN_THROWS();
    }

    int x, y, width, height;
    tui_sprite_get_bounds(sprite, &x, &y, &width, &height);

    array_init(return_value);
    add_assoc_long(return_value, "x", x);
    add_assoc_long(return_value, "y", y);
    add_assoc_long(return_value, "width", width);
    add_assoc_long(return_value, "height", height);
}
/* }}} */

/* {{{ tui_sprite_collides(resource $a, resource $b): bool */
PHP_FUNCTION(tui_sprite_collides)
{
    zval *za, *zb;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(za)
        Z_PARAM_RESOURCE(zb)
    ZEND_PARSE_PARAMETERS_END();

    tui_sprite *a = (tui_sprite *)zend_fetch_resource(Z_RES_P(za), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!a) {
        RETURN_THROWS();
    }

    tui_sprite *b = (tui_sprite *)zend_fetch_resource(Z_RES_P(zb), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!b) {
        RETURN_THROWS();
    }

    RETURN_BOOL(tui_sprite_collides(a, b));
}
/* }}} */
