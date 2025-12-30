/*
  +----------------------------------------------------------------------+
  | ext-tui: Table functions                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"

/* ------------------------------------------------------------------
 * Table Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_table_create(array $headers): resource */
PHP_FUNCTION(tui_table_create)
{
    zval *zheaders;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(zheaders)
    ZEND_PARSE_PARAMETERS_END();

    HashTable *ht = Z_ARRVAL_P(zheaders);
    int header_count = zend_hash_num_elements(ht);

    if (header_count == 0) {
        RETURN_FALSE;
    }

    const char **headers = emalloc(sizeof(char*) * header_count);
    int i = 0;
    zval *val;
    ZEND_HASH_FOREACH_VAL(ht, val) {
        convert_to_string(val);
        headers[i++] = Z_STRVAL_P(val);
    } ZEND_HASH_FOREACH_END();

    tui_table *table = tui_table_create(headers, header_count);
    efree(headers);

    if (!table) {
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(table, le_tui_table));
}
/* }}} */

/* {{{ tui_table_add_row(resource $table, array $cells): void */
PHP_FUNCTION(tui_table_add_row)
{
    zval *ztable;
    zval *zcells;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(ztable)
        Z_PARAM_ARRAY(zcells)
    ZEND_PARSE_PARAMETERS_END();

    tui_table *table = (tui_table *)zend_fetch_resource(Z_RES_P(ztable), TUI_TABLE_RES_NAME, le_tui_table);
    if (!table) {
        RETURN_THROWS();
    }

    HashTable *ht = Z_ARRVAL_P(zcells);
    int cell_count = zend_hash_num_elements(ht);

    const char **cells = emalloc(sizeof(char*) * cell_count);
    int i = 0;
    zval *val;
    ZEND_HASH_FOREACH_VAL(ht, val) {
        convert_to_string(val);
        cells[i++] = Z_STRVAL_P(val);
    } ZEND_HASH_FOREACH_END();

    tui_table_add_row(table, cells);
    efree(cells);
}
/* }}} */

/* {{{ tui_table_set_align(resource $table, int $column, bool $right_align): void */
PHP_FUNCTION(tui_table_set_align)
{
    zval *ztable;
    zend_long column;
    zend_bool right_align;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_RESOURCE(ztable)
        Z_PARAM_LONG(column)
        Z_PARAM_BOOL(right_align)
    ZEND_PARSE_PARAMETERS_END();

    tui_table *table = (tui_table *)zend_fetch_resource(Z_RES_P(ztable), TUI_TABLE_RES_NAME, le_tui_table);
    if (!table) {
        RETURN_THROWS();
    }

    tui_table_set_align(table, (int)column, right_align ? 1 : 0);
}
/* }}} */

/* {{{ tui_table_render_to_buffer(resource $buffer, resource $table, int $x, int $y, string $border = 'single', ?array $header_style = null, ?array $cell_style = null): int */
PHP_FUNCTION(tui_table_render_to_buffer)
{
    zval *zbuffer, *ztable;
    zend_long x, y;
    zend_string *border = NULL;
    zval *zheader_style = NULL, *zcell_style = NULL;

    ZEND_PARSE_PARAMETERS_START(4, 7)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_RESOURCE(ztable)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(border)
        Z_PARAM_ARRAY_OR_NULL(zheader_style)
        Z_PARAM_ARRAY_OR_NULL(zcell_style)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    tui_table *table = (tui_table *)zend_fetch_resource(Z_RES_P(ztable), TUI_TABLE_RES_NAME, le_tui_table);
    if (!table) {
        RETURN_THROWS();
    }

    tui_border_style border_style = TUI_BORDER_SINGLE;
    if (border) {
        const char *bs = ZSTR_VAL(border);
        if (strcmp(bs, "double") == 0) border_style = TUI_BORDER_DOUBLE;
        else if (strcmp(bs, "round") == 0) border_style = TUI_BORDER_ROUND;
        else if (strcmp(bs, "bold") == 0) border_style = TUI_BORDER_BOLD;
    }

    tui_style header_style = {0}, cell_style = {0};
    if (zheader_style) {
        parse_style_array(zheader_style, &header_style);
    }
    if (zcell_style) {
        parse_style_array(zcell_style, &cell_style);
    }

    int height = tui_table_render(buffer, table, (int)x, (int)y, border_style, &header_style, &cell_style);
    RETURN_LONG(height);
}
/* }}} */
