--TEST--
Testing framework: timer simulation (run_timers)
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

$renderer = tui_test_create(80, 24);

// Create a simple UI
$box = new ContainerNode(['width' => 80, 'height' => 24]);
$box->children = [new ContentNode("Timer test")];
tui_test_render($renderer, $box);

// Test tui_test_run_timers - advance simulated time
tui_test_run_timers($renderer, 100);
echo "run_timers 100ms: ok\n";

// Test with larger time advances
tui_test_run_timers($renderer, 1000);
echo "run_timers 1000ms: ok\n";

// Test with very small time
tui_test_run_timers($renderer, 1);
echo "run_timers 1ms: ok\n";

// Multiple timer runs should work
for ($i = 0; $i < 10; $i++) {
    tui_test_run_timers($renderer, 16);  // Typical frame time
}
echo "multiple run_timers: ok\n";

// Zero or negative ms should be handled gracefully
tui_test_run_timers($renderer, 0);
echo "run_timers 0ms: ok\n";

tui_test_destroy($renderer);
echo "Done\n";
?>
--EXPECT--
run_timers 100ms: ok
run_timers 1000ms: ok
run_timers 1ms: ok
multiple run_timers: ok
run_timers 0ms: ok
Done
