--TEST--
Security: String length limits for key, id, and text content
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Test that string length limits are enforced and truncation works.
 * Limits:
 * - TUI_MAX_KEY_LENGTH: 256 chars
 * - TUI_MAX_ID_LENGTH: 256 chars
 * - TUI_MAX_TEXT_LENGTH: 1048576 chars (1MB)
 */

use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

echo "Test 1: Normal key/id lengths\n";
$renderer = tui_test_create(80, 24);
$tree = new Box(['key' => 'normal-key', 'id' => 'normal-id', 'children' => [
    new Text("hello", ['key' => 'text-key', 'id' => 'text-id'])
]]);
tui_test_render($renderer, $tree);
echo "Normal lengths work\n";
tui_test_destroy($renderer);

echo "\nTest 2: Long key (>256 chars) should truncate with warning\n";
$longKey = str_repeat('k', 300);
$renderer = tui_test_create(80, 24);
$tree = new Box(['key' => $longKey, 'children' => [new Text("test")]]);
tui_test_render($renderer, $tree);
echo "Long key handled\n";
tui_test_destroy($renderer);

echo "\nTest 3: Long id (>256 chars) should truncate with warning\n";
$longId = str_repeat('i', 300);
$renderer = tui_test_create(80, 24);
$tree = new Box(['id' => $longId, 'children' => [new Text("test")]]);
tui_test_render($renderer, $tree);
echo "Long id handled\n";
tui_test_destroy($renderer);

echo "\nTest 4: Text key and id limits\n";
$renderer = tui_test_create(80, 24);
$tree = new Box(['children' => [
    new Text("content", ['key' => str_repeat('x', 300), 'id' => str_repeat('y', 300)])
]]);
tui_test_render($renderer, $tree);
echo "Text key/id limits handled\n";
tui_test_destroy($renderer);

echo "\nTest 5: Maximum allowed lengths (exactly 256)\n";
$maxKey = str_repeat('a', 256);
$maxId = str_repeat('b', 256);
$renderer = tui_test_create(80, 24);
$tree = new Box(['key' => $maxKey, 'id' => $maxId, 'children' => [new Text("ok")]]);
tui_test_render($renderer, $tree);
echo "Maximum allowed lengths work without warning\n";
tui_test_destroy($renderer);

echo "\nTest 6: Large text content\n";
$largeText = str_repeat('A', 100000);  // 100KB - well under 1MB limit
$renderer = tui_test_create(80, 24);
$tree = new Box(['children' => [new Text($largeText)]]);
tui_test_render($renderer, $tree);
echo "100KB text content handled\n";
tui_test_destroy($renderer);

echo "\nSecurity string limits test completed!\n";
?>
--EXPECTF--
Test 1: Normal key/id lengths
Normal lengths work

Test 2: Long key (>256 chars) should truncate with warning

Warning: %s: Box key exceeds maximum length (256), truncating in %s on line %d
Long key handled

Test 3: Long id (>256 chars) should truncate with warning

Warning: %s: Box id exceeds maximum length (256), truncating in %s on line %d
Long id handled

Test 4: Text key and id limits

Warning: %s: Text key exceeds maximum length (256), truncating in %s on line %d

Warning: %s: Text id exceeds maximum length (256), truncating in %s on line %d
Text key/id limits handled

Test 5: Maximum allowed lengths (exactly 256)
Maximum allowed lengths work without warning

Test 6: Large text content
100KB text content handled

Security string limits test completed!
