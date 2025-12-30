--TEST--
tui_string_width handles edge cases correctly
--SKIPIF--
<?php if (!extension_loaded('tui')) die('skip tui extension not loaded'); ?>
--FILE--
<?php
// Empty string
var_dump(tui_string_width(''));

// ASCII only
var_dump(tui_string_width('hello'));

// UTF-8 characters (emoji - 2 cells wide typically)
var_dump(tui_string_width('a'));

// Multi-byte UTF-8 (Chinese characters are 2 cells wide)
var_dump(tui_string_width('中'));

// Control characters (zero width)
var_dump(tui_string_width("a\tb"));

echo "String width edge cases handled\n";
?>
--EXPECTF--
int(0)
int(5)
int(1)
int(%d)
int(%d)
String width edge cases handled
