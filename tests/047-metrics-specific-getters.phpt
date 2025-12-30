--TEST--
Telemetry: specific metrics getter functions
--EXTENSIONS--
tui
--FILE--
<?php
tui_metrics_enable();
tui_metrics_reset();

// Test tui_get_node_metrics
$m = tui_get_node_metrics();
var_dump(array_key_exists('node_count', $m));
var_dump(array_key_exists('box_count', $m));
var_dump(array_key_exists('text_count', $m));
var_dump(array_key_exists('static_count', $m));
var_dump(array_key_exists('max_depth', $m));

// Test tui_get_reconciler_metrics
$m = tui_get_reconciler_metrics();
var_dump(array_key_exists('diff_runs', $m));
var_dump(array_key_exists('total_ops', $m));
var_dump(array_key_exists('creates', $m));
var_dump(array_key_exists('updates', $m));
var_dump(array_key_exists('deletes', $m));
var_dump(array_key_exists('avg_ops_per_diff', $m));

// Test tui_get_render_metrics
$m = tui_get_render_metrics();
var_dump(array_key_exists('render_count', $m));
var_dump(array_key_exists('layout_time_ms', $m));
var_dump(array_key_exists('buffer_time_ms', $m));
var_dump(array_key_exists('output_time_ms', $m));
var_dump(array_key_exists('avg_render_ms', $m));

// Test tui_get_loop_metrics
$m = tui_get_loop_metrics();
var_dump(array_key_exists('loop_iterations', $m));
var_dump(array_key_exists('input_events', $m));
var_dump(array_key_exists('resize_events', $m));
var_dump(array_key_exists('timer_fires', $m));

echo "Done\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
Done
