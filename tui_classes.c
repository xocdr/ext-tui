/*
  +----------------------------------------------------------------------+
  | ext-tui: PHP class method implementations                           |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
  |                                                                      |
  | This file contains PHP method implementations for all TUI classes:  |
  |   - Color enum methods (toRgb, toAnsi, toAnsiBg, fromName)          |
  |   - TuiNode interface (abstract methods)                            |
  |   - TuiBox (ContainerNode) methods                                  |
  |   - TuiText (ContentNode) methods                                   |
  |   - TuiInstance methods (lifecycle, hooks, state)                   |
  |   - Focus and FocusManager methods                                  |
  |   - Context classes (Stdin, Stdout, Stderr)                         |
  |                                                                      |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"

/* ------------------------------------------------------------------
 * Color enum helper: Get named_colors index from enum case (by hex value)
 * ------------------------------------------------------------------ */
static int get_color_index_from_hex(const char *hex)
{
    if (!hex || hex[0] != '#' || strlen(hex) != 7) return -1;

    unsigned int r, g, b;
    if (sscanf(hex, "#%02x%02x%02x", &r, &g, &b) != 3) return -1;

    for (int i = 0; named_colors[i].name != NULL; i++) {
        if (named_colors[i].r == (uint8_t)r && named_colors[i].g == (uint8_t)g && named_colors[i].b == (uint8_t)b) {
            return i;
        }
    }
    return -1;
}

/* ------------------------------------------------------------------
 * Color enum methods
 * ------------------------------------------------------------------ */

/* Color::toRgb(): array{r: int, g: int, b: int} */
PHP_METHOD(Color, toRgb)
{
    ZEND_PARSE_PARAMETERS_NONE();

    zval *value = zend_enum_fetch_case_value(Z_OBJ_P(ZEND_THIS));
    if (!value || Z_TYPE_P(value) != IS_STRING) {
        zend_throw_exception(tui_validation_exception_ce,
            "Color enum value is not a valid hex string", 0);
        RETURN_THROWS();
    }

    int idx = get_color_index_from_hex(Z_STRVAL_P(value));
    if (idx < 0) {
        zend_throw_exception(tui_validation_exception_ce,
            "Color hex value not found in color table", 0);
        RETURN_THROWS();
    }

    array_init(return_value);
    add_assoc_long(return_value, "r", named_colors[idx].r);
    add_assoc_long(return_value, "g", named_colors[idx].g);
    add_assoc_long(return_value, "b", named_colors[idx].b);
}

/* Color::toAnsi(): string - ANSI escape sequence for foreground */
PHP_METHOD(Color, toAnsi)
{
    ZEND_PARSE_PARAMETERS_NONE();

    zval *value = zend_enum_fetch_case_value(Z_OBJ_P(ZEND_THIS));
    if (!value || Z_TYPE_P(value) != IS_STRING) {
        RETURN_EMPTY_STRING();
    }

    int idx = get_color_index_from_hex(Z_STRVAL_P(value));
    if (idx < 0) {
        RETURN_EMPTY_STRING();
    }

    char ansi[32];
    int len = snprintf(ansi, sizeof(ansi), "\033[38;2;%d;%d;%dm",
             named_colors[idx].r, named_colors[idx].g, named_colors[idx].b);
    if (len < 0 || (size_t)len >= sizeof(ansi)) {
        RETURN_EMPTY_STRING();
    }
    RETURN_STRING(ansi);
}

/* Color::toAnsiBg(): string - ANSI escape sequence for background */
PHP_METHOD(Color, toAnsiBg)
{
    ZEND_PARSE_PARAMETERS_NONE();

    zval *value = zend_enum_fetch_case_value(Z_OBJ_P(ZEND_THIS));
    if (!value || Z_TYPE_P(value) != IS_STRING) {
        RETURN_EMPTY_STRING();
    }

    int idx = get_color_index_from_hex(Z_STRVAL_P(value));
    if (idx < 0) {
        RETURN_EMPTY_STRING();
    }

    char ansi[32];
    int len = snprintf(ansi, sizeof(ansi), "\033[48;2;%d;%d;%dm",
             named_colors[idx].r, named_colors[idx].g, named_colors[idx].b);
    if (len < 0 || (size_t)len >= sizeof(ansi)) {
        RETURN_EMPTY_STRING();
    }
    RETURN_STRING(ansi);
}

/* Color::fromName(string $name): ?Color - Get color case by CSS name */
PHP_METHOD(Color, fromName)
{
    zend_string *name;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    /* Lookup by CSS name (case-insensitive) */
    for (int i = 0; named_colors[i].name != NULL; i++) {
        if (strcasecmp(ZSTR_VAL(name), named_colors[i].name) == 0) {
            /* Build hex value */
            char hex[8];
            int len = snprintf(hex, sizeof(hex), "#%02x%02x%02x",
                     named_colors[i].r, named_colors[i].g, named_colors[i].b);
            if (len < 0 || (size_t)len >= sizeof(hex)) {
                break;  /* Encoding error - return NULL */
            }

            /* Get enum case by value */
            zend_string *hex_str = zend_string_init(hex, 7, 0);
            zend_object *case_obj = NULL;

            zend_result result = zend_enum_get_case_by_value(&case_obj, tui_color_ce, 0, hex_str, true);
            if (result == SUCCESS && case_obj) {
                zend_string_release(hex_str);
                RETURN_OBJ_COPY(case_obj);
            }

            zend_string_release(hex_str);
            break;
        }
    }

    /* Color name not found - log a notice to help debugging */
    php_error_docref(NULL, E_NOTICE, "Unknown color name '%s'", ZSTR_VAL(name));
    RETURN_NULL();
}

/* Color enum arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_color_torgb, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_color_toansi, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_color_toansibg, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_color_fromname, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

const zend_function_entry tui_color_methods[] = {
    PHP_ME(Color, toRgb, arginfo_color_torgb, ZEND_ACC_PUBLIC)
    PHP_ME(Color, toAnsi, arginfo_color_toansi, ZEND_ACC_PUBLIC)
    PHP_ME(Color, toAnsiBg, arginfo_color_toansibg, ZEND_ACC_PUBLIC)
    PHP_ME(Color, fromName, arginfo_color_fromname, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};
/* ------------------------------------------------------------------
 * TuiNode interface - common interface for Box and Text nodes
 * ------------------------------------------------------------------ */

