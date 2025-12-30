/*
  +----------------------------------------------------------------------+
  | ext-tui: Text utility functions                                     |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"
#include "src/text/grapheme.h"

/* ------------------------------------------------------------------
 * Text Utility Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_string_width(string $text): int */
PHP_FUNCTION(tui_string_width)
{
    zend_string *text;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();

    int width = tui_string_width_n(ZSTR_VAL(text), (int)ZSTR_LEN(text));
    RETURN_LONG(width);
}
/* }}} */

/* {{{ tui_wrap_text(string $text, int $width): array */
PHP_FUNCTION(tui_wrap_text)
{
    zend_string *text;
    zend_long width;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(text)
        Z_PARAM_LONG(width)
    ZEND_PARSE_PARAMETERS_END();

    array_init(return_value);

    tui_wrapped_text *wrapped = tui_wrap_text(ZSTR_VAL(text), (int)width, TUI_WRAP_WORD_CHAR);

    if (wrapped) {
        for (int i = 0; i < wrapped->count; i++) {
            add_next_index_string(return_value, wrapped->lines[i]);
        }
        tui_wrapped_text_free(wrapped);
    }
}
/* }}} */

/* {{{ tui_truncate(string $text, int $width, string $ellipsis = '...', string $position = 'end'): string */
PHP_FUNCTION(tui_truncate)
{
    zend_string *text;
    zend_long width;
    zend_string *ellipsis = NULL;
    zend_string *position = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 4)
        Z_PARAM_STR(text)
        Z_PARAM_LONG(width)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(ellipsis)
        Z_PARAM_STR(position)
    ZEND_PARSE_PARAMETERS_END();

    const char *ellipsis_str = ellipsis ? ZSTR_VAL(ellipsis) : "...";

    tui_truncate_position pos = TUI_TRUNCATE_END;
    if (position) {
        const char *pos_str = ZSTR_VAL(position);
        if (strcmp(pos_str, "start") == 0) {
            pos = TUI_TRUNCATE_START;
        } else if (strcmp(pos_str, "middle") == 0) {
            pos = TUI_TRUNCATE_MIDDLE;
        }
    }

    char *result = tui_truncate_text_ex(ZSTR_VAL(text), (int)width, ellipsis_str, pos);

    if (result) {
        RETVAL_STRING(result);
        free(result);
    } else {
        RETURN_EMPTY_STRING();
    }
}
/* }}} */

/* {{{ tui_pad(string $text, int $width, string $align = 'l', string $pad_char = ' '): string */
PHP_FUNCTION(tui_pad)
{
    zend_string *text;
    zend_long width;
    zend_string *align = NULL;
    zend_string *pad_char = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 4)
        Z_PARAM_STR(text)
        Z_PARAM_LONG(width)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(align)
        Z_PARAM_STR(pad_char)
    ZEND_PARSE_PARAMETERS_END();

    if (width < 0) width = 0;
    if (width > 10000) {
        php_error_docref(NULL, E_WARNING, "Width exceeds maximum of 10000");
        RETURN_FALSE;
    }

    char align_ch = align && ZSTR_LEN(align) > 0 ? ZSTR_VAL(align)[0] : 'l';
    char pad_ch = pad_char && ZSTR_LEN(pad_char) > 0 ? ZSTR_VAL(pad_char)[0] : ' ';

    size_t buf_size = (size_t)width + ZSTR_LEN(text) + 1;
    char *output = emalloc(buf_size);

    int result_len = tui_pad_n(ZSTR_VAL(text), (int)width, align_ch, pad_ch, output, buf_size);
    if (result_len < 0) {
        efree(output);
        RETURN_FALSE;
    }

    RETVAL_STRINGL(output, result_len);
    efree(output);
}
/* }}} */

/* ------------------------------------------------------------------
 * ANSI Utility Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_strip_ansi(string $text): string */
PHP_FUNCTION(tui_strip_ansi)
{
    zend_string *text;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();

    char *result = tui_strip_ansi(ZSTR_VAL(text));

    if (result) {
        RETVAL_STRING(result);
        free(result);
    } else {
        RETURN_EMPTY_STRING();
    }
}
/* }}} */

/* {{{ tui_string_width_ansi(string $text): int */
PHP_FUNCTION(tui_string_width_ansi)
{
    zend_string *text;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();

    int width = tui_string_width_ansi(ZSTR_VAL(text));
    RETURN_LONG(width);
}
/* }}} */

/* {{{ tui_slice_ansi(string $text, int $start, int $end): string */
PHP_FUNCTION(tui_slice_ansi)
{
    zend_string *text;
    zend_long start;
    zend_long end;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_STR(text)
        Z_PARAM_LONG(start)
        Z_PARAM_LONG(end)
    ZEND_PARSE_PARAMETERS_END();

    char *result = tui_slice_ansi(ZSTR_VAL(text), (int)start, (int)end);

    if (result) {
        RETVAL_STRING(result);
        free(result);
    } else {
        RETURN_EMPTY_STRING();
    }
}
/* }}} */

/* ------------------------------------------------------------------
 * Grapheme Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_grapheme_count(string $text): int */
PHP_FUNCTION(tui_grapheme_count)
{
    zend_string *text;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();

    int count = tui_grapheme_count(ZSTR_VAL(text), (int)ZSTR_LEN(text));
    RETURN_LONG(count);
}
/* }}} */
