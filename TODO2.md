# TODO2: Telemetry & Performance Metrics

**Status:** DELEGATED
**Priority:** Low
**Category:** Code Quality / Performance Optimization

---

## Overview

Add optional telemetry/metrics collection to the ext-tui extension for performance tuning, debugging, and optimization. Metrics should be opt-in and have minimal overhead when disabled.

---

## Requirements

### 1. Node Tree Metrics

Track virtual DOM tree statistics:

```php
// API Example
$metrics = tui_get_metrics();
// Returns:
// [
//     'node_count' => 142,           // Total nodes in current tree
//     'box_count' => 89,             // Box nodes
//     'text_count' => 53,            // Text nodes
//     'max_depth' => 12,             // Maximum tree depth
//     'focusable_count' => 8,        // Nodes with focusable=true
// ]
```

**Implementation locations:**
- `src/node/node.c` - Increment counters in `tui_node_create_*()` functions
- `src/node/node.c` - Decrement in `tui_node_destroy()`
- Use module globals or app-level counters

### 2. Reconciliation Metrics

Track diff/patch operations:

```php
$metrics = tui_get_reconciler_metrics();
// Returns:
// [
//     'diff_runs' => 25,             // Number of diff operations
//     'total_ops' => 312,            // Total operations across all diffs
//     'creates' => 45,               // CREATE operations
//     'updates' => 198,              // UPDATE operations
//     'deletes' => 23,               // DELETE operations
//     'replaces' => 12,              // REPLACE operations
//     'reorders' => 34,              // REORDER operations
//     'avg_ops_per_diff' => 12.48,   // Average operations per diff
// ]
```

**Implementation locations:**
- `src/node/reconciler.c` - Increment counters in `diff_result_add()`
- `src/node/reconciler.c` - Track in `tui_reconciler_diff()`

### 3. Render Timing Metrics

Track rendering performance:

```php
$metrics = tui_get_render_metrics();
// Returns:
// [
//     'render_count' => 50,              // Total renders
//     'layout_time_ms' => 12.5,          // Time spent in Yoga layout
//     'buffer_time_ms' => 8.3,           // Time writing to buffer
//     'output_time_ms' => 2.1,           // Time flushing to terminal
//     'total_render_time_ms' => 22.9,    // Total render time
//     'avg_render_ms' => 0.458,          // Average per render
//     'max_render_ms' => 3.2,            // Slowest render
//     'min_render_ms' => 0.1,            // Fastest render
// ]
```

**Implementation locations:**
- `src/app/app.c` - Add timing around `tui_node_calculate_layout()`, `render_node_to_buffer()`, `tui_output_render()`
- Use `clock_gettime(CLOCK_MONOTONIC)` for high-resolution timing

### 4. Layout Metrics

Track Yoga layout engine usage:

```php
$metrics = tui_get_layout_metrics();
// Returns:
// [
//     'layout_runs' => 50,            // Full layout calculations
//     'dirty_nodes' => 12,            // Nodes marked dirty this cycle
//     'measure_calls' => 89,          // Text measure function calls
//     'baseline_calls' => 23,         // Baseline function calls
// ]
```

**Implementation locations:**
- `src/node/node.c` - Track in `tui_node_calculate_layout()`, `text_measure_func()`, `text_baseline_func()`, `node_dirtied_func()`

### 5. Event Loop Metrics

Track event loop performance:

```php
$metrics = tui_get_loop_metrics();
// Returns:
// [
//     'loop_iterations' => 500,       // Event loop cycles
//     'input_events' => 45,           // Keyboard inputs processed
//     'resize_events' => 2,           // Terminal resizes
//     'timer_fires' => 100,           // Timer callbacks executed
//     'tick_callbacks' => 500,        // Tick callbacks
//     'avg_loop_ms' => 16.5,          // Average loop iteration time
// ]
```

**Implementation locations:**
- `src/event/loop.c` - Track in `tui_loop_run()` and callback invocations

---

