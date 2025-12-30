--TEST--
Graphics protocol detection functions
--EXTENSIONS--
tui
--FILE--
<?php

// Test that all functions exist
var_dump(function_exists('tui_graphics_protocol'));
var_dump(function_exists('tui_graphics_supported'));
var_dump(function_exists('tui_iterm2_supported'));
var_dump(function_exists('tui_sixel_supported'));

// Test return types
$protocol = tui_graphics_protocol();
var_dump(is_string($protocol));
var_dump(in_array($protocol, ['kitty', 'iterm2', 'sixel', 'none'], true));

var_dump(is_bool(tui_graphics_supported()));
var_dump(is_bool(tui_iterm2_supported()));
var_dump(is_bool(tui_sixel_supported()));

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
OK