/* Interface arginfo definitions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuinode_getkey, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuinode_getid, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tuinode_setkey, 0, 1, static, 0)
    ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_tuinode_setid, 0, 1, static, 0)
    ZEND_ARG_TYPE_INFO(0, id, IS_STRING, 1)
ZEND_END_ARG_INFO()

/* TuiNode interface methods - abstract, implementations in Box/Text */
const zend_function_entry tui_node_interface_methods[] = {
    PHP_ABSTRACT_ME(TuiNode, getKey, arginfo_tuinode_getkey)
    PHP_ABSTRACT_ME(TuiNode, getId, arginfo_tuinode_getid)
    PHP_ABSTRACT_ME(TuiNode, setKey, arginfo_tuinode_setkey)
    PHP_ABSTRACT_ME(TuiNode, setId, arginfo_tuinode_setid)
    PHP_FE_END
};

/* ==========================================================================
 * TUIBOX CLASS
 *
 * TuiBox is the primary container element for building TUI layouts.
 * It corresponds to a flex container and supports all Yoga layout properties.
 *
 * Key Properties:
 * - Layout: flexDirection, alignItems, justifyContent, flexWrap
 * - Sizing: width, height, minWidth, maxWidth, minHeight, maxHeight
 * - Spacing: padding, paddingTop/Right/Bottom/Left, margin, gap
 * - Appearance: borderStyle, borderColor
 * - Behavior: focusable, focused, key, id
 *
 * Usage in PHP:
 *   $box = new Box([
 *       'flexDirection' => 'row',
 *       'padding' => 2,
 *       'borderStyle' => 'single',
 *   ]);
 *   $box->addChild(new Text('Hello'));
 *
 * Implements TuiNode interface for reconciler key/id matching.
 * ========================================================================== */

/* {{{ TuiBox::getKey(): ?string
 * Returns the reconciliation key for this node, used by the diff algorithm
 * to match nodes between renders and minimize DOM mutations. */
PHP_METHOD(TuiBox, getKey)
{
    zval rv;
    zval *key = zend_read_property(tui_box_ce, Z_OBJ_P(ZEND_THIS), "key", sizeof("key")-1, 1, &rv);
    if (Z_TYPE_P(key) == IS_STRING) {
        RETURN_STR(zend_string_copy(Z_STR_P(key)));
    }
    RETURN_NULL();
}
/* }}} */

/* {{{ TuiBox::getId(): ?string */
PHP_METHOD(TuiBox, getId)
{
    zval rv;
    zval *id = zend_read_property(tui_box_ce, Z_OBJ_P(ZEND_THIS), "id", sizeof("id")-1, 1, &rv);
    if (Z_TYPE_P(id) == IS_STRING) {
        RETURN_STR(zend_string_copy(Z_STR_P(id)));
    }
    RETURN_NULL();
}
/* }}} */

/* {{{ TuiBox::setKey(?string $key): static
 * Sets the reconciliation key for this node. Returns $this for fluent chaining. */
PHP_METHOD(TuiBox, setKey)
{
    zend_string *key = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR_OR_NULL(key)
    ZEND_PARSE_PARAMETERS_END();

    /* Validate length */
    if (key && ZSTR_LEN(key) > TUI_MAX_KEY_LENGTH) {
        zend_throw_exception_ex(tui_validation_exception_ce, 0,
            "Node key exceeds maximum length (%d bytes)", TUI_MAX_KEY_LENGTH);
        RETURN_THROWS();
    }

    if (key) {
        zend_update_property_str(tui_box_ce, Z_OBJ_P(ZEND_THIS), "key", sizeof("key")-1, key);
    } else {
        zend_update_property_null(tui_box_ce, Z_OBJ_P(ZEND_THIS), "key", sizeof("key")-1);
    }

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}
/* }}} */

/* {{{ TuiBox::setId(?string $id): static
 * Sets the id for this node. Returns $this for fluent chaining. */
PHP_METHOD(TuiBox, setId)
{
    zend_string *id = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR_OR_NULL(id)
    ZEND_PARSE_PARAMETERS_END();

    /* Validate length */
    if (id && ZSTR_LEN(id) > TUI_MAX_ID_LENGTH) {
        zend_throw_exception_ex(tui_validation_exception_ce, 0,
            "Node ID exceeds maximum length (%d bytes)", TUI_MAX_ID_LENGTH);
        RETURN_THROWS();
    }

    if (id) {
        zend_update_property_str(tui_box_ce, Z_OBJ_P(ZEND_THIS), "id", sizeof("id")-1, id);
    } else {
        zend_update_property_null(tui_box_ce, Z_OBJ_P(ZEND_THIS), "id", sizeof("id")-1);
    }

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}
/* }}} */

