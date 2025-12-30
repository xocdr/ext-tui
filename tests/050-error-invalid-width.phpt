--TEST--
Error handling for invalid width parameters
--EXTENSIONS--
tui
--FILE--
<?php
// Test tui_truncate with edge case widths
echo "=== tui_truncate edge cases ===\n";

// Zero width - returns ellipsis (minimum output)
$result = tui_truncate("Hello", 0);
echo "truncate(Hello, 0): " . var_export($result, true) . "\n";

// Width 1 - returns ellipsis
$result = tui_truncate("Hello", 1);
echo "truncate(Hello, 1): " . var_export($result, true) . "\n";

// Width 2 - returns ellipsis
$result = tui_truncate("Hello", 2);
echo "truncate(Hello, 2): " . var_export($result, true) . "\n";

// Very large width should return original
$result = tui_truncate("Hello", 10000);
echo "truncate(Hello, 10000): " . var_export($result, true) . "\n";

// Test tui_wrap_text with edge case widths
echo "\n=== tui_wrap_text edge cases ===\n";

// Width 1 should wrap each character
$result = tui_wrap_text("Hi", 1);
echo "wrap(Hi, 1): " . var_export($result, true) . "\n";

// Width 0 - edge case (returns empty array)
$result = tui_wrap_text("Hi", 0);
echo "wrap(Hi, 0): " . var_export($result, true) . "\n";

// Very large width
$result = tui_wrap_text("Hello World", 10000);
echo "wrap(Hello World, 10000): " . var_export($result, true) . "\n";

// Test tui_string_width with various inputs
echo "\n=== tui_string_width edge cases ===\n";

// Empty string
$result = tui_string_width("");
echo "width(''): " . var_export($result, true) . "\n";

// Single space
$result = tui_string_width(" ");
echo "width(' '): " . var_export($result, true) . "\n";

// Tab character
$result = tui_string_width("\t");
echo "width(tab): " . var_export($result, true) . "\n";

// Newline
$result = tui_string_width("\n");
echo "width(newline): " . var_export($result, true) . "\n";

echo "\nAll width edge case tests completed.\n";
?>
--EXPECT--
=== tui_truncate edge cases ===
truncate(Hello, 0): '...'
truncate(Hello, 1): '...'
truncate(Hello, 2): '...'
truncate(Hello, 10000): 'Hello'

=== tui_wrap_text edge cases ===
wrap(Hi, 1): array (
  0 => 'H',
  1 => 'i',
)
wrap(Hi, 0): array (
)
wrap(Hello World, 10000): array (
  0 => 'Hello World',
)

=== tui_string_width edge cases ===
width(''): 0
width(' '): 1
width(tab): 0
width(newline): 0

All width edge case tests completed.
