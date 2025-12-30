/*
  +----------------------------------------------------------------------+
  | ext-tui: Virtual list/windowing implementation                      |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "virtual.h"
#include <math.h>

/* Helper: clamp value between min and max */
static inline float clampf(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

static inline int clampi(int val, int min, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

void tui_virtual_init(tui_virtual_list *vlist, int item_count,
                       int item_height, int viewport_height, int overscan)
{
    if (!vlist) return;

    vlist->item_count = item_count > 0 ? item_count : 0;
    vlist->item_height = item_height > 0 ? item_height : 1;
    vlist->viewport_height = viewport_height > 0 ? viewport_height : 1;
    vlist->overscan = overscan > 0 ? overscan : 5;
    vlist->scroll_offset = 0.0f;
    vlist->visible_start = 0;
    vlist->visible_end = 0;
    vlist->max_scroll = 0.0f;

    tui_virtual_update(vlist);
}

void tui_virtual_update(tui_virtual_list *vlist)
{
    if (!vlist) return;

    /* Calculate max scroll offset */
    int content_height = vlist->item_count * vlist->item_height;
    vlist->max_scroll = (float)(content_height - vlist->viewport_height);
    if (vlist->max_scroll < 0.0f) {
        vlist->max_scroll = 0.0f;
    }

    /* Clamp scroll offset */
    vlist->scroll_offset = clampf(vlist->scroll_offset, 0.0f, vlist->max_scroll);

    /* Calculate visible range based on scroll offset */
    /* First visible item (may be partially visible) */
    int first = (int)(vlist->scroll_offset / vlist->item_height);

    /* Number of items that fit in viewport (round up for partial items) */
    int visible_count = (vlist->viewport_height + vlist->item_height - 1) / vlist->item_height;

    /* Add 1 for potentially partial item at top */
    visible_count += 1;

    /* Apply overscan */
    vlist->visible_start = clampi(first - vlist->overscan, 0, vlist->item_count);
    vlist->visible_end = clampi(first + visible_count + vlist->overscan, 0, vlist->item_count);
}

void tui_virtual_scroll_to(tui_virtual_list *vlist, int index)
{
    if (!vlist) return;

    index = clampi(index, 0, vlist->item_count - 1);
    vlist->scroll_offset = (float)(index * vlist->item_height);
    tui_virtual_update(vlist);
}

void tui_virtual_scroll_by(tui_virtual_list *vlist, int delta)
{
    if (!vlist) return;

    vlist->scroll_offset += (float)delta;
    tui_virtual_update(vlist);
}

void tui_virtual_scroll_items(tui_virtual_list *vlist, int items)
{
    if (!vlist) return;

    tui_virtual_scroll_by(vlist, items * vlist->item_height);
}

void tui_virtual_ensure_visible(tui_virtual_list *vlist, int index)
{
    if (!vlist) return;

    index = clampi(index, 0, vlist->item_count - 1);

    int item_top = index * vlist->item_height;
    int item_bottom = item_top + vlist->item_height;
    int view_top = (int)vlist->scroll_offset;
    int view_bottom = view_top + vlist->viewport_height;

    if (item_top < view_top) {
        /* Item is above viewport - scroll up to show it */
        vlist->scroll_offset = (float)item_top;
    } else if (item_bottom > view_bottom) {
        /* Item is below viewport - scroll down to show it */
        vlist->scroll_offset = (float)(item_bottom - vlist->viewport_height);
    }
    /* Otherwise item is already visible, do nothing */

    tui_virtual_update(vlist);
}

void tui_virtual_scroll_top(tui_virtual_list *vlist)
{
    if (!vlist) return;

    vlist->scroll_offset = 0.0f;
    tui_virtual_update(vlist);
}

void tui_virtual_scroll_bottom(tui_virtual_list *vlist)
{
    if (!vlist) return;

    vlist->scroll_offset = vlist->max_scroll;
    tui_virtual_update(vlist);
}

void tui_virtual_page_up(tui_virtual_list *vlist)
{
    if (!vlist) return;

    /* Page is viewport height minus 1 row for context */
    int page_size = vlist->viewport_height - 1;
    if (page_size < 1) page_size = 1;

    tui_virtual_scroll_by(vlist, -page_size);
}

void tui_virtual_page_down(tui_virtual_list *vlist)
{
    if (!vlist) return;

    int page_size = vlist->viewport_height - 1;
    if (page_size < 1) page_size = 1;

    tui_virtual_scroll_by(vlist, page_size);
}

void tui_virtual_set_count(tui_virtual_list *vlist, int item_count)
{
    if (!vlist) return;

    vlist->item_count = item_count > 0 ? item_count : 0;
    tui_virtual_update(vlist);
}

void tui_virtual_set_viewport(tui_virtual_list *vlist, int viewport_height)
{
    if (!vlist) return;

    vlist->viewport_height = viewport_height > 0 ? viewport_height : 1;
    tui_virtual_update(vlist);
}

int tui_virtual_item_offset(const tui_virtual_list *vlist, int index)
{
    if (!vlist) return 0;

    return index * vlist->item_height - (int)vlist->scroll_offset;
}

int tui_virtual_is_visible(const tui_virtual_list *vlist, int index)
{
    if (!vlist) return 0;

    return index >= vlist->visible_start && index < vlist->visible_end;
}

int tui_virtual_content_height(const tui_virtual_list *vlist)
{
    if (!vlist) return 0;

    return vlist->item_count * vlist->item_height;
}

float tui_virtual_progress(const tui_virtual_list *vlist)
{
    if (!vlist || vlist->max_scroll <= 0.0f) {
        return 0.0f;
    }

    return vlist->scroll_offset / vlist->max_scroll;
}