/* {{{ TuiBox::__construct(array $props = []) */
PHP_METHOD(TuiBox, __construct)
{
    zval *props = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(props)
    ZEND_PARSE_PARAMETERS_END();

    /* Initialize defaults */
    zend_update_property_string(tui_box_ce, Z_OBJ_P(ZEND_THIS), "flexDirection", sizeof("flexDirection")-1, "column");
    zend_update_property_null(tui_box_ce, Z_OBJ_P(ZEND_THIS), "alignItems", sizeof("alignItems")-1);
    zend_update_property_null(tui_box_ce, Z_OBJ_P(ZEND_THIS), "justifyContent", sizeof("justifyContent")-1);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "flexGrow", sizeof("flexGrow")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "flexShrink", sizeof("flexShrink")-1, 1);
    zend_update_property_null(tui_box_ce, Z_OBJ_P(ZEND_THIS), "width", sizeof("width")-1);
    zend_update_property_null(tui_box_ce, Z_OBJ_P(ZEND_THIS), "height", sizeof("height")-1);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "padding", sizeof("padding")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "paddingTop", sizeof("paddingTop")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "paddingBottom", sizeof("paddingBottom")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "paddingLeft", sizeof("paddingLeft")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "paddingRight", sizeof("paddingRight")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "paddingX", sizeof("paddingX")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "paddingY", sizeof("paddingY")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "margin", sizeof("margin")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "marginTop", sizeof("marginTop")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "marginBottom", sizeof("marginBottom")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "marginLeft", sizeof("marginLeft")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "marginRight", sizeof("marginRight")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "marginX", sizeof("marginX")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "marginY", sizeof("marginY")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "gap", sizeof("gap")-1, 0);
    zend_update_property_null(tui_box_ce, Z_OBJ_P(ZEND_THIS), "borderStyle", sizeof("borderStyle")-1);
    zend_update_property_null(tui_box_ce, Z_OBJ_P(ZEND_THIS), "borderColor", sizeof("borderColor")-1);
    zend_update_property_bool(tui_box_ce, Z_OBJ_P(ZEND_THIS), "focusable", sizeof("focusable")-1, 0);
    zend_update_property_bool(tui_box_ce, Z_OBJ_P(ZEND_THIS), "focused", sizeof("focused")-1, 0);

    /* Initialize empty children array */
    zval children;
    array_init(&children);
    zend_update_property(tui_box_ce, Z_OBJ_P(ZEND_THIS), "children", sizeof("children")-1, &children);
    zval_ptr_dtor(&children);

    /* Apply passed properties with validation */
    if (props) {
        HashTable *ht = Z_ARRVAL_P(props);
        zend_string *prop_key;
        zval *val;
        ZEND_HASH_FOREACH_STR_KEY_VAL(ht, prop_key, val) {
            if (prop_key) {
                /* Validate key and id length */
                if (zend_string_equals_literal(prop_key, "key") && Z_TYPE_P(val) == IS_STRING) {
                    if (Z_STRLEN_P(val) > TUI_MAX_KEY_LENGTH) {
                        zend_throw_exception_ex(tui_validation_exception_ce, 0,
                            "Node key exceeds maximum length (%d bytes)", TUI_MAX_KEY_LENGTH);
                        RETURN_THROWS();
                    }
                }
                if (zend_string_equals_literal(prop_key, "id") && Z_TYPE_P(val) == IS_STRING) {
                    if (Z_STRLEN_P(val) > TUI_MAX_ID_LENGTH) {
                        zend_throw_exception_ex(tui_validation_exception_ce, 0,
                            "Node ID exceeds maximum length (%d bytes)", TUI_MAX_ID_LENGTH);
                        RETURN_THROWS();
                    }
                }
                zend_update_property(tui_box_ce, Z_OBJ_P(ZEND_THIS), ZSTR_VAL(prop_key), ZSTR_LEN(prop_key), val);
            }
        } ZEND_HASH_FOREACH_END();
    }
}
/* }}} */

/* {{{ TuiBox::addChild(TuiBox|TuiText $child): self */
PHP_METHOD(TuiBox, addChild)
{
    zval *child;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT(child)
    ZEND_PARSE_PARAMETERS_END();

    /* Verify child is TuiBox or TuiText */
    if (!instanceof_function(Z_OBJCE_P(child), tui_box_ce) &&
        !instanceof_function(Z_OBJCE_P(child), tui_text_ce)) {
        zend_throw_exception(tui_validation_exception_ce,
            "Child must be TuiBox or TuiText", 0);
        RETURN_THROWS();
    }

    /* Get children array */
    zval rv;
    zval *children = zend_read_property(tui_box_ce, Z_OBJ_P(ZEND_THIS), "children", sizeof("children")-1, 1, &rv);

    if (Z_TYPE_P(children) != IS_ARRAY) {
        zval arr;
        array_init(&arr);
        zend_update_property(tui_box_ce, Z_OBJ_P(ZEND_THIS), "children", sizeof("children")-1, &arr);
        children = zend_read_property(tui_box_ce, Z_OBJ_P(ZEND_THIS), "children", sizeof("children")-1, 1, &rv);
        zval_ptr_dtor(&arr);
    }

    /* Add child to array */
    Z_TRY_ADDREF_P(child);
    add_next_index_zval(children, child);

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}
/* }}} */

/* TuiBox arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tuibox_construct, 0, 0, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, props, IS_ARRAY, 1, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tuibox_addchild, 0, 0, 1)
    ZEND_ARG_OBJ_INFO(0, child, TuiBox, 0)
ZEND_END_ARG_INFO()

const zend_function_entry tui_box_methods[] = {
    PHP_ME(TuiBox, __construct, arginfo_tuibox_construct, ZEND_ACC_PUBLIC)
    PHP_ME(TuiBox, addChild, arginfo_tuibox_addchild, ZEND_ACC_PUBLIC)
    PHP_ME(TuiBox, getKey, arginfo_tuinode_getkey, ZEND_ACC_PUBLIC)
    PHP_ME(TuiBox, getId, arginfo_tuinode_getid, ZEND_ACC_PUBLIC)
    PHP_ME(TuiBox, setKey, arginfo_tuinode_setkey, ZEND_ACC_PUBLIC)
    PHP_ME(TuiBox, setId, arginfo_tuinode_setid, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
/* ==========================================================================
 * TUITEXT CLASS
 *
 * TuiText represents text content within the TUI tree. Unlike Box, Text
 * cannot contain children - it only renders its content string.
 *
 * Key Properties:
 * - content: The text string to display
 * - color: Foreground color (CSS name, hex, or RGB array)
 * - backgroundColor: Background color
 * - bold, dim, italic, underline, inverse, strikethrough: Text styles
 * - wrap: Text wrapping mode ('wrap', 'truncate', 'truncate-start', etc.)
 *
 * Usage in PHP:
 *   $text = new Text('Hello, World!', [
 *       'color' => 'green',
 *       'bold' => true,
 *   ]);
 *
 * Text measurement is handled by the Yoga custom measure function,
 * which calculates width based on Unicode character widths.
 * ========================================================================== */

