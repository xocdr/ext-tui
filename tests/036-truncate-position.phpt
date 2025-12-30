--TEST--
tui_truncate() with position modes (start, middle, end)
--EXTENSIONS--
tui
--FILE--
<?php
$text = "Hello World";

// End truncation (default)
echo "End: " . tui_truncate($text, 8, "...", "end") . "\n";

// Start truncation
echo "Start: " . tui_truncate($text, 8, "...", "start") . "\n";

// Middle truncation
echo "Middle: " . tui_truncate($text, 8, "...", "middle") . "\n";

// Default (should be end)
echo "Default: " . tui_truncate($text, 8, "...") . "\n";

// No truncation needed
echo "Short: " . tui_truncate("Hi", 10, "...") . "\n";

// Long text
$long = "This is a very long string that needs truncation";
echo "Long end: " . tui_truncate($long, 15, "...", "end") . "\n";
echo "Long start: " . tui_truncate($long, 15, "...", "start") . "\n";
echo "Long middle: " . tui_truncate($long, 15, "...", "middle") . "\n";
?>
--EXPECT--
End: Hello...
Start: ...World
Middle: He...rld
Default: Hello...
Short: Hi
Long end: This is a ve...
Long start: ...s truncation
Long middle: This i...cation
