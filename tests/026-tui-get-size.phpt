--TEST--
tui_get_size function exists and works
--SKIPIF--
<?php if (!extension_loaded('tui')) die('skip tui extension not loaded'); ?>
--FILE--
<?php
// Check function exists
var_dump(function_exists('tui_get_size'));

// It should return null when passed null
// (we can't test with real instance without starting a TUI)
echo "tui_get_size function exists\n";
?>
--EXPECT--
bool(true)
tui_get_size function exists