/* {{{ TuiText::getKey(): ?string
 * Returns the reconciliation key for this node. */
PHP_METHOD(TuiText, getKey)
{
    zval rv;
    zval *key = zend_read_property(tui_text_ce, Z_OBJ_P(ZEND_THIS), "key", sizeof("key")-1, 1, &rv);
    if (Z_TYPE_P(key) == IS_STRING) {
        RETURN_STR(zend_string_copy(Z_STR_P(key)));
    }
    RETURN_NULL();
}
/* }}} */

/* {{{ TuiText::getId(): ?string */
PHP_METHOD(TuiText, getId)
{
    zval rv;
    zval *id = zend_read_property(tui_text_ce, Z_OBJ_P(ZEND_THIS), "id", sizeof("id")-1, 1, &rv);
    if (Z_TYPE_P(id) == IS_STRING) {
        RETURN_STR(zend_string_copy(Z_STR_P(id)));
    }
    RETURN_NULL();
}
/* }}} */

/* {{{ TuiText::setKey(?string $key): static
 * Sets the reconciliation key for this node. Returns $this for fluent chaining. */
PHP_METHOD(TuiText, setKey)
{
    zend_string *key = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR_OR_NULL(key)
    ZEND_PARSE_PARAMETERS_END();

    /* Validate length */
    if (key && ZSTR_LEN(key) > TUI_MAX_KEY_LENGTH) {
        zend_throw_exception_ex(tui_validation_exception_ce, 0,
            "Node key exceeds maximum length (%d bytes)", TUI_MAX_KEY_LENGTH);
        RETURN_THROWS();
    }

    if (key) {
        zend_update_property_str(tui_text_ce, Z_OBJ_P(ZEND_THIS), "key", sizeof("key")-1, key);
    } else {
        zend_update_property_null(tui_text_ce, Z_OBJ_P(ZEND_THIS), "key", sizeof("key")-1);
    }

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}
/* }}} */

/* {{{ TuiText::setId(?string $id): static
 * Sets the id for this node. Returns $this for fluent chaining. */
PHP_METHOD(TuiText, setId)
{
    zend_string *id = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR_OR_NULL(id)
    ZEND_PARSE_PARAMETERS_END();

    /* Validate length */
    if (id && ZSTR_LEN(id) > TUI_MAX_ID_LENGTH) {
        zend_throw_exception_ex(tui_validation_exception_ce, 0,
            "Node ID exceeds maximum length (%d bytes)", TUI_MAX_ID_LENGTH);
        RETURN_THROWS();
    }

    if (id) {
        zend_update_property_str(tui_text_ce, Z_OBJ_P(ZEND_THIS), "id", sizeof("id")-1, id);
    } else {
        zend_update_property_null(tui_text_ce, Z_OBJ_P(ZEND_THIS), "id", sizeof("id")-1);
    }

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}
/* }}} */

/* {{{ TuiText::__construct(string $content = '', array $props = []) */
PHP_METHOD(TuiText, __construct)
{
    zend_string *content = NULL;
    zval *props = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 2)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(content)
        Z_PARAM_ARRAY(props)
    ZEND_PARSE_PARAMETERS_END();

    /* Validate content length to prevent DoS */
    if (content && ZSTR_LEN(content) > TUI_MAX_TEXT_LENGTH) {
        zend_throw_exception_ex(tui_validation_exception_ce, 0,
            "Text content exceeds maximum length (%d bytes)", TUI_MAX_TEXT_LENGTH);
        RETURN_THROWS();
    }

    /* Initialize defaults */
    zend_update_property_string(tui_text_ce, Z_OBJ_P(ZEND_THIS), "content", sizeof("content")-1, content ? ZSTR_VAL(content) : "");
    zend_update_property_null(tui_text_ce, Z_OBJ_P(ZEND_THIS), "color", sizeof("color")-1);
    zend_update_property_null(tui_text_ce, Z_OBJ_P(ZEND_THIS), "backgroundColor", sizeof("backgroundColor")-1);
    zend_update_property_bool(tui_text_ce, Z_OBJ_P(ZEND_THIS), "bold", sizeof("bold")-1, 0);
    zend_update_property_bool(tui_text_ce, Z_OBJ_P(ZEND_THIS), "dim", sizeof("dim")-1, 0);
    zend_update_property_bool(tui_text_ce, Z_OBJ_P(ZEND_THIS), "italic", sizeof("italic")-1, 0);
    zend_update_property_bool(tui_text_ce, Z_OBJ_P(ZEND_THIS), "underline", sizeof("underline")-1, 0);
    zend_update_property_bool(tui_text_ce, Z_OBJ_P(ZEND_THIS), "inverse", sizeof("inverse")-1, 0);
    zend_update_property_bool(tui_text_ce, Z_OBJ_P(ZEND_THIS), "strikethrough", sizeof("strikethrough")-1, 0);
    zend_update_property_null(tui_text_ce, Z_OBJ_P(ZEND_THIS), "wrap", sizeof("wrap")-1);

    /* Apply passed properties with validation */
    if (props) {
        HashTable *ht = Z_ARRVAL_P(props);
        zend_string *prop_key;
        zval *val;
        ZEND_HASH_FOREACH_STR_KEY_VAL(ht, prop_key, val) {
            if (prop_key) {
                /* Validate key length */
                if (zend_string_equals_literal(prop_key, "key") && Z_TYPE_P(val) == IS_STRING) {
                    if (Z_STRLEN_P(val) > TUI_MAX_KEY_LENGTH) {
                        zend_throw_exception_ex(tui_validation_exception_ce, 0,
                            "Node key exceeds maximum length (%d bytes)", TUI_MAX_KEY_LENGTH);
                        RETURN_THROWS();
                    }
                }
                /* Validate id length */
                if (zend_string_equals_literal(prop_key, "id") && Z_TYPE_P(val) == IS_STRING) {
                    if (Z_STRLEN_P(val) > TUI_MAX_ID_LENGTH) {
                        zend_throw_exception_ex(tui_validation_exception_ce, 0,
                            "Node id exceeds maximum length (%d bytes)", TUI_MAX_ID_LENGTH);
                        RETURN_THROWS();
                    }
                }
                zend_update_property(tui_text_ce, Z_OBJ_P(ZEND_THIS), ZSTR_VAL(prop_key), ZSTR_LEN(prop_key), val);
            }
        } ZEND_HASH_FOREACH_END();
    }
}
/* }}} */

