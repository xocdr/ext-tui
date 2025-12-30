/*
  +----------------------------------------------------------------------+
  | ext-tui: Terminal functions                                         |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"

/* ------------------------------------------------------------------
 * Terminal Info Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_get_terminal_size(): array */
PHP_FUNCTION(tui_get_terminal_size)
{
    struct winsize ws;
    int width = 80;
    int height = 24;

    ZEND_PARSE_PARAMETERS_NONE();

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        if (ws.ws_col > 0 && ws.ws_col <= 1000) {
            width = ws.ws_col;
        }
        if (ws.ws_row > 0 && ws.ws_row <= 1000) {
            height = ws.ws_row;
        }
    }

    TUI_G(terminal_width) = width;
    TUI_G(terminal_height) = height;

    array_init(return_value);
    add_next_index_long(return_value, width);
    add_next_index_long(return_value, height);
}
/* }}} */

/* {{{ tui_is_interactive(): bool */
PHP_FUNCTION(tui_is_interactive)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_BOOL(isatty(STDIN_FILENO) && isatty(STDOUT_FILENO));
}
/* }}} */

/* {{{ tui_is_ci(): bool */
PHP_FUNCTION(tui_is_ci)
{
    ZEND_PARSE_PARAMETERS_NONE();

    if (getenv("CI") != NULL ||
        getenv("GITHUB_ACTIONS") != NULL ||
        getenv("GITLAB_CI") != NULL ||
        getenv("CIRCLECI") != NULL ||
        getenv("TRAVIS") != NULL ||
        getenv("JENKINS_URL") != NULL) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}
/* }}} */

/* ------------------------------------------------------------------
 * Mouse Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_mouse_enable(int $mode = TUI_MOUSE_MODE_BUTTON): bool */
PHP_FUNCTION(tui_mouse_enable)
{
    zend_long mode = TUI_MOUSE_MODE_BUTTON;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(mode)
    ZEND_PARSE_PARAMETERS_END();

    if (mode < TUI_MOUSE_MODE_OFF || mode > TUI_MOUSE_MODE_ALL) {
        zend_throw_exception(zend_ce_exception, "Invalid mouse mode", 0);
        RETURN_THROWS();
    }

    int result = tui_terminal_enable_mouse((tui_mouse_mode)mode);
    RETURN_BOOL(result == 0);
}
/* }}} */

/* {{{ tui_mouse_disable(): bool */
PHP_FUNCTION(tui_mouse_disable)
{
    ZEND_PARSE_PARAMETERS_NONE();
    int result = tui_terminal_disable_mouse();
    RETURN_BOOL(result == 0);
}
/* }}} */

/* {{{ tui_mouse_get_mode(): int */
PHP_FUNCTION(tui_mouse_get_mode)
{
    ZEND_PARSE_PARAMETERS_NONE();
    tui_mouse_mode mode = tui_terminal_get_mouse_mode();
    RETURN_LONG((zend_long)mode);
}
/* }}} */

/* ------------------------------------------------------------------
 * Bracketed Paste Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_bracketed_paste_enable(): bool */
PHP_FUNCTION(tui_bracketed_paste_enable)
{
    ZEND_PARSE_PARAMETERS_NONE();
    int result = tui_terminal_enable_bracketed_paste();
    RETURN_BOOL(result == 0);
}
/* }}} */

/* {{{ tui_bracketed_paste_disable(): bool */
PHP_FUNCTION(tui_bracketed_paste_disable)
{
    ZEND_PARSE_PARAMETERS_NONE();
    int result = tui_terminal_disable_bracketed_paste();
    RETURN_BOOL(result == 0);
}
/* }}} */

/* {{{ tui_bracketed_paste_is_enabled(): bool */
PHP_FUNCTION(tui_bracketed_paste_is_enabled)
{
    ZEND_PARSE_PARAMETERS_NONE();
    int result = tui_terminal_is_bracketed_paste_enabled();
    RETURN_BOOL(result != 0);
}
/* }}} */

