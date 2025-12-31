--TEST--
ANSI utility functions comprehensive testing
--EXTENSIONS--
tui
--FILE--
<?php
echo "=== tui_strip_ansi() ===\n";

// Basic color codes
$colored = "\x1b[31mRed text\x1b[0m";
echo "Strip red: '" . tui_strip_ansi($colored) . "'\n";

// Multiple colors
$multi = "\x1b[1m\x1b[31mBold Red\x1b[0m and \x1b[32mGreen\x1b[0m";
echo "Strip multi: '" . tui_strip_ansi($multi) . "'\n";

// No ANSI codes
$plain = "Plain text";
echo "Strip plain: '" . tui_strip_ansi($plain) . "'\n";

// Empty string
echo "Strip empty: '" . tui_strip_ansi("") . "'\n";

// Complex escape sequences
$complex = "\x1b[38;2;255;0;0mTrue color\x1b[0m";
echo "Strip truecolor: '" . tui_strip_ansi($complex) . "'\n";

// Cursor movement codes
$cursor = "\x1b[2J\x1b[H";  // Clear screen, home
echo "Strip cursor: '" . tui_strip_ansi($cursor) . "'\n";

echo "\n=== tui_string_width_ansi() ===\n";

// Text with color
$colored = "\x1b[31mHello\x1b[0m";
echo "Width of red 'Hello': " . tui_string_width_ansi($colored) . "\n";

// Multiple styles
$styled = "\x1b[1m\x1b[31mBold\x1b[0m Normal \x1b[4mUnderline\x1b[0m";
echo "Width of styled text: " . tui_string_width_ansi($styled) . "\n";  // "Bold Normal Underline" = 21

// No ANSI
$plain = "Plain";
echo "Width of plain: " . tui_string_width_ansi($plain) . "\n";

// Empty
echo "Width of empty: " . tui_string_width_ansi("") . "\n";

// Wide chars with ANSI
$wide = "\x1b[31m中文\x1b[0m";  // 2 wide chars = 4
echo "Width of red CJK: " . tui_string_width_ansi($wide) . "\n";

echo "\n=== tui_slice_ansi() ===\n";

// Slice colored text
$colored = "\x1b[31mHello World\x1b[0m";
$slice = tui_slice_ansi($colored, 0, 5);
echo "Slice 0-5: '" . tui_strip_ansi($slice) . "'\n";

$slice = tui_slice_ansi($colored, 6, 11);
echo "Slice 6-11: '" . tui_strip_ansi($slice) . "'\n";

// Slice in middle
$slice = tui_slice_ansi($colored, 2, 8);
echo "Slice 2-8: '" . tui_strip_ansi($slice) . "'\n";

// Slice from start
$slice = tui_slice_ansi($colored, 0, 3);
echo "Slice 0-3: '" . tui_strip_ansi($slice) . "'\n";

// Slice beyond length
$slice = tui_slice_ansi($colored, 0, 100);
echo "Slice 0-100: '" . tui_strip_ansi($slice) . "'\n";

// Empty slice
$slice = tui_slice_ansi($colored, 5, 5);
echo "Slice 5-5: '" . tui_strip_ansi($slice) . "'\n";

// Plain text slice
$plain = "Hello World";
$slice = tui_slice_ansi($plain, 0, 5);
echo "Plain slice 0-5: '$slice'\n";

echo "\n=== Edge cases ===\n";

// Only ANSI codes
$onlyAnsi = "\x1b[31m\x1b[0m";
echo "Width of only ANSI: " . tui_string_width_ansi($onlyAnsi) . "\n";
echo "Strip only ANSI: '" . tui_strip_ansi($onlyAnsi) . "'\n";

// Nested codes
$nested = "\x1b[1m\x1b[31m\x1b[4mNested\x1b[0m";
echo "Strip nested: '" . tui_strip_ansi($nested) . "'\n";

// Incomplete escape sequence
$incomplete = "\x1b[31mHello";  // No reset
echo "Strip incomplete: '" . tui_strip_ansi($incomplete) . "'\n";

// Binary with ANSI
$binary = "\x1b[31m\x00\x01\x02\x1b[0m";
echo "Width of binary with ANSI: " . tui_string_width_ansi($binary) . "\n";

echo "\nDone!\n";
?>
--EXPECT--
=== tui_strip_ansi() ===
Strip red: 'Red text'
Strip multi: 'Bold Red and Green'
Strip plain: 'Plain text'
Strip empty: ''
Strip truecolor: 'True color'
Strip cursor: ''

=== tui_string_width_ansi() ===
Width of red 'Hello': 5
Width of styled text: 21
Width of plain: 5
Width of empty: 0
Width of red CJK: 4

=== tui_slice_ansi() ===
Slice 0-5: 'Hello'
Slice 6-11: 'World'
Slice 2-8: 'llo Wo'
Slice 0-3: 'Hel'
Slice 0-100: 'Hello World'
Slice 5-5: ''
Plain slice 0-5: 'Hello'

=== Edge cases ===
Width of only ANSI: 0
Strip only ANSI: ''
Strip nested: 'Nested'
Strip incomplete: 'Hello'
Width of binary with ANSI: 0

Done!