/* TuiText arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tuitext_construct, 0, 0, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, content, IS_STRING, 0, "\"\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, props, IS_ARRAY, 1, "[]")
ZEND_END_ARG_INFO()

const zend_function_entry tui_text_methods[] = {
    PHP_ME(TuiText, __construct, arginfo_tuitext_construct, ZEND_ACC_PUBLIC)
    PHP_ME(TuiText, getKey, arginfo_tuinode_getkey, ZEND_ACC_PUBLIC)
    PHP_ME(TuiText, getId, arginfo_tuinode_getid, ZEND_ACC_PUBLIC)
    PHP_ME(TuiText, setKey, arginfo_tuinode_setkey, ZEND_ACC_PUBLIC)
    PHP_ME(TuiText, setId, arginfo_tuinode_setid, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* ==========================================================================
 * TUIINSTANCE CLASS
 *
 * TuiInstance represents a running TUI application. It's returned by
 * tui_render() and provides methods for controlling the render lifecycle,
 * managing state, and handling events.
 *
 * Lifecycle Methods:
 * - rerender(): Force a re-render of the UI tree
 * - unmount(): Stop and clean up the application
 * - waitUntilExit(): Block until the application exits
 * - exit(code): Request application exit with optional exit code
 *
 * State Management (React-like hooks):
 * - state(initial): Create a state slot, returns [value, index]
 * - setState(index, value): Update state at given index
 *
 * Event Handling:
 * - onInput(handler): Register keyboard input handler
 * - setInputHandler(handler): Legacy input handler (deprecated)
 * - setFocusHandler(handler): Handle focus changes
 * - setResizeHandler(handler): Handle terminal resize
 * - setTickHandler(handler): Called on each event loop tick
 *
 * Focus Management:
 * - focus(): Get Focus helper for programmatic focus
 * - focusManager(): Get FocusManager for navigation
 * - focusNext(), focusPrev(): Navigate focus
 *
 * Timers:
 * - addTimer(intervalMs, callback): Add recurring timer
 * - removeTimer(timerId): Remove a timer
 *
 * The Instance wraps an internal tui_app structure that holds:
 * - The current node tree
 * - State slots (dynamic array, max: tui.max_states)
 * - Timer callbacks (dynamic array, max: tui.max_timers)
 * - Focus state and navigation
 * - Output buffer and terminal state
 * ========================================================================== */

/* {{{ TuiInstance::rerender(): void
 * Triggers a complete re-render of the UI tree. This calls the original
 * render callback with the Instance, rebuilds the C node tree, runs
 * layout calculation, and outputs the result. */
PHP_METHOD(TuiInstance, rerender)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (obj->app) {
        /* Use rerender_callback to properly rebuild tree with Instance parameter */
        if (obj->app->rerender_callback) {
            obj->app->rerender_callback(obj->app);
        }
        tui_app_render_tree(obj->app);
    }
}
/* }}} */

/* {{{ TuiInstance::unmount(): void */
PHP_METHOD(TuiInstance, unmount)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (obj->app) {
        tui_app_stop(obj->app);
        tui_app_destroy(obj->app);
        obj->app = NULL;
    }
}
/* }}} */

/* {{{ TuiInstance::waitUntilExit(): void */
PHP_METHOD(TuiInstance, waitUntilExit)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (obj->app) {
        tui_app_wait_until_exit(obj->app);
    }
}
/* }}} */

/* {{{ TuiInstance::exit(int $code = 0): void */
PHP_METHOD(TuiInstance, exit)
{
    zend_long code = 0;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(code)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (obj->app) {
        tui_app_exit(obj->app, (int)code);
    }
}
/* }}} */

/* {{{ TuiInstance::useState(mixed $initial): array */
PHP_METHOD(TuiInstance, useState)
{
    zval *initial;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(initial)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    tui_app *app = obj->app;
    int is_new = 0;
    int index = tui_app_get_or_create_state_slot(app, initial, &is_new);

    if (index < 0) {
        zend_throw_exception(tui_state_limit_exception_ce,
            "Failed to allocate state slot: maximum states exceeded", 0);
        RETURN_THROWS();
    }

    /* Create setter closure if this is a new state slot */
    if (is_new) {
        /* Create a closure that calls our state setter
         * We use a simple approach: create an internal callable that captures
         * the app pointer and index via a wrapper object */
        zval setter_obj;
        object_init(&setter_obj);

        /* Store app pointer and index as properties we can retrieve later */
        zend_update_property_long(Z_OBJCE(setter_obj), Z_OBJ(setter_obj),
            "_app_ptr", sizeof("_app_ptr")-1, (zend_long)(uintptr_t)app);
        zend_update_property_long(Z_OBJCE(setter_obj), Z_OBJ(setter_obj),
            "_index", sizeof("_index")-1, index);

        /* For now, store a simple callable array - the actual setter logic
         * will be handled when called via a special internal function */
        ZVAL_COPY(&app->states[index].setter, &setter_obj);
        zval_ptr_dtor(&setter_obj);
    }

    /* Return [value, setter] array */
    array_init(return_value);

    /* Add current value */
    zval value_copy;
    ZVAL_COPY(&value_copy, &app->states[index].value);
    add_next_index_zval(return_value, &value_copy);

    /* Add setter (we need a callable - for now return the index for manual handling) */
    add_next_index_long(return_value, index);
}
/* }}} */

/* {{{ TuiInstance::useInput(callable $handler, array $options = []): void */
PHP_METHOD(TuiInstance, useInput)
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    zval *options = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_FUNC(fci, fcc)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(options)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    /* Check isActive option */
    zend_bool is_active = 1;
    if (options) {
        zval *val = zend_hash_str_find(Z_ARRVAL_P(options), "isActive", sizeof("isActive")-1);
        if (val) {
            is_active = zend_is_true(val);
        }
    }

    if (is_active) {
        tui_app_set_input_handler(obj->app, &fci, &fcc);
    }
}
/* }}} */

