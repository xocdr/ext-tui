--TEST--
Virtual list windowing functions
--EXTENSIONS--
tui
--FILE--
<?php
// Create virtual list: 100 items, 1 row each, 10 row viewport, 2 overscan
$vlist = tui_virtual_create(100, 1, 10, 2);

// Test initial state
$range = tui_virtual_get_range($vlist);
var_dump($range['start'] === 0);
var_dump($range['item_count'] === 100);
var_dump($range['viewport_height'] === 10);

// Test scroll_to
tui_virtual_scroll_to($vlist, 50);
$range = tui_virtual_get_range($vlist);
var_dump($range['offset'] === 50.0);
var_dump($range['start'] === 48); // 50 - 2 overscan

// Test scroll_top
tui_virtual_scroll_top($vlist);
$range = tui_virtual_get_range($vlist);
var_dump($range['offset'] === 0.0);
var_dump($range['start'] === 0);

// Test scroll_by
tui_virtual_scroll_by($vlist, 5);
$range = tui_virtual_get_range($vlist);
var_dump($range['offset'] === 5.0);

// Test page_down
tui_virtual_scroll_top($vlist);
tui_virtual_page_down($vlist);
$range = tui_virtual_get_range($vlist);
var_dump($range['offset'] === 9.0); // viewport - 1

// Test scroll_bottom
tui_virtual_scroll_bottom($vlist);
$range = tui_virtual_get_range($vlist);
var_dump($range['progress'] === 1.0);
var_dump($range['offset'] === 90.0); // 100 - 10

// Test ensure_visible (item already visible)
tui_virtual_scroll_to($vlist, 50);
$offset_before = tui_virtual_get_range($vlist)['offset'];
tui_virtual_ensure_visible($vlist, 55);
$offset_after = tui_virtual_get_range($vlist)['offset'];
var_dump($offset_before === $offset_after); // No change, item is visible

// Test ensure_visible (item below viewport)
tui_virtual_scroll_to($vlist, 0);
tui_virtual_ensure_visible($vlist, 20);
$range = tui_virtual_get_range($vlist);
var_dump($range['offset'] === 11.0); // Scrolled to show item 20 at bottom

// Test is_visible
tui_virtual_scroll_to($vlist, 50);
var_dump(tui_virtual_is_visible($vlist, 52));  // true - within range
var_dump(tui_virtual_is_visible($vlist, 10));  // false - outside range

// Test item_offset
tui_virtual_scroll_to($vlist, 50);
var_dump(tui_virtual_item_offset($vlist, 50) === 0);  // At top
var_dump(tui_virtual_item_offset($vlist, 55) === 5);  // 5 rows down

// Test set_count
tui_virtual_set_count($vlist, 20);
$range = tui_virtual_get_range($vlist);
var_dump($range['item_count'] === 20);

// Test set_viewport
tui_virtual_set_viewport($vlist, 5);
tui_virtual_scroll_top($vlist);
$range = tui_virtual_get_range($vlist);
var_dump($range['viewport_height'] === 5);

// Clean up
tui_virtual_destroy($vlist);
echo "Virtual list tests passed\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
Virtual list tests passed
