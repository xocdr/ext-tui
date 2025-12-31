--TEST--
tui_truncate() comprehensive edge cases
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Comprehensive edge case testing for tui_truncate()
 * Tests: empty strings, exact fits, wide chars, custom ellipsis,
 * positions (start, middle, end), zero width
 */

echo "=== Empty string ===\n";
var_dump(tui_truncate("", 10));
var_dump(tui_truncate("", 0));

echo "\n=== String shorter than width ===\n";
var_dump(tui_truncate("Hello", 10));
var_dump(tui_truncate("Hello", 5));

echo "\n=== String longer than width (default ellipsis) ===\n";
var_dump(tui_truncate("Hello World", 8));
var_dump(tui_truncate("Hello World", 5));

echo "\n=== Custom ellipsis ===\n";
var_dump(tui_truncate("Hello World", 8, ".."));
var_dump(tui_truncate("Hello World", 8, "→"));
var_dump(tui_truncate("Hello World", 8, ""));

echo "\n=== Width smaller than ellipsis ===\n";
var_dump(tui_truncate("Hello World", 2));  // "..." is 3 chars
var_dump(tui_truncate("Hello World", 1));
var_dump(tui_truncate("Hello World", 0));

echo "\n=== Position: end (default) ===\n";
var_dump(tui_truncate("Hello World", 8, "...", "end"));

echo "\n=== Position: start ===\n";
var_dump(tui_truncate("Hello World", 8, "...", "start"));

echo "\n=== Position: middle ===\n";
var_dump(tui_truncate("Hello World", 8, "...", "middle"));

echo "\n=== Wide characters (CJK) ===\n";
var_dump(tui_truncate("你好世界测试", 6));  // 3 wide chars + ...
var_dump(tui_truncate("Hello你好", 8));

echo "\n=== Exact boundary ===\n";
var_dump(tui_truncate("12345", 5));   // exact fit
var_dump(tui_truncate("123456", 5));  // one over

echo "\n=== Long text ===\n";
$long = str_repeat("abcdefghij", 100);  // 1000 chars
$result = tui_truncate($long, 20);
echo "Length: " . strlen($result) . "\n";
echo "Result: $result\n";

echo "\n=== Ellipsis longer than width ===\n";
var_dump(tui_truncate("Hello", 2, "......"));

echo "\n=== Emoji truncation ===\n";
var_dump(tui_truncate("😀😎🎉👍", 6));

echo "\nDone!\n";
?>
--EXPECTF--
=== Empty string ===
string(0) ""
string(0) ""

=== String shorter than width ===
string(5) "Hello"
string(5) "Hello"

=== String longer than width (default ellipsis) ===
string(%d) "%s"
string(%d) "%s"

=== Custom ellipsis ===
string(%d) "%s"
string(%d) "%s"
string(%d) "%s"

=== Width smaller than ellipsis ===
string(%d) "%s"
string(%d) "%s"
string(%d) "%s"

=== Position: end (default) ===
string(%d) "%s"

=== Position: start ===
string(%d) "%s"

=== Position: middle ===
string(%d) "%s"

=== Wide characters (CJK) ===
string(%d) "%s"
string(%d) "%s"

=== Exact boundary ===
string(5) "12345"
string(%d) "%s"

=== Long text ===
Length: %d
Result: %s

=== Ellipsis longer than width ===
string(%d) "%s"

=== Emoji truncation ===
string(%d) "%s"

Done!