/* {{{ TuiInstance::useFocus(array $options = []): Focus */
PHP_METHOD(TuiInstance, useFocus)
{
    zval *options = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(options)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    /* Create Focus object */
    object_init_ex(return_value, tui_focus_ce);
    tui_focus_object *focus_obj = Z_TUI_FOCUS_P(return_value);
    focus_obj->app = obj->app;

    /* Check autoFocus option */
    if (options) {
        zval *auto_focus = zend_hash_str_find(Z_ARRVAL_P(options), "autoFocus", sizeof("autoFocus")-1);
        if (auto_focus && zend_is_true(auto_focus)) {
            /* Auto-focus the first focusable element */
            tui_app_focus_next(obj->app);
        }
    }

    /* Set isFocused property based on current focus state */
    zend_bool is_focused = obj->app->focused_node != NULL;
    zend_update_property_bool(tui_focus_ce, Z_OBJ_P(return_value),
        "isFocused", sizeof("isFocused")-1, is_focused);
}
/* }}} */

/* {{{ TuiInstance::useFocusManager(): FocusManager */
PHP_METHOD(TuiInstance, useFocusManager)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    /* Create FocusManager object */
    object_init_ex(return_value, tui_focus_manager_ce);
    tui_focus_manager_object *fm_obj = Z_TUI_FOCUS_MANAGER_P(return_value);
    fm_obj->app = obj->app;
}
/* }}} */

/* {{{ TuiInstance::useStdin(): StdinContext */
PHP_METHOD(TuiInstance, useStdin)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    /* Create StdinContext object */
    object_init_ex(return_value, tui_stdin_context_ce);

    /* Set properties */
    zend_update_property_bool(tui_stdin_context_ce, Z_OBJ_P(return_value),
        "isRawModeSupported", sizeof("isRawModeSupported")-1, isatty(STDIN_FILENO));
}
/* }}} */

/* {{{ TuiInstance::useStdout(): StdoutContext */
PHP_METHOD(TuiInstance, useStdout)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    /* Create StdoutContext object */
    object_init_ex(return_value, tui_stdout_context_ce);

    /* Set properties */
    zend_update_property_long(tui_stdout_context_ce, Z_OBJ_P(return_value),
        "columns", sizeof("columns")-1, obj->app->width);
    zend_update_property_long(tui_stdout_context_ce, Z_OBJ_P(return_value),
        "rows", sizeof("rows")-1, obj->app->height);
    zend_update_property_bool(tui_stdout_context_ce, Z_OBJ_P(return_value),
        "isTTY", sizeof("isTTY")-1, isatty(STDOUT_FILENO));
}
/* }}} */

/* {{{ TuiInstance::useStderr(): StderrContext */
PHP_METHOD(TuiInstance, useStderr)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    /* Create StderrContext object */
    object_init_ex(return_value, tui_stderr_context_ce);

    /* Set properties */
    zend_update_property_long(tui_stderr_context_ce, Z_OBJ_P(return_value),
        "columns", sizeof("columns")-1, obj->app->width);
    zend_update_property_long(tui_stderr_context_ce, Z_OBJ_P(return_value),
        "rows", sizeof("rows")-1, obj->app->height);
    zend_update_property_bool(tui_stderr_context_ce, Z_OBJ_P(return_value),
        "isTTY", sizeof("isTTY")-1, isatty(STDERR_FILENO));
}
/* }}} */

/* {{{ TuiInstance::getTerminalSize(): array */
PHP_METHOD(TuiInstance, getTerminalSize)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    array_init(return_value);
    add_assoc_long(return_value, "columns", obj->app->width);
    add_assoc_long(return_value, "rows", obj->app->height);
}
/* }}} */

/* {{{ TuiInstance::getSize(): array - Get terminal size with width/height aliases */
PHP_METHOD(TuiInstance, getSize)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    array_init(return_value);
    add_assoc_long(return_value, "width", obj->app->width);
    add_assoc_long(return_value, "height", obj->app->height);
    add_assoc_long(return_value, "columns", obj->app->width);
    add_assoc_long(return_value, "rows", obj->app->height);
}
/* }}} */

/* {{{ TuiInstance::setState(int $index, mixed $value): void - Internal for setter */
PHP_METHOD(TuiInstance, setState)
{
    zend_long index;
    zval *value;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(index)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }
    if (index < 0 || index >= obj->app->state_count) {
        zend_throw_exception(tui_validation_exception_ce,
            "Invalid state index", 0);
        RETURN_THROWS();
    }

    /* Update state value */
    zval_ptr_dtor(&obj->app->states[index].value);
    ZVAL_COPY(&obj->app->states[index].value, value);

    /* Trigger re-render */
    obj->app->render_pending = 1;
}
/* }}} */

/* {{{ TuiInstance::focusNext(): void */
PHP_METHOD(TuiInstance, focusNext)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (obj->app) {
        tui_app_focus_next(obj->app);
    }
}
/* }}} */

/* {{{ TuiInstance::focusPrev(): void */
PHP_METHOD(TuiInstance, focusPrev)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (obj->app) {
        tui_app_focus_prev(obj->app);
    }
}
/* }}} */

/* Helper: find node by ID in tree (with depth limit to prevent stack overflow) */
static tui_node* find_node_by_id_in_tree_impl(tui_node *node, const char *id, int depth)
{
    if (!node || !id) return NULL;

    /* Prevent stack overflow from deeply nested trees */
    if (depth > MAX_TREE_DEPTH) return NULL;

    if (node->id && strcmp(node->id, id) == 0) {
        return node;
    }

    for (int i = 0; i < node->child_count; i++) {
        tui_node *found = find_node_by_id_in_tree_impl(node->children[i], id, depth + 1);
        if (found) return found;
    }

    return NULL;
}

