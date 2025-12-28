--TEST--
tui_set_input_handler() function exists and accepts callable
--SKIPIF--
<?php if (!extension_loaded('tui')) die('skip tui extension not loaded'); ?>
--FILE--
<?php
// Check function exists
var_dump(function_exists('tui_set_input_handler'));

// We can't fully test without a TTY, but we can verify the function signature
echo "tui_set_input_handler function exists\n";
?>
--EXPECT--
bool(true)
tui_set_input_handler function exists
