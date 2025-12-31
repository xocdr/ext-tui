--TEST--
Boundary value testing for numeric parameters
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

echo "=== Zero values ===\n";

// Zero width
var_dump(tui_string_width(""));

// Zero width wrap
$result = tui_wrap_text("hello", 0);
echo "Wrap with 0 width: " . (is_array($result) ? 'array' : gettype($result)) . "\n";

// Zero width truncate
$result = tui_truncate("hello", 0);
echo "Truncate to 0: '$result'\n";

// Zero-size buffer
$buffer = tui_buffer_create(0, 0);
echo "Zero buffer: " . (is_resource($buffer) || is_object($buffer) ? 'created' : 'null') . "\n";

// Zero-size box
$box = new Box(['width' => 0, 'height' => 0]);
echo "Zero box width: " . $box->width . "\n";
echo "Zero box height: " . $box->height . "\n";

echo "\n=== Maximum/Large values ===\n";

// Very large width
$result = tui_truncate("hi", 1000000);
echo "Truncate to 1M: '$result'\n";

// Very large wrap width
$result = tui_wrap_text("hello world", 1000000);
echo "Wrap with 1M width: " . count($result) . " lines\n";

// Large buffer (but reasonable)
$buffer = tui_buffer_create(1000, 500);
echo "Large buffer: " . (is_resource($buffer) || is_object($buffer) ? 'created' : 'null') . "\n";

// Large box dimensions
$box = new Box(['width' => 10000, 'height' => 5000]);
echo "Large box: {$box->width}x{$box->height}\n";

echo "\n=== Negative values ===\n";

// Negative box dimensions (should be treated as auto or 0)
$box = new Box(['width' => -10, 'height' => -20]);
echo "Negative width: " . ($box->width ?? 'null') . "\n";
echo "Negative height: " . ($box->height ?? 'null') . "\n";

// Negative padding/margin (should handle gracefully)
$box = new Box(['padding' => -5]);
echo "Negative padding set\n";

$box = new Box(['margin' => -5]);
echo "Negative margin set\n";

echo "\n=== Percentage values ===\n";

$box = new Box(['width' => '0%']);
echo "0% width set\n";

$box = new Box(['width' => '100%']);
echo "100% width set\n";

$box = new Box(['width' => '150%']);  // Over 100%
echo "150% width set\n";

echo "\n=== Edge case dimensions ===\n";

// 1x1 buffer
$buffer = tui_buffer_create(1, 1);
echo "1x1 buffer: " . (is_resource($buffer) || is_object($buffer) ? 'created' : 'null') . "\n";

// Very wide, short buffer
$buffer = tui_buffer_create(1000, 1);
echo "1000x1 buffer: " . (is_resource($buffer) || is_object($buffer) ? 'created' : 'null') . "\n";

// Narrow, tall buffer
$buffer = tui_buffer_create(1, 1000);
echo "1x1000 buffer: " . (is_resource($buffer) || is_object($buffer) ? 'created' : 'null') . "\n";

echo "\n=== Single character operations ===\n";

// Single char string width
var_dump(tui_string_width("a"));
var_dump(tui_string_width("中"));  // wide char
var_dump(tui_string_width("\t"));  // tab
var_dump(tui_string_width("\n"));  // newline
var_dump(tui_string_width("\r"));  // carriage return
var_dump(tui_string_width("\0"));  // null byte

// Wrap single char
$result = tui_wrap_text("a", 1);
echo "Wrap 'a' to 1: " . count($result) . " lines\n";

// Truncate to 1
$result = tui_truncate("hello", 1);
echo "Truncate to 1: '$result'\n";

// Truncate to 2 (needs at least ... which is 3)
$result = tui_truncate("hello", 2);
echo "Truncate to 2: '$result'\n";

// Truncate to 3 (exactly fits ...)
$result = tui_truncate("hello", 3);
echo "Truncate to 3: '$result'\n";

// Truncate to 4
$result = tui_truncate("hello", 4);
echo "Truncate to 4: '$result'\n";

echo "\n=== Float values ===\n";

// Floats should be converted to int
$box = new Box(['width' => 100.9, 'height' => 50.1]);
echo "Float box: {$box->width}x{$box->height}\n";

// Very small float
$box = new Box(['width' => 0.001]);
echo "Tiny float width: " . $box->width . "\n";

// Negative float
$box = new Box(['width' => -0.5]);
echo "Negative float width: " . ($box->width ?? 'null') . "\n";

echo "\n=== History capacity bounds ===\n";

// Zero capacity
$h = tui_history_create(0);
echo "Zero capacity history: " . (is_resource($h) || is_object($h) ? 'created' : 'null') . "\n";

// Capacity of 1
$h = tui_history_create(1);
tui_history_add($h, "first");
tui_history_add($h, "second");  // Should replace first
tui_history_reset($h);
$prev = tui_history_prev($h);
echo "Cap-1 history most recent: " . ($prev ?? 'null') . "\n";
tui_history_destroy($h);

// Large capacity
$h = tui_history_create(10000);
echo "Large capacity history: " . (is_resource($h) || is_object($h) ? 'created' : 'null') . "\n";
tui_history_destroy($h);

echo "\nDone!\n";
?>
--EXPECTF--
=== Zero values ===
int(0)
Wrap with 0 width: array
Truncate to 0: '%s'
Zero buffer: null
Zero box width: 0
Zero box height: 0

=== Maximum/Large values ===
Truncate to 1M: 'hi'
Wrap with 1M width: 1 lines
Large buffer: null
Large box: 10000x5000

=== Negative values ===
Negative width: %s
Negative height: %s
Negative padding set
Negative margin set

=== Percentage values ===
0% width set
100% width set
150% width set

=== Edge case dimensions ===
1x1 buffer: created
1000x1 buffer: null
1x1000 buffer: null

=== Single character operations ===
int(1)
int(2)
int(%d)
int(%d)
int(%d)
int(%d)
Wrap 'a' to 1: 1 lines
Truncate to 1: '%s'
Truncate to 2: '%s'
Truncate to 3: '%s'
Truncate to 4: '%s'

=== Float values ===
Float box: %s
Tiny float width: %s
Negative float width: %s

=== History capacity bounds ===
Zero capacity history: created
Cap-1 history most recent: second
Large capacity history: created

Done!
