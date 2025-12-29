# TODO3: Object Pooling for Memory Optimization

**Status:** DELEGATED
**Priority:** Low
**Category:** Performance / Code Quality
**Source:** L3 in CODE_REVIEW.md - "Potential Memory Fragmentation"

---

## Overview

Frequent `malloc()`/`realloc()` of diff operations, node arrays, and key maps can cause memory fragmentation over long-running TUI sessions. Implementing object pools for frequently allocated structures would reduce fragmentation and improve performance.

---

## Problem Analysis

### Current Hot Paths

1. **Diff Operations** (`src/node/reconciler.c`)
   - Created every reconciliation (potentially 60 times/second)
   - `diff_result_create()` allocates 16 ops initially, grows dynamically
   - Each diff can have dozens of operations

2. **Node Children Arrays** (`src/node/node.c`)
   - Every Box/Static node has a children array
   - Starts at 4, doubles on growth (4→8→16→32...)
   - Churn when tree is rebuilt each render

3. **Key Map Entries** (`src/node/reconciler.c`)
   - Temporary during reconciliation
   - Allocated/freed every diff cycle

### Symptoms

- Memory fragmentation in long-running sessions
- Potential allocation latency spikes
- Excessive system calls for small allocations

---

## Proposed Solution: Simple Object Pools

### 1. Diff Operation Pool

```c
/* Pool for diff operations - single slab allocator */
#define DIFF_OP_POOL_SIZE 256

typedef struct {
    tui_diff_op ops[DIFF_OP_POOL_SIZE];
    uint8_t in_use[DIFF_OP_POOL_SIZE];
    int free_count;
    int next_free_hint;  /* Start search here for O(1) best case */
} tui_diff_op_pool;

static tui_diff_op_pool g_diff_op_pool;

/* Initialize pool (call in MINIT) */
void tui_diff_op_pool_init(void);

/* Get operation from pool, or malloc if pool exhausted */
tui_diff_op* tui_diff_op_alloc(void);

/* Return operation to pool */
void tui_diff_op_free(tui_diff_op *op);
```

### 2. Node Children Array Pool

Pool pre-allocated arrays of common sizes:

```c
/* Pool of children arrays by size class */
#define POOL_SIZE_CLASSES 4  /* 4, 8, 16, 32 */

typedef struct {
    tui_node **arrays_4[16];    /* Pool of 4-element arrays */
    tui_node **arrays_8[16];    /* Pool of 8-element arrays */
    tui_node **arrays_16[8];    /* Pool of 16-element arrays */
    tui_node **arrays_32[4];    /* Pool of 32-element arrays */
    int counts[POOL_SIZE_CLASSES];
} tui_children_pool;

/* Get array of at least `capacity` elements */
tui_node** tui_children_pool_alloc(int capacity, int *actual_capacity);

/* Return array to pool */
void tui_children_pool_free(tui_node **array, int capacity);
```

### 3. Key Map Pool

```c
/* Reusable key map - single instance per reconciler */
typedef struct {
    key_map map;
    int in_use;
} tui_key_map_pool;

/* Get/create the shared key map (cleared between uses) */
key_map* tui_key_map_acquire(void);

/* Mark key map as available for reuse */
void tui_key_map_release(key_map *map);
```

---

## Implementation Strategy

### Phase 1: Measurement

Before implementing, add metrics to understand allocation patterns:

```c
/* Temporary metrics gathering */
static struct {
    int64_t diff_ops_allocated;
    int64_t diff_ops_reused;
    int64_t children_arrays_allocated;
    int64_t children_arrays_reused;
} g_pool_metrics;
```

### Phase 2: Diff Op Pool

Start with the simplest, highest-frequency case:

1. Add pool data structure to module globals
2. Initialize in MINIT, cleanup in MSHUTDOWN
3. Modify `diff_result_add()` to use pool
4. Modify `tui_reconciler_free_diff()` to return to pool

### Phase 3: Children Array Pool

1. Add pool to module globals
2. Modify `tui_node_append_child()` to use pool for realloc
3. Modify `tui_node_destroy()` to return arrays to pool

### Phase 4: Key Map Reuse

1. Create single reusable key map per reconciler instance
2. Clear and reuse instead of create/destroy

---

## Thread Safety (ZTS Builds)

In ZTS builds, pools should be thread-local:

```c
#ifdef ZTS
#define TUI_POOL_G(field) TSRMG(tui_pool_globals_id, tui_pool_globals *, field)
#else
#define TUI_POOL_G(field) (tui_pool_globals.field)
#endif
```

---

## Testing

### Unit Tests

```php
--TEST--
Object pool performance
--EXTENSIONS--
tui
--FILE--
<?php
// Measure time for 1000 renders
$start = hrtime(true);
for ($i = 0; $i < 1000; $i++) {
    // Create complex tree
    $box = new Box(['children' => array_map(
        fn($n) => new Text("Item $n"),
        range(1, 100)
    )]);
    // Force reconciliation
    tui_internal_reconcile($box);
}
$elapsed = (hrtime(true) - $start) / 1e6;
echo "1000 reconciliations: {$elapsed}ms\n";
?>
--EXPECTF--
1000 reconciliations: %fms
```

### Benchmark Comparison

Before and after pool implementation:

1. Memory usage over time (should be more stable)
2. Allocation count (should decrease significantly)
3. Render latency p99 (should improve)

---

## Files to Modify

| File | Changes |
|------|---------|
| `php_tui.h` | Add pool globals |
| `tui.c` | Pool init/cleanup in MINIT/MSHUTDOWN |
| `src/node/reconciler.c` | Use diff op pool, key map reuse |
| `src/node/node.c` | Use children array pool |

---

## Estimated Effort

- Phase 1 (Metrics): 1-2 hours
- Phase 2 (Diff Pool): 2-3 hours
- Phase 3 (Children Pool): 3-4 hours
- Phase 4 (Key Map): 1-2 hours
- Testing/Benchmarking: 2-3 hours

**Total:** ~12 hours

---

## Success Criteria

1. Memory usage stays stable during 10-minute session
2. No increase in render latency
3. 50%+ reduction in malloc calls during reconciliation
4. All existing tests pass