static tui_node* find_node_by_id_in_tree(tui_node *node, const char *id)
{
    return find_node_by_id_in_tree_impl(node, id, 0);
}

/* {{{ TuiInstance::measureElement(string $id): ?array */
PHP_METHOD(TuiInstance, measureElement)
{
    zend_string *id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(id)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app || !obj->app->root_node) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    tui_node *node = find_node_by_id_in_tree(obj->app->root_node, ZSTR_VAL(id));
    if (!node) {
        RETURN_NULL();
    }

    /* Return computed layout from Yoga */
    array_init(return_value);
    add_assoc_long(return_value, "x", (zend_long)node->x);
    add_assoc_long(return_value, "y", (zend_long)node->y);
    add_assoc_long(return_value, "width", (zend_long)node->width);
    add_assoc_long(return_value, "height", (zend_long)node->height);
}
/* }}} */

/* {{{ TuiInstance::setInputHandler(callable $handler): void */
PHP_METHOD(TuiInstance, setInputHandler)
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }
    tui_app_set_input_handler(obj->app, &fci, &fcc);
}
/* }}} */

/* {{{ TuiInstance::setFocusHandler(callable $handler): void */
PHP_METHOD(TuiInstance, setFocusHandler)
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }
    tui_app_set_focus_handler(obj->app, &fci, &fcc);
}
/* }}} */

/* {{{ TuiInstance::setResizeHandler(callable $handler): void */
PHP_METHOD(TuiInstance, setResizeHandler)
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }
    tui_app_set_resize_handler(obj->app, &fci, &fcc);
}
/* }}} */

/* {{{ TuiInstance::setTickHandler(callable $handler): void */
PHP_METHOD(TuiInstance, setTickHandler)
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }
    tui_app_set_tick_handler(obj->app, &fci, &fcc);
}
/* }}} */

/* {{{ TuiInstance::addTimer(int $intervalMs, callable $callback): int */
PHP_METHOD(TuiInstance, addTimer)
{
    zend_long interval_ms;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(interval_ms)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    int timer_id = tui_app_add_timer(obj->app, (int)interval_ms, &fci, &fcc);
    RETURN_LONG(timer_id);
}
/* }}} */

/* {{{ TuiInstance::removeTimer(int $timerId): void */
PHP_METHOD(TuiInstance, removeTimer)
{
    zend_long timer_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(timer_id)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }
    tui_app_remove_timer(obj->app, (int)timer_id);
}
/* }}} */

/* {{{ TuiInstance::clear(): void */
PHP_METHOD(TuiInstance, clear)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(tui_instance_destroyed_exception_ce,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }
    if (obj->app->buffer) {
        tui_buffer_clear(obj->app->buffer);
        tui_output_flush(obj->app->output);
    }
}
/* }}} */

/* {{{ TuiInstance::getCapturedOutput(): ?string */
PHP_METHOD(TuiInstance, getCapturedOutput)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (obj->app && obj->app->captured_output && obj->app->captured_output_len > 0) {
        RETURN_STRINGL(obj->app->captured_output, obj->app->captured_output_len);
    }
    RETURN_NULL();
}
/* }}} */

/* {{{ TuiInstance::captureFrame(): ?string
 * Returns the current screen buffer as an ANSI-encoded string.
 * Useful for screen recording - captures the complete terminal content
 * including colors and styles as ANSI escape codes.
 */
PHP_METHOD(TuiInstance, captureFrame)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app || !obj->app->buffer) {
        RETURN_NULL();
    }

    char *frame = tui_buffer_to_string(obj->app->buffer);
    if (!frame) {
        RETURN_NULL();
    }

    zend_string *result = zend_string_init(frame, strlen(frame), 0);
    free(frame);
    RETURN_STR(result);
}
/* }}} */

