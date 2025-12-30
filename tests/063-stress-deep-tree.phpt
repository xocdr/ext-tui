--TEST--
Stress test: Deep node tree handling
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Test that the extension handles deeply nested trees correctly
 * and properly cleans up memory on errors.
 */

use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

// Build a deep tree
function buildDeepTree(int $depth, int $childrenPerLevel = 2): Box {
    if ($depth <= 0) {
        return new Box(['children' => [new Text("leaf")]]);
    }

    $children = [];
    for ($i = 0; $i < $childrenPerLevel; $i++) {
        $children[] = buildDeepTree($depth - 1, $childrenPerLevel);
    }
    return new Box(['children' => $children]);
}

// Test 1: Build and render moderately deep tree (10 levels, 2 children each = 2047 nodes)
echo "Test 1: Moderate tree depth (10 levels)\n";
$tree = buildDeepTree(10, 2);
$renderer = tui_test_create(80, 24);
tui_test_render($renderer, $tree);
echo "Rendered successfully\n";

// Get metrics
tui_metrics_enable();
tui_metrics_reset();

// Test 2: Build wide but shallow tree (1000 children)
echo "\nTest 2: Wide tree (1000 children)\n";
$children = [];
for ($i = 0; $i < 1000; $i++) {
    $children[] = new Text("item-$i", ['key' => "key-$i"]);
}
$wideTree = new Box(['children' => $children]);

$renderer2 = tui_test_create(200, 100);
tui_test_render($renderer2, $wideTree);
echo "Wide tree rendered successfully\n";

// Check pool metrics after stress
$poolMetrics = tui_get_pool_metrics();
echo "Pool children allocs: " . ($poolMetrics['children_allocs'] ?? 0) . "\n";
echo "Pool fallbacks: " . ($poolMetrics['children_fallbacks'] ?? 0) . "\n";

// Test 3: Verify cleanup after destroying renderers
tui_test_destroy($renderer);
tui_test_destroy($renderer2);
echo "\nCleanup completed\n";

// Test 4: Memory stress - many small allocations
echo "\nTest 4: Many small trees\n";
for ($i = 0; $i < 100; $i++) {
    $smallTree = new Box(['children' => [
        new Text("a"),
        new Text("b"),
    ]]);
    $r = tui_test_create(20, 5);
    tui_test_render($r, $smallTree);
    tui_test_destroy($r);
}
echo "100 render cycles completed\n";

echo "\nAll stress tests passed!\n";
?>
--EXPECTF--
Test 1: Moderate tree depth (10 levels)
Rendered successfully

Test 2: Wide tree (1000 children)
Wide tree rendered successfully
Pool children allocs: %d
Pool fallbacks: %d

Cleanup completed

Test 4: Many small trees
100 render cycles completed

All stress tests passed!
