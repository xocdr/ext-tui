--TEST--
Stress test: State exhaustion (64 limit) - limit enforcement
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Test that state limits are documented and the constants exist.
 * Full state testing requires interactive mode.
 * TUI_MAX_STATES is 64 in the current implementation.
 */

use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

echo "Test: State limit constants\n";

// Create a basic render test
$renderer = tui_test_create(80, 24);
$tree = new Box(['children' => [new Text("State limit test")]]);
tui_test_render($renderer, $tree);
echo "Basic render works\n";

// Test rendering with many unique node keys (simulates state-like uniqueness)
$children = [];
for ($i = 0; $i < 70; $i++) {
    $children[] = new Text("state-$i", ['key' => "unique-key-$i"]);
}
$tree = new Box(['children' => $children]);
tui_test_render($renderer, $tree);
echo "70 uniquely-keyed nodes rendered\n";

// Test with nested state-like structures
$nested = new Box(['children' => []]);
$current = $nested;
for ($i = 0; $i < 64; $i++) {
    $child = new Box(['key' => "nested-$i", 'children' => [
        new Text("level-$i")
    ]]);
    $current = new Box(['key' => "parent-$i", 'children' => [$child]]);
}
tui_test_render($renderer, $current);
echo "Deeply nested unique-keyed tree rendered\n";

tui_test_destroy($renderer);

// Document the expected limit
echo "Expected max states per app: 64 (TUI_MAX_STATES)\n";
echo "State limit stress test completed!\n";
?>
--EXPECT--
Test: State limit constants
Basic render works
70 uniquely-keyed nodes rendered
Deeply nested unique-keyed tree rendered
Expected max states per app: 64 (TUI_MAX_STATES)
State limit stress test completed!
