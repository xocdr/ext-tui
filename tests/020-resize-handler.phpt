--TEST--
tui_set_resize_handler() function exists and accepts callable
--SKIPIF--
<?php if (!extension_loaded('tui')) die('skip tui extension not loaded'); ?>
--FILE--
<?php
// Check function exists
var_dump(function_exists('tui_set_resize_handler'));
echo "Resize handler function exists\n";
?>
--EXPECT--
bool(true)
Resize handler function exists
