# Testing Framework Reference

This document provides a complete reference for the ext-tui testing framework functions and constants.

## Functions

### tui_test_create

Creates a new headless test renderer.

```php
tui_test_create(int $width, int $height): resource
```

**Parameters:**
- `$width` - Buffer width in characters (1-1000)
- `$height` - Buffer height in characters (1-1000)

**Returns:** A test renderer resource, or `null` on error.

**Example:**
```php
$renderer = tui_test_create(80, 24);
```

---

### tui_test_destroy

Destroys a test renderer and frees all resources.

```php
tui_test_destroy(resource $renderer): void
```

**Parameters:**
- `$renderer` - The test renderer resource

**Example:**
```php
tui_test_destroy($renderer);
```

---

### tui_test_render

Renders a component tree to the test buffer.

```php
tui_test_render(resource $renderer, object $component): void
```

**Parameters:**
- `$renderer` - The test renderer resource
- `$component` - A Box or Text component to render

**Example:**
```php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

$box = new ContainerNode(['width' => 80, 'height' => 24]);
$box->addChild(new ContentNode(['content' => 'Hello World']));
tui_test_render($renderer, $box);
```

---

### tui_test_get_output

Gets the rendered output as an array of strings (one per line).

```php
tui_test_get_output(resource $renderer): array
```

**Parameters:**
- `$renderer` - The test renderer resource

**Returns:** Array of strings, one for each line. Trailing spaces are trimmed.

**Example:**
```php
$lines = tui_test_get_output($renderer);
foreach ($lines as $i => $line) {
    echo "Line $i: $line\n";
}
```

---

### tui_test_to_string

Gets the rendered output as a single string with newlines.

```php
tui_test_to_string(resource $renderer): string
```

**Parameters:**
- `$renderer` - The test renderer resource

**Returns:** String with all lines joined by newlines. Trailing empty lines are trimmed.

**Example:**
```php
$output = tui_test_to_string($renderer);
echo $output;
```

---

### tui_test_send_input

Queues text input to be processed on the next frame.

```php
tui_test_send_input(resource $renderer, string $text): void
```

**Parameters:**
- `$renderer` - The test renderer resource
- `$text` - Text string to queue

**Example:**
```php
tui_test_send_input($renderer, "Hello World");
tui_test_advance_frame($renderer);  // Process the input
```

---

### tui_test_send_key

Queues a special key to be processed on the next frame.

```php
tui_test_send_key(resource $renderer, int $key_code): void
```

**Parameters:**
- `$renderer` - The test renderer resource
- `$key_code` - Key code constant (see Key Constants below)

**Example:**
```php
tui_test_send_key($renderer, TUI_KEY_ENTER);
tui_test_advance_frame($renderer);
```

---

### tui_test_advance_frame

Processes queued input and re-renders.

```php
tui_test_advance_frame(resource $renderer): void
```

**Parameters:**
- `$renderer` - The test renderer resource

**Example:**
```php
tui_test_send_input($renderer, "test");
tui_test_send_key($renderer, TUI_KEY_ENTER);
tui_test_advance_frame($renderer);  // Process all queued input
```

---

### tui_test_run_timers

Advances simulated time and runs due timers.

```php
tui_test_run_timers(resource $renderer, int $ms): void
```

**Parameters:**
- `$renderer` - The test renderer resource
- `$ms` - Milliseconds to advance (must be > 0)

**Example:**
```php
// Simulate 1 second passing
tui_test_run_timers($renderer, 1000);

// Check if timer-based updates occurred
$output = tui_test_to_string($renderer);
```

---

### tui_test_get_by_id

Finds a node by its ID.

```php
tui_test_get_by_id(resource $renderer, string $id): ?array
```

**Parameters:**
- `$renderer` - The test renderer resource
- `$id` - The node ID to search for

**Returns:** Associative array with node info, or `null` if not found.

**Node Info Structure:**
```php
[
    'id' => string|null,      // Node ID
    'type' => string,         // 'box', 'text', 'static', 'newline', 'spacer'
    'text' => string|null,    // Text content (for text nodes)
    'x' => int,               // Computed X position
    'y' => int,               // Computed Y position
    'width' => int,           // Computed width
    'height' => int,          // Computed height
    'focusable' => bool,      // Whether node can receive focus
    'focused' => bool,        // Whether node currently has focus
]
```

