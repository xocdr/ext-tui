--TEST--
Edge case coverage for branch coverage
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Additional edge cases to improve branch coverage
 */

echo "=== String width edge cases ===\n";

// Empty string
echo "Empty: " . tui_string_width("") . "\n";

// Single character
echo "Single ASCII: " . tui_string_width("a") . "\n";
echo "Single CJK: " . tui_string_width("中") . "\n";

// Control characters
echo "Tab: " . tui_string_width("\t") . "\n";
echo "Newline: " . tui_string_width("\n") . "\n";
echo "Bell: " . tui_string_width("\x07") . "\n";

// Variation selectors alone
echo "VS16 alone: " . tui_string_width("\xEF\xB8\x8F") . "\n"; // U+FE0F
echo "VS15 alone: " . tui_string_width("\xEF\xB8\x8E") . "\n"; // U+FE0E

// Incomplete escape sequences
echo "Incomplete ESC: " . tui_string_width("\x1b") . "\n";
echo "ESC[: " . tui_string_width("\x1b[") . "\n";

// OSC without terminator
echo "OSC no term: " . tui_string_width("\x1b]8;;") . "\n";

echo "\n=== Wrap text edge cases ===\n";

// Empty string
$lines = tui_wrap_text("", 40);
echo "Empty wrap: " . count($lines) . " lines\n";

// Single word longer than width
$lines = tui_wrap_text("supercalifragilisticexpialidocious", 10);
echo "Long word: " . count($lines) . " lines, first='" . ($lines[0] ?? '') . "'\n";

// Only whitespace
$lines = tui_wrap_text("   ", 10);
echo "Whitespace: " . count($lines) . " lines\n";

// Width of 1
$lines = tui_wrap_text("abc", 1);
echo "Width 1: " . count($lines) . " lines\n";

echo "\n=== Truncate edge cases ===\n";

// Empty string
echo "Empty truncate: '" . tui_truncate("", 10) . "'\n";

// String shorter than width
echo "Short: '" . tui_truncate("Hi", 10) . "'\n";

// String exactly at width
echo "Exact: '" . tui_truncate("Hello", 5) . "'\n";

// Width of 0
echo "Width 0: '" . tui_truncate("Hello", 0) . "'\n";

// Width of 1
echo "Width 1: '" . tui_truncate("Hello", 1) . "'\n";

// Width of 2
echo "Width 2: '" . tui_truncate("Hello", 2) . "'\n";

// CJK at boundary (would split wide char)
echo "CJK boundary: '" . tui_truncate("Hi中文", 3) . "'\n";

// Custom ellipsis
echo "Custom ellipsis: '" . tui_truncate("Hello World", 8, "…") . "'\n";
echo "Empty ellipsis: '" . tui_truncate("Hello World", 5, "") . "'\n";

echo "\n=== Pad edge cases ===\n";

// Empty string
echo "Pad empty: '" . tui_pad("", 10) . "'\n";

// String longer than width
echo "Pad longer: '" . tui_pad("Hello World!", 5) . "'\n";

// Width of 0
echo "Pad width 0: '" . tui_pad("Hi", 0) . "'\n";

// Different alignments
echo "Left: '" . tui_pad("Hi", 5, 'left') . "'\n";
echo "Right: '" . tui_pad("Hi", 5, 'right') . "'\n";
echo "Center: '" . tui_pad("Hi", 5, 'center') . "'\n";

echo "\n=== Strip ANSI edge cases ===\n";

// No ANSI codes
echo "No ANSI: '" . tui_strip_ansi("Hello") . "'\n";

// Only ANSI codes
echo "Only ANSI: '" . tui_strip_ansi("\x1b[31m\x1b[0m") . "'\n";

// Nested/complex codes
echo "Complex: '" . tui_strip_ansi("\x1b[1;31;44mBold Red on Blue\x1b[0m") . "'\n";

