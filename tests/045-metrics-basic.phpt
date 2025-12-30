--TEST--
Telemetry: basic metrics functions
--EXTENSIONS--
tui
--FILE--
<?php
// Test metrics_enabled starts false by default
var_dump(tui_metrics_enabled());

// Enable metrics
tui_metrics_enable();
var_dump(tui_metrics_enabled());

// Get metrics (should be zeroes)
$m = tui_get_metrics();
var_dump(is_array($m));
var_dump($m['node_count'] === 0);
var_dump($m['diff_runs'] === 0);
var_dump($m['render_count'] === 0);

// Disable metrics
tui_metrics_disable();
var_dump(tui_metrics_enabled());

// Reset metrics
tui_metrics_reset();
$m = tui_get_metrics();
var_dump($m['node_count'] === 0);

echo "Done\n";
?>
--EXPECT--
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(true)
Done