/* TuiInstance arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_rerender, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_unmount, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_waituntilexit, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_exit, 0, 0, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, code, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_usestate, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, initial, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_useinput, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tuiinstance_usefocus, 0, 0, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tuiinstance_usefocusmanager, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tuiinstance_usestdin, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tuiinstance_usestdout, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tuiinstance_usestderr, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_getterminalsize, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_getsize, 0, 0, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_setstate, 0, 2, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_focusnext, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_focusprev, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_measureelement, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_TYPE_INFO(0, id, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_setinputhandler, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_setfocushandler, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_setresizehandler, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_settickhandler, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_addtimer, 0, 2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, intervalMs, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_removetimer, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, timerId, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_clear, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_getcapturedoutput, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_captureframe, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

const zend_function_entry tui_instance_methods[] = {
    PHP_ME(TuiInstance, rerender, arginfo_tuiinstance_rerender, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, unmount, arginfo_tuiinstance_unmount, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, waitUntilExit, arginfo_tuiinstance_waituntilexit, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, exit, arginfo_tuiinstance_exit, ZEND_ACC_PUBLIC)
    /* New method names */
    PHP_MALIAS(TuiInstance, state, useState, arginfo_tuiinstance_usestate, ZEND_ACC_PUBLIC)
    PHP_MALIAS(TuiInstance, onInput, useInput, arginfo_tuiinstance_useinput, ZEND_ACC_PUBLIC)
    PHP_MALIAS(TuiInstance, focus, useFocus, arginfo_tuiinstance_usefocus, ZEND_ACC_PUBLIC)
    PHP_MALIAS(TuiInstance, focusManager, useFocusManager, arginfo_tuiinstance_usefocusmanager, ZEND_ACC_PUBLIC)
    PHP_MALIAS(TuiInstance, stdin, useStdin, arginfo_tuiinstance_usestdin, ZEND_ACC_PUBLIC)
    PHP_MALIAS(TuiInstance, stdout, useStdout, arginfo_tuiinstance_usestdout, ZEND_ACC_PUBLIC)
    PHP_MALIAS(TuiInstance, stderr, useStderr, arginfo_tuiinstance_usestderr, ZEND_ACC_PUBLIC)
    /* Deprecated aliases (old names) */
    PHP_ME(TuiInstance, useState, arginfo_tuiinstance_usestate, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
    PHP_ME(TuiInstance, useInput, arginfo_tuiinstance_useinput, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
    PHP_ME(TuiInstance, useFocus, arginfo_tuiinstance_usefocus, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
    PHP_ME(TuiInstance, useFocusManager, arginfo_tuiinstance_usefocusmanager, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
    PHP_ME(TuiInstance, useStdin, arginfo_tuiinstance_usestdin, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
    PHP_ME(TuiInstance, useStdout, arginfo_tuiinstance_usestdout, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
    PHP_ME(TuiInstance, useStderr, arginfo_tuiinstance_usestderr, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
    PHP_ME(TuiInstance, getTerminalSize, arginfo_tuiinstance_getterminalsize, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, getSize, arginfo_tuiinstance_getsize, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, setState, arginfo_tuiinstance_setstate, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, focusNext, arginfo_tuiinstance_focusnext, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, focusPrev, arginfo_tuiinstance_focusprev, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, measureElement, arginfo_tuiinstance_measureelement, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, setInputHandler, arginfo_tuiinstance_setinputhandler, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, setFocusHandler, arginfo_tuiinstance_setfocushandler, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, setResizeHandler, arginfo_tuiinstance_setresizehandler, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, setTickHandler, arginfo_tuiinstance_settickhandler, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, addTimer, arginfo_tuiinstance_addtimer, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, removeTimer, arginfo_tuiinstance_removetimer, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, clear, arginfo_tuiinstance_clear, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, getCapturedOutput, arginfo_tuiinstance_getcapturedoutput, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, captureFrame, arginfo_tuiinstance_captureframe, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* ------------------------------------------------------------------
 * TuiFocus class
 * ------------------------------------------------------------------ */

/* {{{ Focus::focus(string $id): void */
PHP_METHOD(Focus, focus)
{
    zend_string *id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(id)
    ZEND_PARSE_PARAMETERS_END();

    tui_focus_object *intern = Z_TUI_FOCUS_P(ZEND_THIS);
    if (intern->app) {
        tui_app_focus_by_id(intern->app, ZSTR_VAL(id));
    }
}
/* }}} */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_focus_focus, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, id, IS_STRING, 0)
ZEND_END_ARG_INFO()

const zend_function_entry tui_focus_methods[] = {
    PHP_ME(Focus, focus, arginfo_focus_focus, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* ------------------------------------------------------------------
 * TuiFocusManager class
 * ------------------------------------------------------------------ */

/* {{{ FocusManager::focusNext(): void */
PHP_METHOD(FocusManager, focusNext)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_focus_manager_object *intern = Z_TUI_FOCUS_MANAGER_P(ZEND_THIS);
    if (intern->app) {
        tui_app_focus_next(intern->app);
    }
}
/* }}} */

/* {{{ FocusManager::focusPrevious(): void */
PHP_METHOD(FocusManager, focusPrevious)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_focus_manager_object *intern = Z_TUI_FOCUS_MANAGER_P(ZEND_THIS);
    if (intern->app) {
        tui_app_focus_prev(intern->app);
    }
}
/* }}} */

/* {{{ FocusManager::focus(string $id): void */
PHP_METHOD(FocusManager, focus)
{
    zend_string *id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(id)
    ZEND_PARSE_PARAMETERS_END();

    tui_focus_manager_object *intern = Z_TUI_FOCUS_MANAGER_P(ZEND_THIS);
    if (intern->app) {
        tui_app_focus_by_id(intern->app, ZSTR_VAL(id));
    }
}
/* }}} */

/* {{{ FocusManager::enableFocus(): void */
PHP_METHOD(FocusManager, enableFocus)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_focus_manager_object *intern = Z_TUI_FOCUS_MANAGER_P(ZEND_THIS);
    if (intern->app) {
        tui_app_enable_focus(intern->app);
    }
}
/* }}} */

/* {{{ FocusManager::disableFocus(): void */
PHP_METHOD(FocusManager, disableFocus)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_focus_manager_object *intern = Z_TUI_FOCUS_MANAGER_P(ZEND_THIS);
    if (intern->app) {
        tui_app_disable_focus(intern->app);
    }
}
/* }}} */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_focusmanager_focusnext, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_focusmanager_focusprevious, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_focusmanager_focus, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, id, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_focusmanager_enablefocus, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_focusmanager_disablefocus, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

const zend_function_entry tui_focus_manager_methods[] = {
    PHP_ME(FocusManager, focusNext, arginfo_focusmanager_focusnext, ZEND_ACC_PUBLIC)
    PHP_ME(FocusManager, focusPrevious, arginfo_focusmanager_focusprevious, ZEND_ACC_PUBLIC)
    PHP_ME(FocusManager, focus, arginfo_focusmanager_focus, ZEND_ACC_PUBLIC)
    PHP_ME(FocusManager, enableFocus, arginfo_focusmanager_enablefocus, ZEND_ACC_PUBLIC)
    PHP_ME(FocusManager, disableFocus, arginfo_focusmanager_disablefocus, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* ------------------------------------------------------------------
 * Stream Context classes (StdinContext, StdoutContext, StderrContext)
 * ------------------------------------------------------------------ */

/* {{{ StdoutContext::write(string $data): void */
PHP_METHOD(StdoutContext, write)
{
    zend_string *data;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();

    /* Write to stdout (outside of TUI rendering) */
    fwrite(ZSTR_VAL(data), 1, ZSTR_LEN(data), stdout);
    fflush(stdout);
}
/* }}} */

/* {{{ StderrContext::write(string $data): void */
PHP_METHOD(StderrContext, write)
{
    zend_string *data;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();

    /* Write to stderr */
    fwrite(ZSTR_VAL(data), 1, ZSTR_LEN(data), stderr);
    fflush(stderr);
}
/* }}} */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_context_write, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

const zend_function_entry tui_stdin_context_methods[] = {
    PHP_FE_END
};

const zend_function_entry tui_stdout_context_methods[] = {
    PHP_ME(StdoutContext, write, arginfo_context_write, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

const zend_function_entry tui_stderr_context_methods[] = {
    PHP_ME(StderrContext, write, arginfo_context_write, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

