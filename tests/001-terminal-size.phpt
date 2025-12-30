--TEST--
tui_get_terminal_size() returns array with width and height
--EXTENSIONS--
tui
--FILE--
<?php
$size = tui_get_terminal_size();
var_dump(is_array($size));
var_dump(count($size));
var_dump(is_int($size[0]));
var_dump(is_int($size[1]));
var_dump($size[0] > 0);
var_dump($size[1] > 0);
?>
--EXPECT--
bool(true)
int(2)
bool(true)
bool(true)
bool(true)
bool(true)
