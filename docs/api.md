# ext-tui API Reference

## Overview

ext-tui is a PHP C extension that provides terminal UI capabilities with a React-like component model and Yoga flexbox layout.

## Functions

### tui_render()

```php
function tui_render(callable $component, array $options = []): TuiInstance
```

Renders a TUI component tree to the terminal.

**Parameters:**
- `$component` - A callable that returns a `TuiBox` or `TuiText` component
- `$options` - Optional configuration array:
  - `fullscreen` (bool): Use alternate screen buffer (default: `true`)
  - `exitOnCtrlC` (bool): Auto-exit on Ctrl+C (default: `true`)

**Returns:** `TuiInstance` object for controlling the running TUI

**Example:**
```php
$instance = tui_render(function() {
    return new TuiBox(['padding' => 1]);
}, ['fullscreen' => false]);
```

### tui_rerender()

```php
function tui_rerender(TuiInstance $instance): void
```

Forces a re-render of the component tree. Call this after state changes.

### tui_unmount()

```php
function tui_unmount(TuiInstance $instance): void
```

Stops the TUI, restores terminal state, and cleans up resources.

### tui_wait_until_exit()

```php
function tui_wait_until_exit(TuiInstance $instance): void
```

Blocks execution until the TUI exits (via Ctrl+C or `exit()` method).

### tui_set_input_handler()

```php
function tui_set_input_handler(TuiInstance $instance, callable $handler): void
```

Registers a callback for keyboard input events.

**Parameters:**
- `$instance` - The TuiInstance to attach the handler to
- `$handler` - Callback receiving a `TuiKey` object: `function(TuiKey $key): void`

**Example:**
```php
tui_set_input_handler($instance, function(TuiKey $key) {
    if ($key->escape) {
        // Handle escape
    }
    if ($key->key === 'q') {
        // Handle 'q' key
    }
});
```

### tui_set_focus_handler()

```php
function tui_set_focus_handler(TuiInstance $instance, callable $handler): void
```

Registers a callback for focus change events.

**Parameters:**
- `$instance` - The TuiInstance to attach the handler to
- `$handler` - Callback: `function(TuiFocusEvent $event): void`

**Example:**
```php
tui_set_focus_handler($instance, function(TuiFocusEvent $event) {
    // $event->previous and $event->current contain node info
    // $event->direction is 'next', 'prev', or 'programmatic'
    if ($event->current) {
        echo "Focus moved to new element ({$event->direction})\n";
    }
});
```

### tui_focus_next()

```php
function tui_focus_next(TuiInstance $instance): void
```

Moves focus to the next focusable element in the tree.

### tui_focus_prev()

```php
function tui_focus_prev(TuiInstance $instance): void
```

Moves focus to the previous focusable element in the tree.

### tui_set_resize_handler()

```php
function tui_set_resize_handler(TuiInstance $instance, callable $handler): void
```

Registers a callback for terminal resize events.

**Parameters:**
- `$instance` - The TuiInstance to attach the handler to
- `$handler` - Callback: `function(int $width, int $height): void`

**Example:**
```php
tui_set_resize_handler($instance, function($width, $height) {
    echo "Terminal resized to {$width}x{$height}\n";
});
```

### tui_get_terminal_size()

```php
function tui_get_terminal_size(): array
```

Returns the current terminal dimensions.

**Returns:** `[int $width, int $height]`

### tui_is_interactive()

```php
function tui_is_interactive(): bool
```

Returns `true` if stdin and stdout are connected to a TTY.

### tui_is_ci()

```php
function tui_is_ci(): bool
```

Returns `true` if running in a known CI environment (GitHub Actions, GitLab CI, etc.).

### tui_string_width()

```php
function tui_string_width(string $text): int
```

Returns the display width of text, correctly handling:
- Multi-byte UTF-8 characters
- Wide characters (CJK, emoji)
- Zero-width characters (combining marks)

### tui_wrap_text()

```php
function tui_wrap_text(string $text, int $width): array
```

Wraps text to fit within the specified width.

**Returns:** Array of lines

### tui_truncate()

```php
function tui_truncate(string $text, int $width, string $ellipsis = '...'): string
```

Truncates text to fit within width, adding ellipsis if truncated.

### tui_get_size()

```php
function tui_get_size(TuiInstance $instance): ?array
```

Returns the current render size for the TUI instance.

