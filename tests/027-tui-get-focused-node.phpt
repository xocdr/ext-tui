--TEST--
tui_get_focused_node function exists
--SKIPIF--
<?php if (!extension_loaded('tui')) die('skip tui extension not loaded'); ?>
--FILE--
<?php
// Check function exists
var_dump(function_exists('tui_get_focused_node'));

echo "tui_get_focused_node function exists\n";
?>
--EXPECT--
bool(true)
tui_get_focused_node function exists
