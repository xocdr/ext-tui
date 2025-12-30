/*
  +----------------------------------------------------------------------+
  | ext-tui: Accessibility PHP bindings                                 |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"
#include "src/a11y/accessibility.h"

/* {{{ proto bool tui_announce(string $message, string $priority = 'polite')
   Send announcement to screen reader */
PHP_FUNCTION(tui_announce)
{
    zend_string *message;
    zend_string *priority_str = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(message)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(priority_str)
    ZEND_PARSE_PARAMETERS_END();

    tui_announce_priority priority = TUI_ANNOUNCE_POLITE;
    if (priority_str) {
        if (strcmp(ZSTR_VAL(priority_str), "assertive") == 0) {
            priority = TUI_ANNOUNCE_ASSERTIVE;
        }
    }

    RETURN_BOOL(tui_announce(ZSTR_VAL(message), priority) == 0);
}
/* }}} */

/* {{{ proto bool tui_prefers_reduced_motion()
   Check if user prefers reduced motion */
PHP_FUNCTION(tui_prefers_reduced_motion)
{
    ZEND_PARSE_PARAMETERS_NONE();

    RETURN_BOOL(tui_prefers_reduced_motion());
}
/* }}} */

/* {{{ proto bool tui_prefers_high_contrast()
   Check if user prefers high contrast */
PHP_FUNCTION(tui_prefers_high_contrast)
{
    ZEND_PARSE_PARAMETERS_NONE();

    RETURN_BOOL(tui_prefers_high_contrast());
}
/* }}} */

/* {{{ proto array tui_get_accessibility_features()
   Get accessibility feature flags */
PHP_FUNCTION(tui_get_accessibility_features)
{
    ZEND_PARSE_PARAMETERS_NONE();

    int features = tui_get_accessibility_features();

    array_init(return_value);
    add_assoc_bool(return_value, "reduced_motion", (features & TUI_A11Y_REDUCED_MOTION) != 0);
    add_assoc_bool(return_value, "high_contrast", (features & TUI_A11Y_HIGH_CONTRAST) != 0);
    add_assoc_bool(return_value, "screen_reader", (features & TUI_A11Y_SCREEN_READER) != 0);
}
/* }}} */

/* {{{ proto string tui_aria_role_to_string(int $role)
   Convert ARIA role enum to string */
PHP_FUNCTION(tui_aria_role_to_string)
{
    zend_long role;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(role)
    ZEND_PARSE_PARAMETERS_END();

    const char *role_str = tui_aria_role_to_string((tui_aria_role)role);
    RETURN_STRING(role_str);
}
/* }}} */

/* {{{ proto int tui_aria_role_from_string(string $role)
   Convert ARIA role string to enum */
PHP_FUNCTION(tui_aria_role_from_string)
{
    zend_string *role_str;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(role_str)
    ZEND_PARSE_PARAMETERS_END();

    tui_aria_role role = tui_aria_role_from_string(ZSTR_VAL(role_str));
    RETURN_LONG(role);
}
/* }}} */
