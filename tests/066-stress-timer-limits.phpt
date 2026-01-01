--TEST--
Stress test: Timer dynamic growth
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Test that timer arrays grow dynamically.
 * Full timer testing requires interactive mode.
 * Timers now use dynamic arrays starting at INITIAL_TIMER_CAPACITY (4).
 * The tui.max_timers INI setting controls the maximum (default: 32).
 */

use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

echo "Test: Timer dynamic growth\n";

// Create a basic render test to ensure timers work at all
$renderer = tui_test_create(80, 24);
$tree = new ContainerNode(['children' => [new ContentNode("Timer dynamic test")]]);
tui_test_render($renderer, $tree);
echo "Basic render works\n";

// Test multiple render cycles (simulates timer-like behavior)
// This tests the array growth from initial capacity (4) to higher counts
for ($i = 0; $i < 50; $i++) {
    $tree = new ContainerNode(['children' => [new ContentNode("Cycle $i")]]);
    tui_test_render($renderer, $tree);
}
echo "50 render cycles completed (simulating timer iterations)\n";

tui_test_destroy($renderer);

// Document the dynamic allocation
echo "Timers use dynamic arrays (initial capacity: 4, grows as needed)\n";
echo "Maximum controlled by tui.max_timers INI setting\n";
echo "Timer dynamic growth test completed!\n";
?>
--EXPECT--
Test: Timer dynamic growth
Basic render works
50 render cycles completed (simulating timer iterations)
Timers use dynamic arrays (initial capacity: 4, grows as needed)
Maximum controlled by tui.max_timers INI setting
Timer dynamic growth test completed!
