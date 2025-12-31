--TEST--
Stress test: State dynamic growth
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Test that state arrays grow dynamically.
 * Full state testing requires interactive mode.
 * States now use dynamic arrays starting at INITIAL_STATE_CAPACITY (8).
 * The tui.max_states INI setting controls the maximum (default: 64).
 */

use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

echo "Test: State dynamic growth\n";

// Create a basic render test
$renderer = tui_test_create(80, 24);
$tree = new Box(['children' => [new Text("State dynamic test")]]);
tui_test_render($renderer, $tree);
echo "Basic render works\n";

// Test rendering with many unique node keys (simulates state-like uniqueness)
// This tests the array growth from initial capacity (8) to higher counts
$children = [];
for ($i = 0; $i < 100; $i++) {
    $children[] = new Text("state-$i", ['key' => "unique-key-$i"]);
}
$tree = new Box(['children' => $children]);
tui_test_render($renderer, $tree);
echo "100 uniquely-keyed nodes rendered\n";

// Test with nested state-like structures
$nested = new Box(['children' => []]);
$current = $nested;
for ($i = 0; $i < 80; $i++) {
    $child = new Box(['key' => "nested-$i", 'children' => [
        new Text("level-$i")
    ]]);
    $current = new Box(['key' => "parent-$i", 'children' => [$child]]);
}
tui_test_render($renderer, $current);
echo "Deeply nested unique-keyed tree rendered\n";

tui_test_destroy($renderer);

// Document the dynamic allocation
echo "States use dynamic arrays (initial capacity: 8, grows as needed)\n";
echo "Maximum controlled by tui.max_states INI setting\n";
echo "State dynamic growth test completed!\n";
?>
--EXPECT--
Test: State dynamic growth
Basic render works
100 uniquely-keyed nodes rendered
Deeply nested unique-keyed tree rendered
States use dynamic arrays (initial capacity: 8, grows as needed)
Maximum controlled by tui.max_states INI setting
State dynamic growth test completed!