**Example:**
```php
$box = new ContainerNode(['id' => 'my-button', 'focusable' => true]);
tui_test_render($renderer, $box);

$node = tui_test_get_by_id($renderer, 'my-button');
if ($node) {
    echo "Found button at ({$node['x']}, {$node['y']})\n";
    echo "Size: {$node['width']}x{$node['height']}\n";
    echo "Focused: " . ($node['focused'] ? 'yes' : 'no') . "\n";
}
```

---

### tui_test_get_by_text

Finds all nodes containing the specified text.

```php
tui_test_get_by_text(resource $renderer, string $text): array
```

**Parameters:**
- `$renderer` - The test renderer resource
- `$text` - Text substring to search for

**Returns:** Array of node info arrays (same structure as `tui_test_get_by_id`).

**Example:**
```php
$box = new ContainerNode([]);
$box->addChild(new ContentNode(['content' => 'Hello']));
$box->addChild(new ContentNode(['content' => 'Hello World']));
$box->addChild(new ContentNode(['content' => 'Goodbye']));
tui_test_render($renderer, $box);

$nodes = tui_test_get_by_text($renderer, "Hello");
echo "Found " . count($nodes) . " nodes containing 'Hello'\n";
// Output: Found 2 nodes containing 'Hello'
```

---

## Key Constants

Key codes for `tui_test_send_key()`. These start at 100 to avoid conflicts with Ctrl+key combinations (1-26).

| Constant | Value | Description |
|----------|-------|-------------|
| `TUI_KEY_ENTER` | 100 | Enter/Return key |
| `TUI_KEY_TAB` | 101 | Tab key |
| `TUI_KEY_ESCAPE` | 102 | Escape key |
| `TUI_KEY_BACKSPACE` | 103 | Backspace key |
| `TUI_KEY_UP` | 104 | Up arrow |
| `TUI_KEY_DOWN` | 105 | Down arrow |
| `TUI_KEY_RIGHT` | 106 | Right arrow |
| `TUI_KEY_LEFT` | 107 | Left arrow |
| `TUI_KEY_HOME` | 108 | Home key |
| `TUI_KEY_END` | 109 | End key |
| `TUI_KEY_PAGE_UP` | 110 | Page Up |
| `TUI_KEY_PAGE_DOWN` | 111 | Page Down |
| `TUI_KEY_DELETE` | 112 | Delete key |
| `TUI_KEY_INSERT` | 113 | Insert key |
| `TUI_KEY_F1` | 114 | F1 function key |
| `TUI_KEY_F2` | 115 | F2 function key |
| `TUI_KEY_F3` | 116 | F3 function key |
| `TUI_KEY_F4` | 117 | F4 function key |
| `TUI_KEY_F5` | 118 | F5 function key |
| `TUI_KEY_F6` | 119 | F6 function key |
| `TUI_KEY_F7` | 120 | F7 function key |
| `TUI_KEY_F8` | 121 | F8 function key |
| `TUI_KEY_F9` | 122 | F9 function key |
| `TUI_KEY_F10` | 123 | F10 function key |
| `TUI_KEY_F11` | 124 | F11 function key |
| `TUI_KEY_F12` | 125 | F12 function key |

### Ctrl+Key Combinations

For Ctrl+key combinations, use values 1-26 directly:

```php
// Ctrl+C = 3
tui_test_send_key($renderer, 3);

// Ctrl+A = 1
tui_test_send_key($renderer, 1);

// Ctrl+Z = 26
tui_test_send_key($renderer, 26);
```

---

## Resource Management

Test renderer resources are automatically cleaned up when the PHP variable goes out of scope. However, for explicit cleanup (especially in loops), call `tui_test_destroy()`:

```php
for ($i = 0; $i < 100; $i++) {
    $renderer = tui_test_create(80, 24);
    // ... run test ...
    tui_test_destroy($renderer);  // Explicit cleanup
}
```

---

## Limitations

- Maximum buffer size: 1000x1000 characters
- Maximum tree depth for queries: 256 levels
- Input queue grows dynamically but has overflow protection
- Timer simulation requires an app instance to be set (internal)

---

## See Also

- [Testing Manual](../manual/testing.md) - Step-by-step testing guide
- [Functions Reference](functions.md) - All ext-tui functions
- [Classes Reference](classes.md) - Component classes
