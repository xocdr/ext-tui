--TEST--
ANSI utility functions (strip, width, slice)
--EXTENSIONS--
tui
--FILE--
<?php
// Test tui_strip_ansi() - removes ANSI codes
$styled = "\033[31mRed\033[0m Text";
$plain = tui_strip_ansi($styled);
echo "Strip: '$plain'\n";

// Test tui_string_width_ansi() - width ignoring ANSI codes
$width = tui_string_width_ansi($styled);
echo "Width: $width\n";

// Plain string for comparison
echo "Plain width: " . tui_string_width("Red Text") . "\n";

// Test tui_slice_ansi() - extract substring preserving ANSI codes
$styled2 = "\033[32mHello\033[0m \033[34mWorld\033[0m";
$slice = tui_slice_ansi($styled2, 0, 5);  // Should get "Hello" with green styling
echo "Slice 0-5: " . tui_strip_ansi($slice) . "\n";

$slice2 = tui_slice_ansi($styled2, 6, 11);  // Should get "World" with blue styling
echo "Slice 6-11: " . tui_strip_ansi($slice2) . "\n";

// Edge case: empty string
echo "Empty strip: '" . tui_strip_ansi("") . "'\n";
echo "Empty width: " . tui_string_width_ansi("") . "\n";

// Edge case: no ANSI codes
echo "No ANSI: '" . tui_strip_ansi("Plain text") . "'\n";

// Complex ANSI sequence (bold + color)
$complex = "\033[1;33mBold Yellow\033[0m";
echo "Complex strip: '" . tui_strip_ansi($complex) . "'\n";
echo "Complex width: " . tui_string_width_ansi($complex) . "\n";

echo "Done\n";
?>
--EXPECT--
Strip: 'Red Text'
Width: 8
Plain width: 8
Slice 0-5: Hello
Slice 6-11: World
Empty strip: ''
Empty width: 0
No ANSI: 'Plain text'
Complex strip: 'Bold Yellow'
Complex width: 11
Done