## Configuration

### PHP INI Settings

```ini
; Enable/disable metrics collection (default: 0)
tui.metrics_enabled = 1

; Reset metrics on each render (default: 0)
tui.metrics_reset_on_render = 0
```

### Runtime Control

```php
// Enable metrics at runtime
tui_metrics_enable();

// Disable metrics
tui_metrics_disable();

// Reset all counters
tui_metrics_reset();

// Check if enabled
$enabled = tui_metrics_enabled();
```

---

## Implementation Notes

### Data Structures

Add to `php_tui.h` module globals:

```c
typedef struct {
    int enabled;

    /* Node metrics */
    int64_t node_count;
    int64_t box_count;
    int64_t text_count;
    int max_depth;

    /* Reconciler metrics */
    int64_t diff_runs;
    int64_t create_ops;
    int64_t update_ops;
    int64_t delete_ops;
    int64_t replace_ops;
    int64_t reorder_ops;

    /* Timing (nanoseconds) */
    int64_t layout_time_ns;
    int64_t buffer_time_ns;
    int64_t output_time_ns;
    int64_t render_count;
    int64_t max_render_ns;
    int64_t min_render_ns;

    /* Event loop */
    int64_t loop_iterations;
    int64_t input_events;
    int64_t resize_events;
    int64_t timer_fires;
} tui_metrics;

ZEND_BEGIN_MODULE_GLOBALS(tui)
    YGConfigRef yoga_config;
    tui_metrics metrics;
ZEND_END_MODULE_GLOBALS(tui)
```

### Overhead Considerations

- Use `TUI_G(metrics).enabled` check before any counter increment
- Use inline functions or macros for hot paths
- Consider thread-local counters for ZTS builds

```c
#define TUI_METRIC_INC(field) \
    do { if (TUI_G(metrics).enabled) TUI_G(metrics).field++; } while(0)

#define TUI_METRIC_ADD(field, val) \
    do { if (TUI_G(metrics).enabled) TUI_G(metrics).field += (val); } while(0)
```

### Example Usage

```php
<?php
// Enable metrics before rendering
tui_metrics_enable();

$instance = tui_render(function() {
    return new Box([
        'children' => array_map(fn($i) =>
            new Text("Item $i"), range(1, 100)
        )
    ]);
});

// After some interactions...
$instance->exit();
$instance->waitUntilExit();

// Get performance report
$metrics = tui_get_metrics();
echo "Total renders: {$metrics['render_count']}\n";
echo "Avg render time: {$metrics['avg_render_ms']}ms\n";
echo "Node count: {$metrics['node_count']}\n";
echo "Diff operations: {$metrics['total_ops']}\n";
```

---

## Files to Modify

| File | Changes |
|------|---------|
| `php_tui.h` | Add tui_metrics struct to module globals |
| `tui.c` | Add PHP functions, INI settings, initialization |
| `src/node/node.c` | Node creation/destruction counters |
| `src/node/reconciler.c` | Diff operation counters |
| `src/app/app.c` | Render timing |
| `src/event/loop.c` | Event loop metrics |

---

## Testing

Create test file `tests/metrics.phpt`:

```
--TEST--
TUI Metrics collection
--EXTENSIONS--
tui
--FILE--
<?php
tui_metrics_enable();
$m = tui_get_metrics();
var_dump($m['node_count'] === 0);

// Create some nodes
$box = new Xocdr\Tui\Ext\Box();
$box->addChild(new Xocdr\Tui\Ext\Text("Hello"));

$m = tui_get_metrics();
var_dump($m['node_count'] > 0);
?>
--EXPECT--
bool(true)
bool(true)
```

---

## Estimated Effort

- Implementation: 4-6 hours
- Testing: 2 hours
- Documentation: 1 hour

**Total:** ~8 hours

---

## References

- L4 in CODE_REVIEW.md: "No Telemetry/Metrics" suggestion
- Similar to Node.js diagnostic_channel or PHP's xhprof