**Returns:** Array with `width`, `height`, `columns`, `rows` keys, or `null` if invalid.

**Example:**
```php
$size = tui_get_size($instance);
echo "Current size: {$size['width']}x{$size['height']}\n";
```

### tui_get_focused_node()

```php
function tui_get_focused_node(TuiInstance $instance): ?array
```

Returns information about the currently focused node.

**Returns:** Array with node info, or `null` if no node is focused.

**Example:**
```php
$node = tui_get_focused_node($instance);
if ($node) {
    echo "Focused: {$node['type']} at ({$node['x']}, {$node['y']})\n";
}
```

## Classes

### TuiBox

Container component with flexbox layout support.

#### Constructor

```php
new TuiBox(array $props = [])
```

#### Properties

All properties can be set via constructor or direct assignment:

```php
$box = new TuiBox(['padding' => 1]);
$box->flexDirection = 'row';
```

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `flexDirection` | string | `'column'` | Child layout direction |
| `alignItems` | string\|null | null | Cross-axis alignment |
| `justifyContent` | string\|null | null | Main-axis alignment |
| `flexGrow` | int | 0 | Grow factor |
| `flexShrink` | int | 1 | Shrink factor |
| `width` | int\|string\|null | null | Width (cells or percent) |
| `height` | int\|string\|null | null | Height (cells or percent) |
| `padding` | int | 0 | All-side padding |
| `paddingTop/Bottom/Left/Right` | int | 0 | Individual padding |
| `paddingX/Y` | int | 0 | Horizontal/vertical padding |
| `margin` | int | 0 | All-side margin |
| `marginTop/Bottom/Left/Right` | int | 0 | Individual margin |
| `marginX/Y` | int | 0 | Horizontal/vertical margin |
| `gap` | int | 0 | Gap between children |
| `borderStyle` | string\|null | null | Border style |
| `borderColor` | string\|null | null | Border color |
| `focusable` | bool | false | Whether this element can receive focus |
| `focused` | bool | false | Whether this element is currently focused |
| `children` | array | [] | Child components |

#### Methods

```php
addChild(TuiBox|TuiText $child): self
```

Adds a child component and returns `$this` for chaining.

### TuiText

Text display component with rich styling.

#### Constructor

```php
new TuiText(string $content = '', array $props = [])
```

#### Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `content` | string | `''` | Text content |
| `color` | string\|array\|null | null | Foreground color |
| `backgroundColor` | string\|array\|null | null | Background color |
| `bold` | bool | false | Bold style |
| `dim` | bool | false | Dim/faint style |
| `italic` | bool | false | Italic style |
| `underline` | bool | false | Underline style |
| `inverse` | bool | false | Inverse colors |
| `strikethrough` | bool | false | Strikethrough |
| `wrap` | string\|null | null | Wrap mode |

**Color formats:**
- Hex string: `'#ff0000'`
- RGB array: `[255, 0, 0]`

### TuiInstance

Represents a running TUI application.

#### Methods

```php
rerender(): void
unmount(): void
waitUntilExit(): void
exit(int $code = 0): void
```

### TuiKey

Keyboard event object. Not instantiated directly - received via input handlers.

#### Properties

| Property | Type | Description |
|----------|------|-------------|
| `key` | string | The character pressed |
| `upArrow` | bool | Up arrow key |
| `downArrow` | bool | Down arrow key |
| `leftArrow` | bool | Left arrow key |
| `rightArrow` | bool | Right arrow key |
| `return` | bool | Enter/Return key |
| `escape` | bool | Escape key |
| `backspace` | bool | Backspace key |
| `delete` | bool | Delete key |
| `tab` | bool | Tab key |
| `ctrl` | bool | Ctrl modifier |
| `meta` | bool | Alt/Meta modifier |
| `shift` | bool | Shift modifier |

### TuiFocusEvent

Focus change event object. Received via focus handlers.

#### Properties

| Property | Type | Description |
|----------|------|-------------|
| `previous` | ?array | Info about previously focused node |
| `current` | ?array | Info about newly focused node |
| `direction` | string | Focus direction: 'next', 'prev', or 'programmatic' |

Each node array contains:
- `focusable` (bool): Whether the node is focusable
- `focused` (bool): Whether the node is currently focused
- `x` (int): X position
- `y` (int): Y position
- `width` (int): Node width
- `height` (int): Node height
- `type` (string): 'box' or 'text'
- `content` (string): Text content (for text nodes only)
