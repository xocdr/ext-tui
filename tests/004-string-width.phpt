--TEST--
tui_string_width() measures text width correctly
--EXTENSIONS--
tui
--FILE--
<?php
// ASCII
var_dump(tui_string_width("hello"));  // 5

// Empty
var_dump(tui_string_width(""));       // 0

// Unicode combining marks (should not add width)
var_dump(tui_string_width("e\xCC\x81")); // 1 (e + combining acute accent)

// Wide characters (CJK)
var_dump(tui_string_width("\xE4\xB8\xAD\xE6\x96\x87")); // 4 (2 chars, 2 width each)
?>
--EXPECT--
int(5)
int(0)
int(1)
int(4)