/* ------------------------------------------------------------------
 * Clipboard Functions (OSC 52)
 * ------------------------------------------------------------------ */

/* {{{ tui_clipboard_copy(string $text, string $target = 'clipboard'): bool */
PHP_FUNCTION(tui_clipboard_copy)
{
    zend_string *text;
    zend_string *target = NULL;
    tui_clipboard_target clipboard_target = TUI_CLIPBOARD_CLIPBOARD;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(text)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(target)
    ZEND_PARSE_PARAMETERS_END();

    if (target != NULL) {
        if (zend_string_equals_literal(target, "primary")) {
            clipboard_target = TUI_CLIPBOARD_PRIMARY;
        } else if (zend_string_equals_literal(target, "secondary")) {
            clipboard_target = TUI_CLIPBOARD_SECONDARY;
        } else if (!zend_string_equals_literal(target, "clipboard")) {
            zend_throw_exception(zend_ce_exception,
                "Invalid clipboard target. Use 'clipboard', 'primary', or 'secondary'", 0);
            RETURN_THROWS();
        }
    }

    size_t buf_size = tui_base64_encode_len(ZSTR_LEN(text)) + 32;
    char *buf = emalloc(buf_size);

    int len = tui_ansi_clipboard_write(buf, buf_size, ZSTR_VAL(text), ZSTR_LEN(text), clipboard_target);
    if (len < 0) {
        efree(buf);
        RETURN_FALSE;
    }

    ssize_t written = write(STDOUT_FILENO, buf, (size_t)len);
    efree(buf);

    RETURN_BOOL(written == len);
}
/* }}} */

/* {{{ tui_clipboard_request(string $target = 'clipboard'): void */
PHP_FUNCTION(tui_clipboard_request)
{
    zend_string *target = NULL;
    tui_clipboard_target clipboard_target = TUI_CLIPBOARD_CLIPBOARD;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(target)
    ZEND_PARSE_PARAMETERS_END();

    if (target != NULL) {
        if (zend_string_equals_literal(target, "primary")) {
            clipboard_target = TUI_CLIPBOARD_PRIMARY;
        } else if (zend_string_equals_literal(target, "secondary")) {
            clipboard_target = TUI_CLIPBOARD_SECONDARY;
        } else if (!zend_string_equals_literal(target, "clipboard")) {
            zend_throw_exception(zend_ce_exception,
                "Invalid clipboard target. Use 'clipboard', 'primary', or 'secondary'", 0);
            RETURN_THROWS();
        }
    }

    char buf[32];
    size_t len;
    tui_ansi_clipboard_request(buf, &len, clipboard_target);
    write(STDOUT_FILENO, buf, len);
}
/* }}} */

/* {{{ tui_clipboard_clear(string $target = 'clipboard'): void */
PHP_FUNCTION(tui_clipboard_clear)
{
    zend_string *target = NULL;
    tui_clipboard_target clipboard_target = TUI_CLIPBOARD_CLIPBOARD;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(target)
    ZEND_PARSE_PARAMETERS_END();

    if (target != NULL) {
        if (zend_string_equals_literal(target, "primary")) {
            clipboard_target = TUI_CLIPBOARD_PRIMARY;
        } else if (zend_string_equals_literal(target, "secondary")) {
            clipboard_target = TUI_CLIPBOARD_SECONDARY;
        } else if (!zend_string_equals_literal(target, "clipboard")) {
            zend_throw_exception(zend_ce_exception,
                "Invalid clipboard target. Use 'clipboard', 'primary', or 'secondary'", 0);
            RETURN_THROWS();
        }
    }

    char buf[32];
    size_t len;
    tui_ansi_clipboard_clear(buf, &len, clipboard_target);
    write(STDOUT_FILENO, buf, len);
}
/* }}} */

/* ------------------------------------------------------------------
 * Input History Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_history_create(int $maxEntries = 1000): resource */
PHP_FUNCTION(tui_history_create)
{
    zend_long max_entries = 1000;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(max_entries)
    ZEND_PARSE_PARAMETERS_END();

    if (max_entries < 1) max_entries = 1;
    if (max_entries > 100000) max_entries = 100000;

    tui_input_history *history = tui_history_create((int)max_entries);
    if (!history) {
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(history, le_tui_history));
}
/* }}} */

