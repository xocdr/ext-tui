/*
  +----------------------------------------------------------------------+
  | ext-tui: Virtual list/windowing for large lists                     |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_VIRTUAL_H
#define TUI_VIRTUAL_H

#include <stddef.h>

/**
 * Virtual list configuration for efficient rendering of large lists.
 *
 * Key concepts:
 * - Only visible items (plus overscan) are rendered
 * - Scroll offset determines which items are visible
 * - Memory usage is O(visible items) not O(total items)
 */
typedef struct {
    int item_count;        /* Total number of items */
    int item_height;       /* Height of each item in rows (fixed) */
    int viewport_height;   /* Height of visible area in rows */
    int overscan;          /* Extra items to render above/below visible */

    /* Computed state */
    float scroll_offset;   /* Current scroll position in pixels/rows */
    int visible_start;     /* First visible item index */
    int visible_end;       /* Last visible item index (exclusive) */

    /* Scroll boundaries */
    float max_scroll;      /* Maximum scroll offset */
} tui_virtual_list;

/**
 * Initialize a virtual list.
 *
 * @param vlist     Virtual list to initialize
 * @param item_count     Total number of items
 * @param item_height    Height of each item in rows
 * @param viewport_height Height of visible area in rows
 * @param overscan       Extra items to render (default: 5)
 */
void tui_virtual_init(tui_virtual_list *vlist, int item_count,
                       int item_height, int viewport_height, int overscan);

/**
 * Update visible range based on current scroll offset.
 * Call this after changing scroll_offset or dimensions.
 */
void tui_virtual_update(tui_virtual_list *vlist);

/**
 * Scroll to a specific item index.
 * Adjusts scroll_offset to show the item at the top of viewport.
 *
 * @param vlist   Virtual list
 * @param index   Item index to scroll to
 */
void tui_virtual_scroll_to(tui_virtual_list *vlist, int index);

/**
 * Scroll by a delta (in rows).
 * Positive delta scrolls down, negative scrolls up.
 *
 * @param vlist   Virtual list
 * @param delta   Number of rows to scroll
 */
void tui_virtual_scroll_by(tui_virtual_list *vlist, int delta);

/**
 * Scroll by a number of items.
 * Convenience wrapper that multiplies by item_height.
 *
 * @param vlist   Virtual list
 * @param items   Number of items to scroll (can be negative)
 */
void tui_virtual_scroll_items(tui_virtual_list *vlist, int items);

/**
 * Ensure a specific item is visible in the viewport.
 * If already visible, does nothing. Otherwise scrolls minimally.
 *
 * @param vlist   Virtual list
 * @param index   Item index to ensure visible
 */
void tui_virtual_ensure_visible(tui_virtual_list *vlist, int index);

/**
 * Scroll to top of list.
 */
void tui_virtual_scroll_top(tui_virtual_list *vlist);

/**
 * Scroll to bottom of list.
 */
void tui_virtual_scroll_bottom(tui_virtual_list *vlist);

/**
 * Page up (scroll by viewport_height - 1).
 */
void tui_virtual_page_up(tui_virtual_list *vlist);

/**
 * Page down (scroll by viewport_height - 1).
 */
void tui_virtual_page_down(tui_virtual_list *vlist);

/**
 * Set new item count (e.g., after filtering or adding items).
 * Adjusts scroll position if necessary to stay in bounds.
 */
void tui_virtual_set_count(tui_virtual_list *vlist, int item_count);

/**
 * Set new viewport height.
 */
void tui_virtual_set_viewport(tui_virtual_list *vlist, int viewport_height);

/**
 * Get the Y offset for a specific item relative to the viewport.
 * Useful for positioning items when rendering.
 *
 * @param vlist   Virtual list
 * @param index   Item index
 * @return Y offset in rows (may be negative if scrolled past)
 */
int tui_virtual_item_offset(const tui_virtual_list *vlist, int index);

/**
 * Check if an item is currently in the visible range (including overscan).
 */
int tui_virtual_is_visible(const tui_virtual_list *vlist, int index);

/**
 * Get the total scrollable content height.
 */
int tui_virtual_content_height(const tui_virtual_list *vlist);

/**
 * Get scroll progress (0.0 to 1.0).
 */
float tui_virtual_progress(const tui_virtual_list *vlist);

#endif /* TUI_VIRTUAL_H */
