--TEST--
All event-related functions exist
--SKIPIF--
<?php if (!extension_loaded('tui')) die('skip tui extension not loaded'); ?>
--FILE--
<?php
// Input events
var_dump(function_exists('tui_set_input_handler'));

// Focus events
var_dump(function_exists('tui_set_focus_handler'));
var_dump(function_exists('tui_focus_next'));
var_dump(function_exists('tui_focus_prev'));

// Resize events
var_dump(function_exists('tui_set_resize_handler'));

echo "All event functions exist\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
All event functions exist
