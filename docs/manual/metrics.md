# Performance Metrics

ext-tui includes a built-in telemetry system for measuring performance and debugging. When enabled, it tracks node creation, reconciliation operations, render timing, and event loop activity.

## Quick Start

```php
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

// Enable metrics
tui_metrics_enable();
tui_metrics_reset();

$instance = tui_render(function() {
    $box = new ContainerNode(['width' => '100%', 'height' => '100%']);
    $box->addChild(new ContentNode(['content' => 'Performance Demo']));
    return $box;
});

tui_set_input_handler($instance, function($key) use ($instance) {
    if ($key->key === 'q') {
        $instance->exit();
    }
});

$instance->waitUntilExit();

// Print metrics
$m = tui_get_metrics();
echo "=== Performance Report ===\n";
echo "Renders: {$m['render_count']}\n";
echo "Avg render: {$m['avg_render_ms']}ms\n";
echo "Max render: {$m['max_render_ms']}ms\n";
echo "Input events: {$m['input_events']}\n";
```

## Enabling Metrics

### Runtime Enable

```php
tui_metrics_enable();   // Start collecting
tui_metrics_disable();  // Stop collecting
tui_metrics_reset();    // Clear all counters
```

### INI Setting

Enable metrics from script start via `php.ini`:

```ini
tui.metrics_enabled = 1
```

Or per-script:

```php
ini_set('tui.metrics_enabled', '1');
```

This is useful for profiling scripts that need metrics from the very first render.

## What Gets Measured

### Node Metrics

Track the component tree structure:

- **node_count**: Total active nodes in the tree
- **box_count/text_count/static_count**: Breakdown by node type
- **max_depth**: Deepest nesting level seen

```php
$m = tui_get_node_metrics();
echo "Total nodes: {$m['node_count']}\n";
echo "  Boxes: {$m['box_count']}\n";
echo "  Text: {$m['text_count']}\n";
```

### Reconciler Metrics

Track the virtual DOM diffing efficiency:

- **diff_runs**: How many times the reconciler compared trees
- **creates/updates/deletes**: Operations by type
- **avg_ops_per_diff**: Average changes per re-render

```php
$m = tui_get_reconciler_metrics();
echo "Diff runs: {$m['diff_runs']}\n";
echo "Avg ops/diff: {$m['avg_ops_per_diff']}\n";
```

Low `avg_ops_per_diff` means your component tree is stable. High values indicate excessive DOM churn.

### Render Timing

Track time spent in each render phase (all times in milliseconds):

- **layout_time_ms**: Yoga layout calculation
- **buffer_time_ms**: Writing to character buffer
- **output_time_ms**: Terminal I/O
- **avg_render_ms**: Average total render time
- **max_render_ms/min_render_ms**: Extremes

```php
$m = tui_get_render_metrics();
echo "Avg render: {$m['avg_render_ms']}ms\n";
echo "  Layout: " . ($m['layout_time_ms'] / $m['render_count']) . "ms\n";
echo "  Buffer: " . ($m['buffer_time_ms'] / $m['render_count']) . "ms\n";
echo "  Output: " . ($m['output_time_ms'] / $m['render_count']) . "ms\n";
```

### Event Loop Metrics

Track the main loop activity:

- **loop_iterations**: Event loop cycles
- **input_events**: Keyboard inputs processed
- **resize_events**: Terminal resize events
- **timer_fires**: Timer callback executions

```php
$m = tui_get_loop_metrics();
echo "Loop iterations: {$m['loop_iterations']}\n";
echo "Inputs: {$m['input_events']}\n";
echo "Timer fires: {$m['timer_fires']}\n";
```

## Debugging Performance Issues

### Slow Renders

Check where time is spent:

```php
$m = tui_get_render_metrics();
$avg = $m['render_count'] > 0 ? $m['total_render_ms'] / $m['render_count'] : 0;

if ($avg > 16) {  // >16ms = <60fps
    echo "WARNING: Slow renders ({$avg}ms avg)\n";

    // Where is time spent?
    $layout_pct = ($m['layout_time_ms'] / $m['total_render_ms']) * 100;
    $buffer_pct = ($m['buffer_time_ms'] / $m['total_render_ms']) * 100;
    $output_pct = ($m['output_time_ms'] / $m['total_render_ms']) * 100;

    echo "  Layout: {$layout_pct}%\n";
    echo "  Buffer: {$buffer_pct}%\n";
    echo "  Output: {$output_pct}%\n";
}
```

### Excessive Re-renders

Check reconciler activity:

```php
$m = tui_get_reconciler_metrics();

if ($m['avg_ops_per_diff'] > 50) {
    echo "WARNING: High DOM churn ({$m['avg_ops_per_diff']} ops/diff)\n";
    echo "Consider:\n";
    echo "  - Use keys for list items\n";
    echo "  - Memoize stable subtrees\n";
    echo "  - Batch state updates\n";
}
```

### Memory Growth

Monitor node count over time:

```php
$timer = tui_add_timer($instance, 1000, function() {  // Check every second
    static $prev = 0;
    $m = tui_get_node_metrics();
    $delta = $m['node_count'] - $prev;
    $prev = $m['node_count'];

    if ($delta > 10) {
        echo "Node leak? +{$delta} nodes\n";
    }
});
```

## Performance Best Practices

1. **Disable in production**: Metrics add minimal overhead when disabled, but enable only when profiling.

2. **Reset before measuring**: Call `tui_metrics_reset()` to get clean measurements.

3. **Sample periodically**: Instead of checking every frame, sample metrics every second or so.

4. **Target 60fps**: Aim for <16ms average render time. If layout is slow, simplify your tree. If output is slow, reduce visible area changes.

5. **Watch reconciler efficiency**: `avg_ops_per_diff` close to 0 means only necessary updates. High values mean wasted work.

## Overhead

When disabled (default), metrics have **zero overhead**. The underlying macros early-exit when the flag is off.

When enabled:
- Counter increments: ~1ns each
- Timing calls: ~100ns per render (3 calls to `clock_gettime`)

For a typical TUI running at 60fps, enabled metrics add <0.01ms per frame.