/* {{{ tui_history_destroy(resource $history): void */
PHP_FUNCTION(tui_history_destroy)
{
    zval *res;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(res)
    ZEND_PARSE_PARAMETERS_END();

    tui_input_history *history = (tui_input_history *)zend_fetch_resource(Z_RES_P(res), TUI_HISTORY_RES_NAME, le_tui_history);
    if (!history) {
        RETURN_THROWS();
    }

    zend_list_close(Z_RES_P(res));
}
/* }}} */

/* {{{ tui_history_add(resource $history, string $entry): void */
PHP_FUNCTION(tui_history_add)
{
    zval *res;
    zend_string *entry;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(res)
        Z_PARAM_STR(entry)
    ZEND_PARSE_PARAMETERS_END();

    tui_input_history *history = (tui_input_history *)zend_fetch_resource(Z_RES_P(res), TUI_HISTORY_RES_NAME, le_tui_history);
    if (!history) {
        RETURN_THROWS();
    }

    tui_history_add(history, ZSTR_VAL(entry));
}
/* }}} */

/* {{{ tui_history_prev(resource $history): ?string */
PHP_FUNCTION(tui_history_prev)
{
    zval *res;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(res)
    ZEND_PARSE_PARAMETERS_END();

    tui_input_history *history = (tui_input_history *)zend_fetch_resource(Z_RES_P(res), TUI_HISTORY_RES_NAME, le_tui_history);
    if (!history) {
        RETURN_THROWS();
    }

    const char *entry = tui_history_prev(history);
    if (entry) {
        RETURN_STRING(entry);
    }
    RETURN_NULL();
}
/* }}} */

/* {{{ tui_history_next(resource $history): ?string */
PHP_FUNCTION(tui_history_next)
{
    zval *res;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(res)
    ZEND_PARSE_PARAMETERS_END();

    tui_input_history *history = (tui_input_history *)zend_fetch_resource(Z_RES_P(res), TUI_HISTORY_RES_NAME, le_tui_history);
    if (!history) {
        RETURN_THROWS();
    }

    const char *entry = tui_history_next(history);
    if (entry) {
        RETURN_STRING(entry);
    }
    RETURN_NULL();
}
/* }}} */

/* {{{ tui_history_reset(resource $history): void */
PHP_FUNCTION(tui_history_reset)
{
    zval *res;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(res)
    ZEND_PARSE_PARAMETERS_END();

    tui_input_history *history = (tui_input_history *)zend_fetch_resource(Z_RES_P(res), TUI_HISTORY_RES_NAME, le_tui_history);
    if (!history) {
        RETURN_THROWS();
    }

    tui_history_reset_position(history);
}
/* }}} */

/* {{{ tui_history_save_temp(resource $history, string $input): void */
PHP_FUNCTION(tui_history_save_temp)
{
    zval *res;
    zend_string *input;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(res)
        Z_PARAM_STR(input)
    ZEND_PARSE_PARAMETERS_END();

    tui_input_history *history = (tui_input_history *)zend_fetch_resource(Z_RES_P(res), TUI_HISTORY_RES_NAME, le_tui_history);
    if (!history) {
        RETURN_THROWS();
    }

    tui_history_save_temp(history, ZSTR_VAL(input));
}
/* }}} */

/* {{{ tui_history_get_temp(resource $history): ?string */
PHP_FUNCTION(tui_history_get_temp)
{
    zval *res;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(res)
    ZEND_PARSE_PARAMETERS_END();

    tui_input_history *history = (tui_input_history *)zend_fetch_resource(Z_RES_P(res), TUI_HISTORY_RES_NAME, le_tui_history);
    if (!history) {
        RETURN_THROWS();
    }

    const char *temp = tui_history_get_temp(history);
    if (temp) {
        RETURN_STRING(temp);
    }
    RETURN_NULL();
}
/* }}} */
