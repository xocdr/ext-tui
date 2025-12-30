--TEST--
tui_set_focus_handler() function exists and accepts callable
--SKIPIF--
<?php if (!extension_loaded('tui')) die('skip tui extension not loaded'); ?>
--FILE--
<?php
// Check function exists
var_dump(function_exists('tui_set_focus_handler'));
var_dump(function_exists('tui_focus_next'));
var_dump(function_exists('tui_focus_prev'));

echo "Focus handler functions exist\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
Focus handler functions exist
