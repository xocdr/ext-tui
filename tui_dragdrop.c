/*
  +----------------------------------------------------------------------+
  | ext-tui: Drag and drop PHP bindings                                 |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"
#include "src/interaction/dragdrop.h"

/* {{{ proto bool tui_drag_start(int $x, int $y, string $type, string $data)
   Start a drag operation */
PHP_FUNCTION(tui_drag_start)
{
    zend_long x, y;
    zend_string *type;
    zend_string *data;

    ZEND_PARSE_PARAMETERS_START(4, 4)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_STR(type)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();

    int result = tui_drag_start(&tui_global_drag_ctx, NULL,
        (int)x, (int)y,
        ZSTR_VAL(type), ZSTR_VAL(data), ZSTR_LEN(data));

    RETURN_BOOL(result == 0);
}
/* }}} */

/* {{{ proto bool tui_drag_move(int $x, int $y)
   Update drag position */
PHP_FUNCTION(tui_drag_move)
{
    zend_long x, y;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
    ZEND_PARSE_PARAMETERS_END();

    int result = tui_drag_move(&tui_global_drag_ctx, (int)x, (int)y);
    RETURN_BOOL(result == 0);
}
/* }}} */

/* {{{ proto bool tui_drag_end(bool $dropped = true)
   End drag operation */
PHP_FUNCTION(tui_drag_end)
{
    zend_bool dropped = 1;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(dropped)
    ZEND_PARSE_PARAMETERS_END();

    int result = tui_drag_end(&tui_global_drag_ctx, dropped);
    RETURN_BOOL(result == 0);
}
/* }}} */

/* {{{ proto bool tui_drag_cancel()
   Cancel drag operation */
PHP_FUNCTION(tui_drag_cancel)
{
    ZEND_PARSE_PARAMETERS_NONE();

    int result = tui_drag_cancel(&tui_global_drag_ctx);
    RETURN_BOOL(result == 0);
}
/* }}} */

/* {{{ proto bool tui_drag_is_active()
   Check if drag is in progress */
PHP_FUNCTION(tui_drag_is_active)
{
    ZEND_PARSE_PARAMETERS_NONE();

    RETURN_BOOL(tui_drag_is_active(&tui_global_drag_ctx));
}
/* }}} */

/* {{{ proto ?string tui_drag_get_type()
   Get drag data type */
PHP_FUNCTION(tui_drag_get_type)
{
    ZEND_PARSE_PARAMETERS_NONE();

    const char *type = tui_drag_get_type(&tui_global_drag_ctx);
    if (type) {
        RETURN_STRING(type);
    }
    RETURN_NULL();
}
/* }}} */

/* {{{ proto ?string tui_drag_get_data()
   Get drag data */
PHP_FUNCTION(tui_drag_get_data)
{
    ZEND_PARSE_PARAMETERS_NONE();

    size_t len;
    const char *data = tui_drag_get_data(&tui_global_drag_ctx, &len);
    if (data && len > 0) {
        RETURN_STRINGL(data, len);
    }
    RETURN_NULL();
}
/* }}} */

/* {{{ proto array tui_drag_get_state()
   Get current drag state */
PHP_FUNCTION(tui_drag_get_state)
{
    ZEND_PARSE_PARAMETERS_NONE();

    array_init(return_value);

    const char *state_str;
    switch (tui_global_drag_ctx.state) {
        case TUI_DRAG_IDLE: state_str = "idle"; break;
        case TUI_DRAG_STARTED: state_str = "started"; break;
        case TUI_DRAG_DRAGGING: state_str = "dragging"; break;
        case TUI_DRAG_ENDED: state_str = "ended"; break;
        default: state_str = "unknown";
    }

    add_assoc_string(return_value, "state", state_str);
    add_assoc_long(return_value, "start_x", tui_global_drag_ctx.start_x);
    add_assoc_long(return_value, "start_y", tui_global_drag_ctx.start_y);
    add_assoc_long(return_value, "current_x", tui_global_drag_ctx.current_x);
    add_assoc_long(return_value, "current_y", tui_global_drag_ctx.current_y);

    const char *type = tui_drag_get_type(&tui_global_drag_ctx);
    if (type) {
        add_assoc_string(return_value, "type", type);
    } else {
        add_assoc_null(return_value, "type");
    }
}
/* }}} */
