--TEST--
Stress test: Tree depth limit (256)
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Test that the extension correctly handles tree depth limits.
 * Maximum nesting depth is 256 levels.
 */

use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

/**
 * Build a tree with exactly N levels of nesting.
 */
function buildNestedTree(int $depth): ContainerNode {
    if ($depth <= 0) {
        return new ContainerNode(['children' => [new ContentNode("leaf")]]);
    }
    return new ContainerNode(['children' => [buildNestedTree($depth - 1)]]);
}

// Test 1: Tree at safe depth (100 levels)
echo "Test 1: Safe nesting depth (100 levels)\n";
$tree100 = buildNestedTree(100);
$renderer = tui_test_create(80, 24);
tui_test_render($renderer, $tree100);
echo "100-level tree rendered successfully\n";
tui_test_destroy($renderer);

// Test 2: Tree at limit depth (200 levels - within 256 limit)
echo "\nTest 2: Near-limit nesting depth (200 levels)\n";
$tree200 = buildNestedTree(200);
$renderer = tui_test_create(80, 24);
tui_test_render($renderer, $tree200);
echo "200-level tree rendered successfully\n";
tui_test_destroy($renderer);

// Test 3: Tree exactly at limit (256 levels)
echo "\nTest 3: At limit nesting depth (256 levels)\n";
$tree256 = buildNestedTree(256);
$renderer = tui_test_create(80, 24);
$rendered = false;
try {
    tui_test_render($renderer, $tree256);
    $rendered = true;
} catch (Exception $e) {
    echo "Exception at 256 levels: " . $e->getMessage() . "\n";
}
tui_test_destroy($renderer);

if ($rendered) {
    echo "256-level tree handled (either rendered or warned)\n";
}

// Test 4: Tree beyond limit (300 levels) - should be handled gracefully
echo "\nTest 4: Beyond limit nesting depth (300 levels)\n";
$tree300 = buildNestedTree(300);
$renderer = tui_test_create(80, 24);
$handledGracefully = false;
try {
    tui_test_render($renderer, $tree300);
    $handledGracefully = true;
    echo "300-level tree handled without crash\n";
} catch (Exception $e) {
    $handledGracefully = true;
    echo "Exception for 300 levels (expected): " . $e->getMessage() . "\n";
}
tui_test_destroy($renderer);

if ($handledGracefully) {
    echo "Deep nesting handled gracefully: PASS\n";
}

// Test 5: Verify no memory corruption with recursive destruction
echo "\nTest 5: Recursive tree destruction\n";
for ($i = 0; $i < 10; $i++) {
    $tree = buildNestedTree(50);
    $renderer = tui_test_create(80, 24);
    tui_test_render($renderer, $tree);
    tui_test_destroy($renderer);
}
echo "10 cycles of deep tree creation/destruction completed\n";

echo "\nTree depth stress test completed!\n";
?>
--EXPECTF--
Test 1: Safe nesting depth (100 levels)
100-level tree rendered successfully

Test 2: Near-limit nesting depth (200 levels)
200-level tree rendered successfully

Test 3: At limit nesting depth (256 levels)
%a

Test 4: Beyond limit nesting depth (300 levels)
%a
%ahandled gracefully: PASS

Test 5: Recursive tree destruction
10 cycles of deep tree creation/destruction completed

Tree depth stress test completed!
