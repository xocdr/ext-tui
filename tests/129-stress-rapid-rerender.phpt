--TEST--
Stress test: Rapid consecutive rerenders
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

echo "=== Test 1: 100 rapid rerenders ===\n";
$renderer = tui_test_create(80, 24);
$renderCount = 0;

$start = microtime(true);

for ($i = 0; $i < 100; $i++) {
    $tree = new ContainerNode([
        'children' => [
            new ContentNode("Render #$i")
        ]
    ]);
    tui_test_render($renderer, $tree);
    $renderCount++;
}

$elapsed = microtime(true) - $start;
echo "Completed $renderCount renders\n";
echo "Time: " . ($elapsed < 1 ? "under 1 second" : round($elapsed, 2) . " seconds") . "\n";

// Verify final state - tui_test_get_output returns array of lines
$output = tui_test_get_output($renderer);
$outputStr = is_array($output) ? implode("\n", $output) : $output;
echo "Final output contains: " . (strpos($outputStr, "Render #99") !== false ? "Render #99" : "unexpected content") . "\n";

tui_test_destroy($renderer);

echo "\n=== Test 2: Rapid rerenders with state changes ===\n";
$renderer = tui_test_create(80, 24);
$states = ['loading', 'loaded', 'error', 'retry', 'success'];
$stateIndex = 0;

for ($i = 0; $i < 50; $i++) {
    $state = $states[$stateIndex];
    $stateIndex = ($stateIndex + 1) % count($states);

    $tree = new ContainerNode([
        'children' => [
            new ContainerNode(['children' => [new ContentNode("State: $state")]]),
            new ContainerNode(['children' => [new ContentNode("Iteration: $i")]])
        ]
    ]);
    tui_test_render($renderer, $tree);
}

echo "Completed 50 state-change renders\n";
$output = tui_test_get_output($renderer);
$outputStr = is_array($output) ? implode("\n", $output) : $output;
echo "Final state visible: " . (strpos($outputStr, "State: ") !== false ? "yes" : "no") . "\n";

tui_test_destroy($renderer);

echo "\n=== Test 3: Rapid rerenders with tree structure changes ===\n";
$renderer = tui_test_create(80, 24);

for ($i = 0; $i < 30; $i++) {
    // Alternate between different tree structures
    if ($i % 3 === 0) {
        // Single child
        $tree = new ContainerNode(['children' => [new ContentNode("Single")]]);
    } elseif ($i % 3 === 1) {
        // Multiple children
        $tree = new ContainerNode([
            'children' => [
                new ContentNode("First"),
                new ContentNode("Second"),
                new ContentNode("Third")
            ]
        ]);
    } else {
        // Nested structure
        $tree = new ContainerNode([
            'children' => [
                new ContainerNode(['children' => [new ContentNode("Nested")]])
            ]
        ]);
    }
    tui_test_render($renderer, $tree);
}

echo "Completed 30 structure-change renders\n";

tui_test_destroy($renderer);

echo "\n=== Test 4: Multiple render calls on same tree ===\n";
$renderer = tui_test_create(80, 24);

// Initial render
$tree = new ContainerNode(['children' => [new ContentNode("Initial")]]);
tui_test_render($renderer, $tree);

// Re-render same tree structure multiple times
for ($i = 0; $i < 5; $i++) {
    $tree = new ContainerNode(['children' => [new ContentNode("Initial")]]);
    tui_test_render($renderer, $tree);
}

echo "Completed 5 re-renders of same structure\n";

$output = tui_test_get_output($renderer);
$outputStr = is_array($output) ? implode("\n", $output) : $output;
echo "Content preserved: " . (strpos($outputStr, "Initial") !== false ? "yes" : "no") . "\n";

tui_test_destroy($renderer);

echo "\n=== Test 5: Rerender with increasing complexity ===\n";
$renderer = tui_test_create(120, 40);

for ($size = 1; $size <= 20; $size++) {
    $children = [];
    for ($j = 0; $j < $size; $j++) {
        $children[] = new ContentNode("Item $j");
    }
    $tree = new ContainerNode(['children' => $children]);
    tui_test_render($renderer, $tree);
}

echo "Completed complexity scaling test (1-20 children)\n";

tui_test_destroy($renderer);

echo "\n=== Test 6: Memory stability check ===\n";
$initialMem = memory_get_usage(true);

$renderer = tui_test_create(80, 24);
for ($i = 0; $i < 100; $i++) {
    $tree = new ContainerNode([
        'children' => [
            new ContentNode("Memory test iteration $i"),
            new ContainerNode(['children' => [new ContentNode("Nested content")]])
        ]
    ]);
    tui_test_render($renderer, $tree);
}
tui_test_destroy($renderer);

$finalMem = memory_get_usage(true);
$memDiff = $finalMem - $initialMem;

// Memory shouldn't grow significantly (allow 2MB margin for normal fluctuations)
echo "Memory stable: " . ($memDiff < 2 * 1024 * 1024 ? "yes" : "no (grew by " . round($memDiff / 1024 / 1024, 2) . "MB)") . "\n";

echo "\nStress test completed!\n";
?>
--EXPECTF--
=== Test 1: 100 rapid rerenders ===
Completed 100 renders
Time: under 1 second
Final output contains: Render #99

=== Test 2: Rapid rerenders with state changes ===
Completed 50 state-change renders
Final state visible: yes

=== Test 3: Rapid rerenders with tree structure changes ===
Completed 30 structure-change renders

=== Test 4: Multiple render calls on same tree ===
Completed 5 re-renders of same structure
Content preserved: yes

=== Test 5: Rerender with increasing complexity ===
Completed complexity scaling test (1-20 children)

=== Test 6: Memory stability check ===
Memory stable: yes

Stress test completed!