// OSC with BEL terminator
echo "OSC BEL: '" . tui_strip_ansi("\x1b]0;Title\x07Text") . "'\n";

// OSC with ST terminator
echo "OSC ST: '" . tui_strip_ansi("\x1b]0;Title\x1b\\Text") . "'\n";

echo "\n=== History edge cases ===\n";

$history = tui_history_create(3); // Small capacity

// Navigate empty history
echo "Prev empty: " . var_export(tui_history_prev($history), true) . "\n";
echo "Next empty: " . var_export(tui_history_next($history), true) . "\n";

// Add entries
tui_history_add($history, "first");
tui_history_add($history, "second");
tui_history_add($history, "third");
tui_history_add($history, "fourth"); // Should evict "first"

// Navigate
echo "Prev 1: " . var_export(tui_history_prev($history), true) . "\n";
echo "Prev 2: " . var_export(tui_history_prev($history), true) . "\n";
echo "Prev 3: " . var_export(tui_history_prev($history), true) . "\n";
echo "Prev 4 (at oldest): " . var_export(tui_history_prev($history), true) . "\n";

// Temp storage
tui_history_save_temp($history, "temp input");
echo "Temp saved: " . var_export(tui_history_get_temp($history), true) . "\n";

// Reset position
tui_history_reset($history);
echo "After reset prev: " . var_export(tui_history_prev($history), true) . "\n";

// Duplicate suppression
tui_history_add($history, "same");
tui_history_add($history, "same"); // Should not add duplicate
tui_history_reset($history);
echo "After dup, prev 1: " . var_export(tui_history_prev($history), true) . "\n";
echo "After dup, prev 2: " . var_export(tui_history_prev($history), true) . "\n";

tui_history_destroy($history);

echo "\n=== Terminal info ===\n";

// Terminal size (may vary, just check it doesn't crash)
$size = tui_get_terminal_size();
echo "Terminal size: " . (is_array($size) ? "array" : "error") . "\n";

// Interactive check
echo "Is interactive: " . (is_bool(tui_is_interactive()) ? "bool" : "error") . "\n";

// CI check
echo "Is CI: " . (is_bool(tui_is_ci()) ? "bool" : "error") . "\n";

echo "\nAll edge case tests completed.\n";
?>
--EXPECT--
=== String width edge cases ===
Empty: 0
Single ASCII: 1
Single CJK: 2
Tab: 0
Newline: 0
Bell: 0
VS16 alone: 0
VS15 alone: 0
Incomplete ESC: 0
ESC[: 0
OSC no term: 0

=== Wrap text edge cases ===
Empty wrap: 0 lines
Long word: 4 lines, first='supercalif'
Whitespace: 1 lines
Width 1: 3 lines

=== Truncate edge cases ===
Empty truncate: ''
Short: 'Hi'
Exact: 'Hello'
Width 0: '...'
Width 1: '...'
Width 2: '...'
CJK boundary: '...'
Custom ellipsis: 'Hello W…'
Empty ellipsis: 'Hello'

=== Pad edge cases ===
Pad empty: '          '
Pad longer: 'Hello World!'
Pad width 0: 'Hi'
Left: 'Hi   '
Right: '   Hi'
Center: ' Hi  '

=== Strip ANSI edge cases ===
No ANSI: 'Hello'
Only ANSI: ''
Complex: 'Bold Red on Blue'
OSC BEL: 'Text'
OSC ST: 'Text'

=== History edge cases ===
Prev empty: NULL
Next empty: NULL
Prev 1: 'fourth'
Prev 2: 'third'
Prev 3: 'second'
Prev 4 (at oldest): 'second'
Temp saved: 'temp input'
After reset prev: 'fourth'
After dup, prev 1: 'same'
After dup, prev 2: 'fourth'

=== Terminal info ===
Terminal size: array
Is interactive: bool
Is CI: bool

All edge case tests completed.
