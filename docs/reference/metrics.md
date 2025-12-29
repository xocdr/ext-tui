# Metrics Reference

Functions for collecting performance metrics and telemetry data.

## Enabling/Disabling Metrics

### tui_metrics_enable

```php
tui_metrics_enable(): void
```

Enables metrics collection. Metrics are disabled by default to minimize overhead in production.

### tui_metrics_disable

```php
tui_metrics_disable(): void
```

Disables metrics collection. Stops all tracking but preserves existing metrics data.

### tui_metrics_enabled

```php
tui_metrics_enabled(): bool
```

Returns `true` if metrics collection is currently enabled.

### tui_metrics_reset

```php
tui_metrics_reset(): void
```

Resets all metric counters to zero. Does not change enabled/disabled state.

---

## INI Setting

### tui.metrics_enabled

```ini
tui.metrics_enabled = 0
```

Set to `1` to enable metrics collection from script startup. Can be set in `php.ini` or per-script:

```php
ini_set('tui.metrics_enabled', '1');
```

---

## Retrieving Metrics

### tui_get_metrics

```php
tui_get_metrics(): array
```

Returns all metrics in a single associative array:

```php
[
    // Node metrics
    'node_count' => int,       // Total active nodes
    'box_count' => int,        // Active Box nodes
    'text_count' => int,       // Active Text nodes
    'static_count' => int,     // Active Static nodes
    'max_depth' => int,        // Maximum tree depth seen

    // Reconciler metrics
    'diff_runs' => int,        // Number of reconciliation runs
    'total_ops' => int,        // Total diff operations
    'creates' => int,          // Node creation operations
    'updates' => int,          // Node update operations
    'deletes' => int,          // Node deletion operations
    'replaces' => int,         // Node replacement operations
    'reorders' => int,         // Node reorder operations
    'avg_ops_per_diff' => float,  // Average operations per diff

    // Render timing (milliseconds)
    'render_count' => int,     // Number of renders
    'layout_time_ms' => float, // Total layout calculation time
    'buffer_time_ms' => float, // Total buffer rendering time
    'output_time_ms' => float, // Total terminal output time
    'total_render_ms' => float, // Total render time
    'avg_render_ms' => float,  // Average render time
    'max_render_ms' => float,  // Maximum single render time
    'min_render_ms' => float,  // Minimum single render time

    // Layout metrics
    'layout_runs' => int,      // Yoga layout calculations
    'measure_calls' => int,    // Text measurement callbacks
    'baseline_calls' => int,   // Baseline calculation callbacks

    // Event loop metrics
    'loop_iterations' => int,  // Event loop cycles
    'input_events' => int,     // Keyboard input events
    'resize_events' => int,    // Terminal resize events
    'timer_fires' => int,      // Timer callback executions

    // Pool metrics (also available via tui_get_pool_metrics with different keys)
    'pool_children_hits' => int,     // Pool allocations (hits)
    'pool_children_misses' => int,   // Malloc fallbacks (misses)
    'pool_children_returns' => int,  // Arrays returned to pool
    'pool_keymap_reuses' => int,     // Key map reuses
]
```

---

## Specific Metric Getters

### tui_get_node_metrics

```php
tui_get_node_metrics(): array
```

Returns only node-related metrics:

```php
[
    'node_count' => int,
    'box_count' => int,
    'text_count' => int,
    'static_count' => int,
    'max_depth' => int,
]
```

### tui_get_reconciler_metrics

```php
tui_get_reconciler_metrics(): array
```

Returns only reconciler-related metrics:

```php
[
    'diff_runs' => int,
    'total_ops' => int,
    'creates' => int,
    'updates' => int,
    'deletes' => int,
    'replaces' => int,
    'reorders' => int,
    'avg_ops_per_diff' => float,
]
```

### tui_get_render_metrics

```php
tui_get_render_metrics(): array
```

Returns only render timing metrics:

```php
[
    'render_count' => int,
    'layout_time_ms' => float,
    'buffer_time_ms' => float,
    'output_time_ms' => float,
    'total_render_ms' => float,
    'avg_render_ms' => float,
    'max_render_ms' => float,
    'min_render_ms' => float,
]
```

### tui_get_loop_metrics

```php
tui_get_loop_metrics(): array
```

Returns only event loop metrics:

```php
[
    'loop_iterations' => int,
    'input_events' => int,
    'resize_events' => int,
    'timer_fires' => int,
]
```

### tui_get_pool_metrics

```php
tui_get_pool_metrics(): array
```

Returns object pool metrics with user-friendly key names:

```php
[
    'children_allocs' => int,      // Successful pool allocations (internal: children_hits)
    'children_fallbacks' => int,   // Allocations that fell back to malloc (internal: children_misses)
    'children_reuses' => int,      // Arrays returned to pool (internal: children_returns)
    'keymap_reuses' => int,        // Key map reuses
    'children_hit_rate' => float,  // Pool hit rate percentage (allocs / (allocs + fallbacks))
]
```

> **Note:** The same metrics are also included in `tui_get_metrics()` with `pool_` prefix and internal naming:
> `pool_children_hits`, `pool_children_misses`, `pool_children_returns`, `pool_keymap_reuses`.

---

## Performance Overhead

When disabled (default), metrics collection has **zero overhead**. The macros check the enabled flag before any operation:

```c
#define TUI_METRIC_INC(field) \
    do { if (TUI_G(metrics_enabled)) TUI_G(metrics).field++; } while(0)
```

When enabled, overhead is minimal:
- Counter increments: ~1 nanosecond each
- Render timing: Uses `clock_gettime(CLOCK_MONOTONIC)` (typically <100ns per call)

---

## Example Usage

```php
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

// Enable metrics before starting
tui_metrics_enable();
tui_metrics_reset();

$instance = tui_render(function() {
    $box = new Box(['padding' => 1]);
    $box->addChild(new Text("Hello!"));
    return $box;
});

// Run for a while...
sleep(5);
$instance->unmount();

// Get performance data
$metrics = tui_get_metrics();
echo "Renders: {$metrics['render_count']}\n";
echo "Avg render time: {$metrics['avg_render_ms']}ms\n";
echo "Node count: {$metrics['node_count']}\n";
```
