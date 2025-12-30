# Scrolling & Virtual Lists

Efficiently handle large datasets and smooth scroll animations.

## Overview

ext-tui provides two complementary scrolling systems:

1. **Virtual List** - Render only visible items from large datasets
2. **Smooth Scrolling** - Spring-physics based scroll animations

## Virtual List

Virtual lists efficiently render large datasets by only creating nodes for visible items plus a small overscan buffer.

### Creating a Virtual List

```php
// Parameters: total items, item height (rows), viewport height (rows), overscan
$vlist = tui_virtual_create(10000, 1, 20, 5);

// 10,000 items total
// Each item is 1 row tall
// Viewport shows 20 rows
// Render 5 extra items above/below (for smooth scrolling)
```

### Getting Visible Range

```php
$range = tui_virtual_get_range($vlist);

// Only render items in this range
for ($i = $range['start']; $i < $range['end']; $i++) {
    $offset = tui_virtual_item_offset($vlist, $i);
    renderItem($items[$i], $offset);
}
```

The range array contains:

| Key | Description |
|-----|-------------|
| `start` | First visible item index (including overscan) |
| `end` | Last visible item index (exclusive) |
| `offset` | Current scroll offset in rows |
| `progress` | Scroll progress 0.0 to 1.0 |
| `item_count` | Total items |
| `viewport_height` | Viewport height |
| `max_scroll` | Maximum scroll offset |

### Scrolling

```php
// Scroll to show specific item at top
tui_virtual_scroll_to($vlist, 500);

// Scroll by rows (positive = down)
tui_virtual_scroll_by($vlist, 5);

// Scroll by items
tui_virtual_scroll_items($vlist, 1);   // Next item
tui_virtual_scroll_items($vlist, -1);  // Previous item

// Ensure item is visible (minimal scroll)
tui_virtual_ensure_visible($vlist, $selectedIndex);

// Page navigation
tui_virtual_page_up($vlist);
tui_virtual_page_down($vlist);

// Jump to ends
tui_virtual_scroll_top($vlist);
tui_virtual_scroll_bottom($vlist);
```

### Dynamic Updates

```php
// Update item count (e.g., after filtering)
tui_virtual_set_count($vlist, count($filteredItems));

// Update viewport height (e.g., after resize)
tui_virtual_set_viewport($vlist, $newHeight);
```

### Checking Visibility

```php
// Is item currently visible?
if (tui_virtual_is_visible($vlist, $index)) {
    // Item is in view
}

// Get item's Y offset relative to viewport
$yOffset = tui_virtual_item_offset($vlist, $index);
// May be negative (above viewport) or > viewport height (below)
```

### Cleanup

```php
tui_virtual_destroy($vlist);
```

### Complete Example

```php
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;
use Xocdr\Tui\Ext\Key;

// Generate large dataset
$items = [];
for ($i = 0; $i < 10000; $i++) {
    $items[] = "Item #$i - " . bin2hex(random_bytes(8));
}

$selectedIndex = 0;
$vlist = null;

$app = function () use ($items, &$selectedIndex, &$vlist) {
    [$width, $height] = tui_get_terminal_size();
    $viewportHeight = $height - 2;  // Leave room for header/footer

    // Create or update virtual list
    if ($vlist === null) {
        $vlist = tui_virtual_create(count($items), 1, $viewportHeight, 3);
    } else {
        tui_virtual_set_viewport($vlist, $viewportHeight);
    }

    // Ensure selection is visible
    tui_virtual_ensure_visible($vlist, $selectedIndex);

    // Get visible range
    $range = tui_virtual_get_range($vlist);

    // Build visible items
    $children = [];
    for ($i = $range['start']; $i < $range['end']; $i++) {
        $isSelected = ($i === $selectedIndex);
        $children[] = new Text([
            'content' => ($isSelected ? '> ' : '  ') . $items[$i],
            'color' => $isSelected ? [100, 255, 100] : [200, 200, 200],
            'bold' => $isSelected,
        ]);
    }

    // Progress indicator
    $pct = round($range['progress'] * 100);
    $children[] = new Text([
        'content' => "--- {$pct}% ({$selectedIndex}/" . count($items) . ") ---",
        'dim' => true,
    ]);

    return new Box([
        'flexDirection' => 'column',
        'children' => $children,
    ]);
};

$instance = tui_render($app);

tui_set_input_handler($instance, function (Key $key) use ($instance, $items, &$selectedIndex, &$vlist) {
    if ($key->escape) {
        tui_virtual_destroy($vlist);
        tui_unmount($instance);
        return;
    }

    if ($key->upArrow) {
        $selectedIndex = max(0, $selectedIndex - 1);
        tui_rerender($instance);
    }

    if ($key->downArrow) {
        $selectedIndex = min(count($items) - 1, $selectedIndex + 1);
        tui_rerender($instance);
    }

    if ($key->key === 'g' && !$key->shift) {
        // Go to top
        $selectedIndex = 0;
        tui_virtual_scroll_top($vlist);
        tui_rerender($instance);
    }

    if ($key->key === 'G' || ($key->key === 'g' && $key->shift)) {
        // Go to bottom
        $selectedIndex = count($items) - 1;
        tui_virtual_scroll_bottom($vlist);
        tui_rerender($instance);
    }
});

tui_wait_until_exit($instance);
```

