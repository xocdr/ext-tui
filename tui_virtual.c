/*
  +----------------------------------------------------------------------+
  | ext-tui: Virtual list PHP bindings                                  |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"
#include "src/scroll/virtual.h"

/* Resource type ID (defined in tui.c) */
extern int le_tui_virtual_list;

/* Resource destructor */
void tui_virtual_list_dtor(zend_resource *res)
{
    tui_virtual_list *vlist = (tui_virtual_list *)res->ptr;
    if (vlist) {
        efree(vlist);
    }
}

/* {{{ proto resource tui_virtual_create(int item_count, int item_height, int viewport_height, int overscan = 5)
   Create a virtual list for efficient large list rendering */
PHP_FUNCTION(tui_virtual_create)
{
    zend_long item_count, item_height, viewport_height;
    zend_long overscan = 5;

    ZEND_PARSE_PARAMETERS_START(3, 4)
        Z_PARAM_LONG(item_count)
        Z_PARAM_LONG(item_height)
        Z_PARAM_LONG(viewport_height)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(overscan)
    ZEND_PARSE_PARAMETERS_END();

    if (item_count < 0) {
        zend_throw_exception(tui_invalid_dimension_exception_ce,
            "item_count must be non-negative", 0);
        RETURN_THROWS();
    }
    if (item_height < 1) {
        zend_throw_exception(tui_invalid_dimension_exception_ce,
            "item_height must be at least 1", 0);
        RETURN_THROWS();
    }
    if (viewport_height < 1) {
        zend_throw_exception(tui_invalid_dimension_exception_ce,
            "viewport_height must be at least 1", 0);
        RETURN_THROWS();
    }

    tui_virtual_list *vlist = emalloc(sizeof(tui_virtual_list));
    tui_virtual_init(vlist, (int)item_count, (int)item_height,
                     (int)viewport_height, (int)overscan);

    RETURN_RES(zend_register_resource(vlist, le_tui_virtual_list));
}
/* }}} */

/* {{{ proto array tui_virtual_get_range(resource vlist)
   Get the visible range of items */
PHP_FUNCTION(tui_virtual_get_range)
{
    zval *zvlist;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zvlist)
    ZEND_PARSE_PARAMETERS_END();

    tui_virtual_list *vlist = (tui_virtual_list *)zend_fetch_resource(
        Z_RES_P(zvlist), TUI_VIRTUAL_LIST_RES_NAME, le_tui_virtual_list);

    if (!vlist) {
        RETURN_NULL();
    }

    array_init(return_value);
    add_assoc_long(return_value, "start", vlist->visible_start);
    add_assoc_long(return_value, "end", vlist->visible_end);
    add_assoc_double(return_value, "offset", vlist->scroll_offset);
    add_assoc_double(return_value, "progress", tui_virtual_progress(vlist));
    add_assoc_long(return_value, "item_count", vlist->item_count);
    add_assoc_long(return_value, "viewport_height", vlist->viewport_height);
    add_assoc_double(return_value, "max_scroll", vlist->max_scroll);
}
/* }}} */

/* {{{ proto void tui_virtual_scroll_to(resource vlist, int index)
   Scroll to show a specific item at the top */
PHP_FUNCTION(tui_virtual_scroll_to)
{
    zval *zvlist;
    zend_long index;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zvlist)
        Z_PARAM_LONG(index)
    ZEND_PARSE_PARAMETERS_END();

    tui_virtual_list *vlist = (tui_virtual_list *)zend_fetch_resource(
        Z_RES_P(zvlist), TUI_VIRTUAL_LIST_RES_NAME, le_tui_virtual_list);

    if (vlist) {
        tui_virtual_scroll_to(vlist, (int)index);
    }
}
/* }}} */

/* {{{ proto void tui_virtual_scroll_by(resource vlist, int delta)
   Scroll by a number of rows */
PHP_FUNCTION(tui_virtual_scroll_by)
{
    zval *zvlist;
    zend_long delta;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zvlist)
        Z_PARAM_LONG(delta)
    ZEND_PARSE_PARAMETERS_END();

    tui_virtual_list *vlist = (tui_virtual_list *)zend_fetch_resource(
        Z_RES_P(zvlist), TUI_VIRTUAL_LIST_RES_NAME, le_tui_virtual_list);

    if (vlist) {
        tui_virtual_scroll_by(vlist, (int)delta);
    }
}
/* }}} */

/* {{{ proto void tui_virtual_scroll_items(resource vlist, int items)
   Scroll by a number of items */
PHP_FUNCTION(tui_virtual_scroll_items)
{
    zval *zvlist;
    zend_long items;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zvlist)
        Z_PARAM_LONG(items)
    ZEND_PARSE_PARAMETERS_END();

    tui_virtual_list *vlist = (tui_virtual_list *)zend_fetch_resource(
        Z_RES_P(zvlist), TUI_VIRTUAL_LIST_RES_NAME, le_tui_virtual_list);

    if (vlist) {
        tui_virtual_scroll_items(vlist, (int)items);
    }
}
/* }}} */

/* {{{ proto void tui_virtual_ensure_visible(resource vlist, int index)
   Ensure a specific item is visible (scroll minimally) */
PHP_FUNCTION(tui_virtual_ensure_visible)
{
    zval *zvlist;
    zend_long index;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zvlist)
        Z_PARAM_LONG(index)
    ZEND_PARSE_PARAMETERS_END();

    tui_virtual_list *vlist = (tui_virtual_list *)zend_fetch_resource(
        Z_RES_P(zvlist), TUI_VIRTUAL_LIST_RES_NAME, le_tui_virtual_list);

    if (vlist) {
        tui_virtual_ensure_visible(vlist, (int)index);
    }
}
/* }}} */

