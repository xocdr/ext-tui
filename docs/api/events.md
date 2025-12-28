# API Reference: Event Functions

[Back to Documentation](../README.md) | [Feature Guide](../features/events.md)

---

## tui_set_input_handler

Register a callback for keyboard input.

```php
void tui_set_input_handler(callable $callback)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$callback` | callable | Function to call on key press |

### Callback Signature

```php
function(string $key): void
```

### Key Values

| Category | Examples |
|----------|----------|
| Printable | `"a"`, `"A"`, `"1"`, `"@"`, `" "` |
| Special | `"Enter"`, `"Tab"`, `"Escape"`, `"Backspace"`, `"Delete"` |
| Navigation | `"Up"`, `"Down"`, `"Left"`, `"Right"`, `"Home"`, `"End"`, `"PageUp"`, `"PageDown"` |
| Function | `"F1"` through `"F12"` |
| Modified | `"Ctrl+c"`, `"Ctrl+x"`, `"Alt+a"`, `"Shift+Tab"` |

### Example

```php
tui_set_input_handler(function(string $key) {
    global $running, $cursor_y;

    switch ($key) {
        case 'q':
        case 'Escape':
            tui_unmount();
            break;
        case 'Up':
            $cursor_y = max(0, $cursor_y - 1);
            break;
        case 'Down':
            $cursor_y++;
            break;
        case 'Enter':
            select_current_item();
            break;
    }

    redraw();
});
```

### Notes

- Only one input handler can be registered at a time
- Called for each key press while event loop is running
- Handler runs in main thread (blocking)

### Related

- [tui_wait_until_exit()](#tui_wait_until_exit) - Start event loop
- [tui_unmount()](#tui_unmount) - Stop event loop

---

## tui_set_resize_handler

Register a callback for terminal resize events.

```php
void tui_set_resize_handler(callable $callback)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$callback` | callable | Function to call on resize |

### Callback Signature

```php
function(int $width, int $height): void
```

### Example

```php
$buf = null;

tui_set_resize_handler(function(int $width, int $height) {
    global $buf;

    // Recreate buffer with new dimensions
    $buf = tui_buffer_create($width, $height);

    // Redraw everything
    redraw();
});
```

### Notes

- Uses SIGWINCH signal on Unix
- Only one resize handler can be registered
- Handler may be called multiple times during resize drag

### Related

- [tui_get_terminal_size()](terminal.md#tui_get_terminal_size) - Get current size
- [tui_get_size()](#tui_get_size) - Get render area size

---

## tui_set_focus_handler

Register a callback for focus change events.

```php
void tui_set_focus_handler(callable $callback)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$callback` | callable | Function to call on focus change |

### Callback Signature

```php
function(mixed $focused_node): void
```

---

## tui_focus_next

Move focus to the next focusable element.

```php
void tui_focus_next()
```

---

## tui_focus_prev

Move focus to the previous focusable element.

```php
void tui_focus_prev()
```

---

## tui_get_focused_node

Get the currently focused node.

```php
mixed tui_get_focused_node()
```

### Return Value

Returns the currently focused node, or null if nothing is focused.

---

## tui_render

Render a component tree to the terminal.

```php
void tui_render(mixed $component)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$component` | mixed | Root component to render |

### Notes

- Used with the component-based rendering system
- For buffer-based rendering, use `tui_buffer_render()`

---

## tui_rerender

Re-render the current component tree.

```php
void tui_rerender()
```

### Notes

- Call after state changes to update the display
- Triggers a diff and minimal update

---

## tui_wait_until_exit

Start the event loop and wait until exit.

```php
void tui_wait_until_exit()
```

### Parameters

None.

### Return Value

None (blocks until `tui_unmount()` is called).

### Example

```php
// Set up handlers
tui_set_input_handler($key_handler);
tui_set_resize_handler($resize_handler);

// Initial render
tui_render($root);

// Start loop (blocks here)
tui_wait_until_exit();

// This runs after tui_unmount()
echo "Exited\n";
```

### Notes

- Enters terminal raw mode
- Hides cursor
- Blocks until `tui_unmount()` is called
- Restores terminal state on exit

---

## tui_unmount

Stop the event loop and clean up.

```php
void tui_unmount()
```

### Parameters

None.

### Return Value

None.

### Example

```php
tui_set_input_handler(function(string $key) {
    if ($key === 'q' || $key === 'Escape') {
        tui_unmount();  // Exit the event loop
    }
});
```

### Notes

- Safe to call from event handlers
- Event loop exits after current handler completes
- Terminal is restored to normal mode

---

## tui_get_size

Get the current render area size.

```php
array tui_get_size()
```

### Return Value

Returns an array with `[width, height]` of the render area.

### Example

```php
[$width, $height] = tui_get_size();
```

---

## Complete Example

```php
<?php
if (!tui_is_interactive()) {
    die("Interactive terminal required\n");
}

// Initialize
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);
$selected = 0;
$items = ['New File', 'Open File', 'Save', 'Settings', 'Exit'];

function draw(): void
{
    global $buf, $width, $height, $selected, $items;

    tui_buffer_clear($buf);

    // Title
    $title_style = ['fg' => [255, 200, 0], 'bold' => true];
    tui_buffer_write($buf, 2, 1, "Menu Example", $title_style);
    tui_draw_line($buf, 2, 2, $width - 3, 2, '─', []);

    // Menu items
    foreach ($items as $i => $item) {
        $y = 4 + $i;
        $is_selected = ($i === $selected);

        $style = $is_selected
            ? ['fg' => [0, 0, 0], 'bg' => [100, 150, 255]]
            : ['fg' => [200, 200, 200]];

        $prefix = $is_selected ? ' > ' : '   ';
        $text = $prefix . tui_pad($item, 20);
        tui_buffer_write($buf, 2, $y, $text, $style);
    }

    // Help
    $help = "↑↓: Navigate | Enter: Select | q: Quit";
    tui_buffer_write($buf, 2, $height - 2, $help, ['dim' => true]);

    tui_buffer_render($buf);
}

// Key handler
tui_set_input_handler(function(string $key) {
    global $selected, $items;

    switch ($key) {
        case 'Up':
            $selected = max(0, $selected - 1);
            break;
        case 'Down':
            $selected = min(count($items) - 1, $selected + 1);
            break;
        case 'Enter':
            if ($items[$selected] === 'Exit') {
                tui_unmount();
                return;
            }
            // Handle other selections...
            break;
        case 'q':
        case 'Escape':
            tui_unmount();
            return;
    }

    draw();
});

// Resize handler
tui_set_resize_handler(function(int $w, int $h) {
    global $buf, $width, $height;

    $width = $w;
    $height = $h;
    $buf = tui_buffer_create($w, $h);
    draw();
});

// Initial draw and run
draw();
tui_wait_until_exit();

echo "Selected: " . $items[$selected] . "\n";
```

---

[Back to Documentation](../README.md) | [Feature Guide](../features/events.md)