---

## Smooth Scrolling

Smooth scrolling uses spring physics for natural-feeling scroll animations.

### Creating a Scroll Animation

```php
$scroll = tui_scroll_create();
```

### Setting Spring Physics

```php
// Configure spring behavior
tui_scroll_set_spring($scroll,
    200.0,  // stiffness (50-500, higher = faster)
    1.0     // damping ratio
);

// Damping ratio:
// < 1.0 = underdamped (oscillates/bounces)
// = 1.0 = critically damped (fastest without overshoot)
// > 1.0 = overdamped (slower, no overshoot)
```

### Animating to Position

```php
// Set target position (animation starts automatically)
tui_scroll_set_target($scroll, 0.0, 100.0);

// Or scroll relative to current target
tui_scroll_by($scroll, 0.0, 10.0);  // Add 10 to Y target
```

### Animation Loop

```php
// In your render loop
while (tui_scroll_is_animating($scroll)) {
    // Update physics (dt = time delta in seconds)
    tui_scroll_update($scroll, 1.0 / 60.0);  // 60 FPS

    // Get current position
    $pos = tui_scroll_get_position($scroll);

    // Apply to your rendering
    $scrollY = (int) round($pos['y']);
    render($scrollY);

    usleep(16666);  // ~60 FPS
}
```

### Immediate Jump

```php
// Skip animation, jump directly to target
tui_scroll_snap($scroll);
```

### Animation State

```php
// Check if animating
if (tui_scroll_is_animating($scroll)) {
    // Still moving
}

// Get animation progress (0.0 to 1.0)
$progress = tui_scroll_progress($scroll);
```

### Cleanup

```php
tui_scroll_destroy($scroll);
```

### Complete Example

```php
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;
use Xocdr\Tui\Ext\Key;

// Items and state
$items = range(1, 100);
$scroll = tui_scroll_create();
$targetY = 0.0;

// Configure spring for smooth feel
tui_scroll_set_spring($scroll, 180.0, 1.0);

$app = function () use ($items, $scroll) {
    $pos = tui_scroll_get_position($scroll);
    $scrollOffset = (int) round($pos['y']);

    [$width, $height] = tui_get_terminal_size();

    $children = [];
    for ($i = 0; $i < $height - 1; $i++) {
        $itemIndex = $i + $scrollOffset;
        if ($itemIndex >= 0 && $itemIndex < count($items)) {
            $children[] = new Text([
                'content' => "Line {$items[$itemIndex]}",
            ]);
        }
    }

    return new Box([
        'flexDirection' => 'column',
        'children' => $children,
    ]);
};

$instance = tui_render($app);

// Animation timer
$timerId = tui_add_timer($instance, function () use ($instance, $scroll) {
    if (tui_scroll_is_animating($scroll)) {
        tui_scroll_update($scroll, 1.0 / 60.0);
        tui_rerender($instance);
    }
}, 16);  // ~60 FPS

tui_set_input_handler($instance, function (Key $key) use ($instance, $scroll, $items, &$targetY) {
    if ($key->escape) {
        tui_scroll_destroy($scroll);
        tui_unmount($instance);
        return;
    }

    [$width, $height] = tui_get_terminal_size();
    $maxScroll = max(0, count($items) - $height + 1);

    if ($key->upArrow) {
        $targetY = max(0, $targetY - 1);
        tui_scroll_set_target($scroll, 0.0, $targetY);
    }

    if ($key->downArrow) {
        $targetY = min($maxScroll, $targetY + 1);
        tui_scroll_set_target($scroll, 0.0, $targetY);
    }

    if ($key->key === ' ') {
        // Page down
        $targetY = min($maxScroll, $targetY + $height - 1);
        tui_scroll_set_target($scroll, 0.0, $targetY);
    }
});

tui_wait_until_exit($instance);
```

