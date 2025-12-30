--TEST--
Drag and drop basic functionality
--EXTENSIONS--
tui
--FILE--
<?php

// Test function existence
var_dump(function_exists('tui_drag_start'));
var_dump(function_exists('tui_drag_move'));
var_dump(function_exists('tui_drag_end'));
var_dump(function_exists('tui_drag_cancel'));
var_dump(function_exists('tui_drag_is_active'));
var_dump(function_exists('tui_drag_get_type'));
var_dump(function_exists('tui_drag_get_data'));
var_dump(function_exists('tui_drag_get_state'));

// Initial state - no drag
var_dump(tui_drag_is_active() === false);

// Start drag
var_dump(tui_drag_start(10, 20, 'text/plain', 'test data'));
var_dump(tui_drag_is_active() === true);
var_dump(tui_drag_get_type() === 'text/plain');
var_dump(tui_drag_get_data() === 'test data');

// Get state
$state = tui_drag_get_state();
var_dump($state['state'] === 'started');
var_dump($state['start_x'] === 10);
var_dump($state['start_y'] === 20);

// Move
tui_drag_move(15, 25);
$state = tui_drag_get_state();
var_dump($state['state'] === 'dragging');
var_dump($state['current_x'] === 15);
var_dump($state['current_y'] === 25);

// End drag
tui_drag_end();
var_dump(tui_drag_is_active() === false);

// Cancel clears state
tui_drag_start(0, 0, 'test', 'data');
tui_drag_cancel();
$state = tui_drag_get_state();
var_dump($state['state'] === 'idle');

echo "OK\n";
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
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
OK
