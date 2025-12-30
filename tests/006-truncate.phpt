--TEST--
tui_truncate() truncates text correctly
--EXTENSIONS--
tui
--FILE--
<?php
// Text that fits
var_dump(tui_truncate("hello", 10));

// Text that needs truncating
$result = tui_truncate("hello world this is long", 10);
var_dump(strlen($result) <= 10);

// Custom ellipsis
$result = tui_truncate("hello world", 8, "..");
var_dump(strlen($result) <= 8);
?>
--EXPECT--
string(5) "hello"
bool(true)
bool(true)
