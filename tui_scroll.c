/*
  +----------------------------------------------------------------------+
  | ext-tui: Smooth scrolling PHP bindings                              |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"
#include "src/scroll/smooth.h"

/* Resource type ID (defined in tui.c) */
extern int le_tui_scroll_animation;

/* Resource destructor */
void tui_scroll_animation_dtor(zend_resource *res)
{
    tui_scroll_animation *anim = (tui_scroll_animation *)res->ptr;
    if (anim) {
        efree(anim);
    }
}

/* {{{ proto resource tui_scroll_create()
   Create a smooth scroll animation controller */
PHP_FUNCTION(tui_scroll_create)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_scroll_animation *anim = emalloc(sizeof(tui_scroll_animation));
    tui_scroll_anim_init(anim);

    RETURN_RES(zend_register_resource(anim, le_tui_scroll_animation));
}
/* }}} */

/* {{{ proto void tui_scroll_set_spring(resource anim, float stiffness, float damping)
   Set spring physics parameters */
PHP_FUNCTION(tui_scroll_set_spring)
{
    zval *zanim;
    double stiffness, damping;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_RESOURCE(zanim)
        Z_PARAM_DOUBLE(stiffness)
        Z_PARAM_DOUBLE(damping)
    ZEND_PARSE_PARAMETERS_END();

    tui_scroll_animation *anim = (tui_scroll_animation *)zend_fetch_resource(
        Z_RES_P(zanim), TUI_SCROLL_ANIM_RES_NAME, le_tui_scroll_animation);

    if (anim) {
        tui_scroll_anim_set_spring(anim, (float)stiffness, (float)damping);
    }
}
/* }}} */

/* {{{ proto void tui_scroll_set_target(resource anim, float x, float y)
   Set scroll target position */
PHP_FUNCTION(tui_scroll_set_target)
{
    zval *zanim;
    double x, y;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_RESOURCE(zanim)
        Z_PARAM_DOUBLE(x)
        Z_PARAM_DOUBLE(y)
    ZEND_PARSE_PARAMETERS_END();

    tui_scroll_animation *anim = (tui_scroll_animation *)zend_fetch_resource(
        Z_RES_P(zanim), TUI_SCROLL_ANIM_RES_NAME, le_tui_scroll_animation);

    if (anim) {
        tui_scroll_anim_set_target(anim, (float)x, (float)y);
    }
}
/* }}} */

/* {{{ proto void tui_scroll_by(resource anim, float dx, float dy)
   Add to scroll target (incremental scrolling) */
PHP_FUNCTION(tui_scroll_by)
{
    zval *zanim;
    double dx, dy;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_RESOURCE(zanim)
        Z_PARAM_DOUBLE(dx)
        Z_PARAM_DOUBLE(dy)
    ZEND_PARSE_PARAMETERS_END();

    tui_scroll_animation *anim = (tui_scroll_animation *)zend_fetch_resource(
        Z_RES_P(zanim), TUI_SCROLL_ANIM_RES_NAME, le_tui_scroll_animation);

    if (anim) {
        tui_scroll_anim_scroll_by(anim, (float)dx, (float)dy);
    }
}
/* }}} */

/* {{{ proto bool tui_scroll_update(resource anim, float dt)
   Update animation for one frame. Returns true if still animating. */
PHP_FUNCTION(tui_scroll_update)
{
    zval *zanim;
    double dt;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zanim)
        Z_PARAM_DOUBLE(dt)
    ZEND_PARSE_PARAMETERS_END();

    tui_scroll_animation *anim = (tui_scroll_animation *)zend_fetch_resource(
        Z_RES_P(zanim), TUI_SCROLL_ANIM_RES_NAME, le_tui_scroll_animation);

    if (!anim) {
        RETURN_FALSE;
    }

    RETURN_BOOL(tui_scroll_anim_update(anim, (float)dt));
}
/* }}} */

/* {{{ proto void tui_scroll_snap(resource anim)
   Immediately snap to target position */
PHP_FUNCTION(tui_scroll_snap)
{
    zval *zanim;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zanim)
    ZEND_PARSE_PARAMETERS_END();

    tui_scroll_animation *anim = (tui_scroll_animation *)zend_fetch_resource(
        Z_RES_P(zanim), TUI_SCROLL_ANIM_RES_NAME, le_tui_scroll_animation);

    if (anim) {
        tui_scroll_anim_snap(anim);
    }
}
/* }}} */

/* {{{ proto array tui_scroll_get_position(resource anim)
   Get current scroll position as [x, y] */
PHP_FUNCTION(tui_scroll_get_position)
{
    zval *zanim;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zanim)
    ZEND_PARSE_PARAMETERS_END();

    tui_scroll_animation *anim = (tui_scroll_animation *)zend_fetch_resource(
        Z_RES_P(zanim), TUI_SCROLL_ANIM_RES_NAME, le_tui_scroll_animation);

    if (!anim) {
        RETURN_NULL();
    }

    float x, y;
    tui_scroll_anim_get_position(anim, &x, &y);

    array_init(return_value);
    add_assoc_double(return_value, "x", (double)x);
    add_assoc_double(return_value, "y", (double)y);
}
/* }}} */

/* {{{ proto bool tui_scroll_is_animating(resource anim)
   Check if animation is in progress */
PHP_FUNCTION(tui_scroll_is_animating)
{
    zval *zanim;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zanim)
    ZEND_PARSE_PARAMETERS_END();

    tui_scroll_animation *anim = (tui_scroll_animation *)zend_fetch_resource(
        Z_RES_P(zanim), TUI_SCROLL_ANIM_RES_NAME, le_tui_scroll_animation);

    if (!anim) {
        RETURN_FALSE;
    }

    RETURN_BOOL(tui_scroll_anim_is_animating(anim));
}
/* }}} */

/* {{{ proto float tui_scroll_progress(resource anim)
   Get animation progress (0.0 to 1.0) */
PHP_FUNCTION(tui_scroll_progress)
{
    zval *zanim;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zanim)
    ZEND_PARSE_PARAMETERS_END();

    tui_scroll_animation *anim = (tui_scroll_animation *)zend_fetch_resource(
        Z_RES_P(zanim), TUI_SCROLL_ANIM_RES_NAME, le_tui_scroll_animation);

    if (!anim) {
        RETURN_DOUBLE(1.0);
    }

    RETURN_DOUBLE((double)tui_scroll_anim_progress(anim));
}
/* }}} */

/* {{{ proto void tui_scroll_destroy(resource anim)
   Destroy scroll animation resource */
PHP_FUNCTION(tui_scroll_destroy)
{
    zval *zanim;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zanim)
    ZEND_PARSE_PARAMETERS_END();

    tui_scroll_animation *anim = (tui_scroll_animation *)zend_fetch_resource(
        Z_RES_P(zanim), TUI_SCROLL_ANIM_RES_NAME, le_tui_scroll_animation);

    if (anim) {
        zend_list_close(Z_RES_P(zanim));
    }
}
/* }}} */
