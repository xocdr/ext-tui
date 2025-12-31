--TEST--
tui_wrap_text() comprehensive edge cases
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Comprehensive edge case testing for tui_wrap_text()
 * Tests: empty strings, single words, exact fits, wide chars,
 * multiple spaces, very long words, zero width
 */

echo "=== Empty string ===\n";
var_dump(tui_wrap_text("", 10));

echo "\n=== Single word shorter than width ===\n";
var_dump(tui_wrap_text("Hello", 10));

echo "\n=== Single word exact width ===\n";
var_dump(tui_wrap_text("Hello", 5));

echo "\n=== Single word longer than width ===\n";
var_dump(tui_wrap_text("Supercalifragilistic", 10));

echo "\n=== Multiple words ===\n";
var_dump(tui_wrap_text("Hello World", 10));
var_dump(tui_wrap_text("Hello World", 5));
var_dump(tui_wrap_text("Hello World", 20));

echo "\n=== Multiple spaces ===\n";
var_dump(tui_wrap_text("Hello    World", 10));

echo "\n=== Leading/trailing spaces ===\n";
var_dump(tui_wrap_text("  Hello  ", 10));

echo "\n=== Newlines in input ===\n";
var_dump(tui_wrap_text("Hello\nWorld", 10));

echo "\n=== Wide characters (CJK) ===\n";
var_dump(tui_wrap_text("你好世界", 4));  // Each char is width 2
var_dump(tui_wrap_text("Hello你好", 7));

echo "\n=== Width 1 (extreme case) ===\n";
var_dump(tui_wrap_text("Hi", 1));

echo "\n=== Width 0 (edge case) ===\n";
var_dump(tui_wrap_text("Hi", 0));

echo "\n=== Very long text ===\n";
$text = "The quick brown fox jumps over the lazy dog";
$result = tui_wrap_text($text, 10);
echo "Lines: " . count($result) . "\n";
foreach ($result as $line) {
    echo "  [$line]\n";
}

echo "\n=== Single character ===\n";
var_dump(tui_wrap_text("X", 10));

echo "\n=== Only spaces ===\n";
var_dump(tui_wrap_text("     ", 3));

echo "\nDone!\n";
?>
--EXPECTF--
=== Empty string ===
array(0) {
}

=== Single word shorter than width ===
array(1) {
  [0]=>
  string(5) "Hello"
}

=== Single word exact width ===
array(1) {
  [0]=>
  string(5) "Hello"
}

=== Single word longer than width ===
array(2) {
  [0]=>
  string(10) "Supercalif"
  [1]=>
  string(10) "ragilistic"
}

=== Multiple words ===
array(2) {
  [0]=>
  string(5) "Hello"
  [1]=>
  string(5) "World"
}
array(2) {
  [0]=>
  string(5) "Hello"
  [1]=>
  string(5) "World"
}
array(1) {
  [0]=>
  string(11) "Hello World"
}

=== Multiple spaces ===
array(2) {
  [0]=>
  string(%d) "%s"
  [1]=>
  string(5) "World"
}

=== Leading/trailing spaces ===
array(1) {
  [0]=>
  string(%d) "%s"
}

=== Newlines in input ===
array(2) {
  [0]=>
  string(5) "Hello"
  [1]=>
  string(5) "World"
}

=== Wide characters (CJK) ===
array(2) {
  [0]=>
  string(6) "你好"
  [1]=>
  string(6) "世界"
}
array(2) {
  [0]=>
  string(%d) "%s"
  [1]=>
  string(%d) "%s"
}

=== Width 1 (extreme case) ===
array(2) {
  [0]=>
  string(1) "H"
  [1]=>
  string(1) "i"
}

=== Width 0 (edge case) ===
array(0) {
}

=== Very long text ===
Lines: 5
  [The quick]
  [brown fox]
  [jumps over]
  [the lazy]
  [dog]

=== Single character ===
array(1) {
  [0]=>
  string(1) "X"
}

=== Only spaces ===
array(1) {
  [0]=>
  string(3) "   "
}

Done!
