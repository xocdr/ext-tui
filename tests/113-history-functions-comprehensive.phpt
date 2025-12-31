--TEST--
Input history functions comprehensive testing
--EXTENSIONS--
tui
--FILE--
<?php
echo "=== tui_history_create() ===\n";
$history = tui_history_create(100);
var_dump(is_resource($history) || is_object($history));

echo "\n=== Add entries ===\n";
tui_history_add($history, "first command");
echo "Added 'first command'\n";

tui_history_add($history, "second command");
echo "Added 'second command'\n";

tui_history_add($history, "third command");
echo "Added 'third command'\n";

echo "\n=== Navigate history ===\n";
$prev = tui_history_prev($history);
echo "Prev: " . ($prev ?? 'null') . "\n";

$prev = tui_history_prev($history);
echo "Prev: " . ($prev ?? 'null') . "\n";

$prev = tui_history_prev($history);
echo "Prev: " . ($prev ?? 'null') . "\n";

$prev = tui_history_prev($history);  // Should be at start
echo "Prev (at start): " . ($prev ?? 'null') . "\n";

echo "\n=== Navigate forward ===\n";
$next = tui_history_next($history);
echo "Next: " . ($next ?? 'null') . "\n";

$next = tui_history_next($history);
echo "Next: " . ($next ?? 'null') . "\n";

echo "\n=== Reset position ===\n";
tui_history_reset($history);
echo "Reset position\n";

$prev = tui_history_prev($history);
echo "After reset, prev: " . ($prev ?? 'null') . "\n";

echo "\n=== Temp storage ===\n";
tui_history_save_temp($history, "partial input");
echo "Saved temp\n";

$temp = tui_history_get_temp($history);
echo "Get temp: " . ($temp ?? 'null') . "\n";

echo "\n=== Empty history ===\n";
$empty = tui_history_create(10);
$prev = tui_history_prev($empty);
echo "Prev on empty: " . ($prev ?? 'null') . "\n";

$next = tui_history_next($empty);
echo "Next on empty: " . ($next ?? 'null') . "\n";

echo "\n=== Small capacity ===\n";
$small = tui_history_create(3);
tui_history_add($small, "a");
tui_history_add($small, "b");
tui_history_add($small, "c");
tui_history_add($small, "d");  // Should push out 'a'
echo "Added 4 items to capacity-3 history\n";

tui_history_reset($small);
$prev = tui_history_prev($small);
echo "Most recent: " . ($prev ?? 'null') . "\n";

echo "\n=== Destroy ===\n";
tui_history_destroy($history);
tui_history_destroy($empty);
tui_history_destroy($small);
echo "Destroyed all histories\n";

echo "\n=== Edge cases ===\n";
$h = tui_history_create(5);

// Empty string
tui_history_add($h, "");
echo "Added empty string\n";

// Very long entry
$long = str_repeat("x", 10000);
tui_history_add($h, $long);
echo "Added very long entry\n";

// Unicode
tui_history_add($h, "你好世界");
echo "Added unicode\n";

// Special characters
tui_history_add($h, "line1\nline2\ttab");
echo "Added special chars\n";

tui_history_destroy($h);

echo "\nDone!\n";
?>
--EXPECTF--
=== tui_history_create() ===
bool(true)

=== Add entries ===
Added 'first command'
Added 'second command'
Added 'third command'

=== Navigate history ===
Prev: third command
Prev: second command
Prev: first command
Prev (at start): %s

=== Navigate forward ===
Next: %s
Next: %s

=== Reset position ===
Reset position
After reset, prev: third command

=== Temp storage ===
Saved temp
Get temp: partial input

=== Empty history ===
Prev on empty: %s
Next on empty: %s

=== Small capacity ===
Added 4 items to capacity-3 history
Most recent: d

=== Destroy ===
Destroyed all histories

=== Edge cases ===
Added empty string
Added very long entry
Added unicode
Added special chars

Done!
