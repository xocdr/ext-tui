--TEST--
tui_wrap_text() wraps text correctly
--EXTENSIONS--
tui
--FILE--
<?php
// Basic word wrap
$lines = tui_wrap_text("hello world foo bar", 10);
var_dump(is_array($lines));
var_dump(count($lines) >= 2);

// Single word
$lines = tui_wrap_text("hello", 10);
var_dump(count($lines));

// Empty string
$lines = tui_wrap_text("", 10);
var_dump(is_array($lines));
?>
--EXPECT--
bool(true)
bool(true)
int(1)
bool(true)
