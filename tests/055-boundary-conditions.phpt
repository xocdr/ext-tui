--TEST--
Boundary condition tests for text functions
--EXTENSIONS--
tui
--FILE--
<?php
echo "=== Empty string handling ===\n";

$result = tui_string_width("");
echo "width(''): $result\n";

$result = tui_truncate("", 10);
echo "truncate('', 10): '$result'\n";

$result = tui_truncate("", 0);
echo "truncate('', 0): '$result'\n";

$result = tui_wrap_text("", 10);
echo "wrap('', 10): " . json_encode($result) . "\n";

echo "\n=== Single character ===\n";

$result = tui_string_width("X");
echo "width('X'): $result\n";

$result = tui_truncate("X", 1);
echo "truncate('X', 1): '$result'\n";

$result = tui_truncate("X", 0);
echo "truncate('X', 0): '$result'\n";

$result = tui_wrap_text("X", 1);
echo "wrap('X', 1): " . json_encode($result) . "\n";

echo "\n=== Exact boundary (text length == width) ===\n";

$result = tui_truncate("Hello", 5);
echo "truncate('Hello', 5): '$result'\n";

$result = tui_truncate("Hello", 6);
echo "truncate('Hello', 6): '$result'\n";

$result = tui_wrap_text("Hello", 5);
echo "wrap('Hello', 5): " . json_encode($result) . "\n";

echo "\n=== One less than needed ===\n";

$result = tui_truncate("Hello", 4);
echo "truncate('Hello', 4): '$result'\n";

$result = tui_truncate("Hello", 3);
echo "truncate('Hello', 3): '$result'\n";

echo "\n=== Wide characters at boundary ===\n";

// Wide char (width 2) at truncation boundary
$result = tui_truncate("Hi中", 3);
echo "truncate('Hi中', 3): '$result' (len=" . strlen($result) . ")\n";

$result = tui_truncate("Hi中", 4);
echo "truncate('Hi中', 4): '$result'\n";

$result = tui_truncate("中中", 3);
echo "truncate('中中', 3): '$result'\n";

echo "\n=== Ellipsis edge cases ===\n";

// Custom ellipsis
$result = tui_truncate("Hello World", 8, "...");
echo "truncate with '...': '$result'\n";

$result = tui_truncate("Hello World", 8, "→");
echo "truncate with '→': '$result'\n";

// Empty ellipsis
$result = tui_truncate("Hello World", 5, "");
echo "truncate with empty ellipsis: '$result'\n";

// Ellipsis longer than width
$result = tui_truncate("Hello", 2, "...");
echo "truncate with long ellipsis: '$result'\n";

echo "\n=== Very long strings ===\n";

$longStr = str_repeat("X", 10000);
$result = tui_string_width($longStr);
echo "width(10000 chars): $result\n";

$result = tui_truncate($longStr, 10);
echo "truncate(10000 chars, 10): '$result' (len=" . strlen($result) . ")\n";

echo "\n=== Whitespace handling ===\n";

$result = tui_string_width("   ");
echo "width('   '): $result\n";

$result = tui_truncate("   ", 2);
echo "truncate('   ', 2): '$result'\n";

$result = tui_wrap_text("   ", 2);
echo "wrap('   ', 2): " . json_encode($result) . "\n";

// Multiple spaces between words
$result = tui_wrap_text("a    b", 3);
echo "wrap('a    b', 3): " . json_encode($result) . "\n";

echo "\n=== Newline handling ===\n";

$result = tui_string_width("a\nb");
echo "width('a\\nb'): $result\n";

$result = tui_wrap_text("a\nb", 10);
echo "wrap('a\\nb', 10): " . json_encode($result) . "\n";

$result = tui_wrap_text("\n\n", 10);
echo "wrap('\\n\\n', 10): " . json_encode($result) . "\n";

echo "\n=== ANSI codes at boundaries ===\n";

$ansi = "\033[31mHello\033[0m";
$result = tui_string_width_ansi($ansi);
echo "width_ansi(red Hello): $result\n";

$result = tui_strip_ansi($ansi);
echo "strip_ansi(red Hello): '$result'\n";

echo "\nAll boundary tests completed.\n";
?>
--EXPECT--
=== Empty string handling ===
width(''): 0
truncate('', 10): ''
truncate('', 0): ''
wrap('', 10): []

=== Single character ===
width('X'): 1
truncate('X', 1): 'X'
truncate('X', 0): '...'
wrap('X', 1): ["X"]

=== Exact boundary (text length == width) ===
truncate('Hello', 5): 'Hello'
truncate('Hello', 6): 'Hello'
wrap('Hello', 5): ["Hello"]

=== One less than needed ===
truncate('Hello', 4): 'H...'
truncate('Hello', 3): '...'

=== Wide characters at boundary ===
truncate('Hi中', 3): '...' (len=3)
truncate('Hi中', 4): 'Hi中'
truncate('中中', 3): '...'

=== Ellipsis edge cases ===
truncate with '...': 'Hello...'
truncate with '→': 'Hello W→'
truncate with empty ellipsis: 'Hello'
truncate with long ellipsis: '...'

=== Very long strings ===
width(10000 chars): 10000
truncate(10000 chars, 10): 'XXXXXXX...' (len=10)

=== Whitespace handling ===
width('   '): 3
truncate('   ', 2): '...'
wrap('   ', 2): ["  "]
wrap('a    b', 3): ["a  ","b"]

=== Newline handling ===
width('a\nb'): 2
wrap('a\nb', 10): ["a","b"]
wrap('\n\n', 10): ["",""]

=== ANSI codes at boundaries ===
width_ansi(red Hello): 5
strip_ansi(red Hello): 'Hello'

All boundary tests completed.