---

## Combining Virtual List + Smooth Scrolling

For the best experience with large lists, combine both systems:

```php
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;
use Xocdr\Tui\Ext\Key;

$items = range(1, 50000);
$vlist = null;
$scroll = tui_scroll_create();
tui_scroll_set_spring($scroll, 200.0, 1.0);

$app = function () use ($items, &$vlist, $scroll) {
    [$width, $height] = tui_get_terminal_size();

    if ($vlist === null) {
        $vlist = tui_virtual_create(count($items), 1, $height, 5);
    }

    // Apply smooth scroll position to virtual list
    $pos = tui_scroll_get_position($scroll);
    $scrollRow = (int) round($pos['y']);

    // Sync virtual list scroll position
    tui_virtual_scroll_to($vlist, (int)($scrollRow));

    $range = tui_virtual_get_range($vlist);

    $children = [];
    for ($i = $range['start']; $i < $range['end']; $i++) {
        $children[] = new Text([
            'content' => "Item $i: " . str_repeat("─", 40),
        ]);
    }

    return new Box([
        'flexDirection' => 'column',
        'children' => $children,
    ]);
};

$instance = tui_render($app);

// Animation loop
$timerId = tui_add_timer($instance, function () use ($instance, $scroll) {
    if (tui_scroll_is_animating($scroll)) {
        tui_scroll_update($scroll, 1.0 / 60.0);
        tui_rerender($instance);
    }
}, 16);

tui_set_input_handler($instance, function (Key $key) use ($instance, $scroll, $items, &$vlist) {
    if ($key->escape) {
        tui_virtual_destroy($vlist);
        tui_scroll_destroy($scroll);
        tui_unmount($instance);
        return;
    }

    // Scroll by one item with animation
    if ($key->downArrow) {
        tui_scroll_by($scroll, 0.0, 1.0);
    }
    if ($key->upArrow) {
        tui_scroll_by($scroll, 0.0, -1.0);
    }

    // Page scroll
    [$width, $height] = tui_get_terminal_size();
    if ($key->key === ' ') {
        tui_scroll_by($scroll, 0.0, (float)($height - 1));
    }
});

tui_wait_until_exit($instance);
```

---

## Performance Tips

1. **Use overscan** - Always use overscan (3-5 items) to prevent visible loading
2. **Fixed item height** - Virtual lists require fixed item heights for O(1) calculations
3. **Batch updates** - When filtering, call `tui_virtual_set_count()` once, not per item
4. **Reuse resources** - Don't recreate the virtual list on each render
5. **Appropriate spring** - Higher stiffness = snappier but more CPU during animation

## Memory Usage

| Items | Without Virtual List | With Virtual List |
|-------|---------------------|-------------------|
| 100 | ~100 nodes | ~30 nodes |
| 1,000 | ~1,000 nodes | ~30 nodes |
| 10,000 | ~10,000 nodes | ~30 nodes |
| 100,000 | Crash/OOM | ~30 nodes |

Virtual lists keep memory constant regardless of dataset size.

## See Also

- [Animation](animation.md) - Easing functions
- [Components](components.md) - Box and Text
- [Reference: Functions](../reference/functions.md) - Complete API