/* {{{ proto void tui_virtual_page_up(resource vlist)
   Scroll up by one page */
PHP_FUNCTION(tui_virtual_page_up)
{
    zval *zvlist;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zvlist)
    ZEND_PARSE_PARAMETERS_END();

    tui_virtual_list *vlist = (tui_virtual_list *)zend_fetch_resource(
        Z_RES_P(zvlist), TUI_VIRTUAL_LIST_RES_NAME, le_tui_virtual_list);

    if (vlist) {
        tui_virtual_page_up(vlist);
    }
}
/* }}} */

/* {{{ proto void tui_virtual_page_down(resource vlist)
   Scroll down by one page */
PHP_FUNCTION(tui_virtual_page_down)
{
    zval *zvlist;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zvlist)
    ZEND_PARSE_PARAMETERS_END();

    tui_virtual_list *vlist = (tui_virtual_list *)zend_fetch_resource(
        Z_RES_P(zvlist), TUI_VIRTUAL_LIST_RES_NAME, le_tui_virtual_list);

    if (vlist) {
        tui_virtual_page_down(vlist);
    }
}
/* }}} */

/* {{{ proto void tui_virtual_scroll_top(resource vlist)
   Scroll to top of list */
PHP_FUNCTION(tui_virtual_scroll_top)
{
    zval *zvlist;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zvlist)
    ZEND_PARSE_PARAMETERS_END();

    tui_virtual_list *vlist = (tui_virtual_list *)zend_fetch_resource(
        Z_RES_P(zvlist), TUI_VIRTUAL_LIST_RES_NAME, le_tui_virtual_list);

    if (vlist) {
        tui_virtual_scroll_top(vlist);
    }
}
/* }}} */

/* {{{ proto void tui_virtual_scroll_bottom(resource vlist)
   Scroll to bottom of list */
PHP_FUNCTION(tui_virtual_scroll_bottom)
{
    zval *zvlist;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zvlist)
    ZEND_PARSE_PARAMETERS_END();

    tui_virtual_list *vlist = (tui_virtual_list *)zend_fetch_resource(
        Z_RES_P(zvlist), TUI_VIRTUAL_LIST_RES_NAME, le_tui_virtual_list);

    if (vlist) {
        tui_virtual_scroll_bottom(vlist);
    }
}
/* }}} */

/* {{{ proto void tui_virtual_set_count(resource vlist, int item_count)
   Update the item count (e.g., after filtering) */
PHP_FUNCTION(tui_virtual_set_count)
{
    zval *zvlist;
    zend_long item_count;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zvlist)
        Z_PARAM_LONG(item_count)
    ZEND_PARSE_PARAMETERS_END();

    tui_virtual_list *vlist = (tui_virtual_list *)zend_fetch_resource(
        Z_RES_P(zvlist), TUI_VIRTUAL_LIST_RES_NAME, le_tui_virtual_list);

    if (vlist) {
        tui_virtual_set_count(vlist, (int)item_count);
    }
}
/* }}} */

/* {{{ proto void tui_virtual_set_viewport(resource vlist, int viewport_height)
   Update the viewport height */
PHP_FUNCTION(tui_virtual_set_viewport)
{
    zval *zvlist;
    zend_long viewport_height;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zvlist)
        Z_PARAM_LONG(viewport_height)
    ZEND_PARSE_PARAMETERS_END();

    tui_virtual_list *vlist = (tui_virtual_list *)zend_fetch_resource(
        Z_RES_P(zvlist), TUI_VIRTUAL_LIST_RES_NAME, le_tui_virtual_list);

    if (vlist) {
        tui_virtual_set_viewport(vlist, (int)viewport_height);
    }
}
/* }}} */

/* {{{ proto int tui_virtual_item_offset(resource vlist, int index)
   Get the Y offset for a specific item relative to viewport */
PHP_FUNCTION(tui_virtual_item_offset)
{
    zval *zvlist;
    zend_long index;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zvlist)
        Z_PARAM_LONG(index)
    ZEND_PARSE_PARAMETERS_END();

    tui_virtual_list *vlist = (tui_virtual_list *)zend_fetch_resource(
        Z_RES_P(zvlist), TUI_VIRTUAL_LIST_RES_NAME, le_tui_virtual_list);

    if (!vlist) {
        RETURN_LONG(0);
    }

    RETURN_LONG(tui_virtual_item_offset(vlist, (int)index));
}
/* }}} */

/* {{{ proto bool tui_virtual_is_visible(resource vlist, int index)
   Check if an item is in the visible range */
PHP_FUNCTION(tui_virtual_is_visible)
{
    zval *zvlist;
    zend_long index;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zvlist)
        Z_PARAM_LONG(index)
    ZEND_PARSE_PARAMETERS_END();

    tui_virtual_list *vlist = (tui_virtual_list *)zend_fetch_resource(
        Z_RES_P(zvlist), TUI_VIRTUAL_LIST_RES_NAME, le_tui_virtual_list);

    if (!vlist) {
        RETURN_FALSE;
    }

    RETURN_BOOL(tui_virtual_is_visible(vlist, (int)index));
}
/* }}} */

/* {{{ proto void tui_virtual_destroy(resource vlist)
   Destroy a virtual list resource */
PHP_FUNCTION(tui_virtual_destroy)
{
    zval *zvlist;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zvlist)
    ZEND_PARSE_PARAMETERS_END();

    tui_virtual_list *vlist = (tui_virtual_list *)zend_fetch_resource(
        Z_RES_P(zvlist), TUI_VIRTUAL_LIST_RES_NAME, le_tui_virtual_list);

    if (vlist) {
        zend_list_close(Z_RES_P(zvlist));
    }
}
/* }}} */
