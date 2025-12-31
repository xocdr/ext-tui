--TEST--
Stress test: Rapid consecutive rerenders
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

echo "=== Test 1: 100 rapid rerenders ===\n";
$renderer = tui_test_create(80, 24);
$renderCount = 0;

$start = microtime(true);

for ($i = 0; $i < 100; $i++) {
    $tree = new Box([
        'children' => [
            new Text("Render #$i")
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

    $tree = new Box([
        'children' => [
            new Box(['children' => [new Text("State: $state")]]),
            new Box(['children' => [new Text("Iteration: $i")]])
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
        $tree = new Box(['children' => [new Text("Single")]]);
    } elseif ($i % 3 === 1) {
        // Multiple children
        $tree = new Box([
            'children' => [
                new Text("First"),
                new Text("Second"),
                new Text("Third")
            ]
        ]);
    } else {
        // Nested structure
        $tree = new Box([
            'children' => [
                new Box(['children' => [new Text("Nested")]])
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
$tree = new Box(['children' => [new Text("Initial")]]);
tui_test_render($renderer, $tree);

// Re-render same tree structure multiple times
for ($i = 0; $i < 5; $i++) {
    $tree = new Box(['children' => [new Text("Initial")]]);
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
        $children[] = new Text("Item $j");
    }
    $tree = new Box(['children' => $children]);
    tui_test_render($renderer, $tree);
}

echo "Completed complexity scaling test (1-20 children)\n";

tui_test_destroy($renderer);

echo "\n=== Test 6: Memory stability check ===\n";
$initialMem = memory_get_usage(true);

$renderer = tui_test_create(80, 24);
for ($i = 0; $i < 100; $i++) {
    $tree = new Box([
        'children' => [
            new Text("Memory test iteration $i"),
            new Box(['children' => [new Text("Nested content")]])
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
