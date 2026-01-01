--TEST--
Stress test: Large children arrays and key map performance
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Test key map performance with many children and reconciliation.
 */

use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

tui_metrics_enable();
tui_metrics_reset();

// Test 1: Create node with 100 keyed children
echo "Test 1: 100 keyed children\n";
$children = [];
for ($i = 0; $i < 100; $i++) {
    $children[] = new ContentNode("item-$i", ['key' => "key-$i"]);
}
$tree1 = new ContainerNode(['children' => $children]);

$renderer = tui_test_create(200, 100);
tui_test_render($renderer, $tree1);
echo "Initial render: 100 children\n";

// Test 2: Reorder children (simulate reconciliation scenario)
// Reverse the order
$reordered = [];
for ($i = 99; $i >= 0; $i--) {
    $reordered[] = new ContentNode("item-$i-updated", ['key' => "key-$i"]);
}
$tree2 = new ContainerNode(['children' => $reordered]);
tui_test_render($renderer, $tree2);
echo "Reordered render completed\n";

// Test 3: Mix of add/remove
$mixed = [];
// Keep every other item and add new ones
for ($i = 0; $i < 50; $i++) {
    $mixed[] = new ContentNode("item-" . ($i * 2), ['key' => "key-" . ($i * 2)]);
    $mixed[] = new ContentNode("new-$i", ['key' => "new-$i"]);
}
$tree3 = new ContainerNode(['children' => $mixed]);
tui_test_render($renderer, $tree3);
echo "Mixed add/remove render completed\n";

// Check metrics
$nodeMetrics = tui_get_node_metrics();
$reconcilerMetrics = tui_get_reconciler_metrics();

echo "\nNode metrics:\n";
echo "- Total nodes: " . $nodeMetrics['node_count'] . "\n";

echo "\nReconciler metrics:\n";
echo "- Diff runs: " . $reconcilerMetrics['diff_runs'] . "\n";
echo "- Creates: " . $reconcilerMetrics['creates'] . "\n";
echo "- Updates: " . $reconcilerMetrics['updates'] . "\n";
echo "- Deletes: " . $reconcilerMetrics['deletes'] . "\n";

// Test 4: Pool metrics - check hash map is being used
$poolMetrics = tui_get_pool_metrics();
echo "\nPool metrics:\n";
echo "- Key map reuses: " . $poolMetrics['keymap_reuses'] . "\n";
echo "- Key map fallbacks: " . $poolMetrics['keymap_fallbacks'] . "\n";

tui_test_destroy($renderer);

echo "\nAll reconciliation stress tests passed!\n";
?>
--EXPECTF--
Test 1: 100 keyed children
Initial render: 100 children
Reordered render completed
Mixed add/remove render completed

Node metrics:
- Total nodes: %d

Reconciler metrics:
- Diff runs: %d
- Creates: %d
- Updates: %d
- Deletes: %d

Pool metrics:
- Key map reuses: %d
- Key map fallbacks: %d

All reconciliation stress tests passed!
