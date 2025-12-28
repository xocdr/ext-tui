# Buffers & Rendering

[← Text Utilities](text.md) | [Back to Documentation](../README.md) | [Next: Drawing →](drawing.md)

---

Buffers are the foundation of ext-tui rendering. All drawing operations happen on buffers, which are then rendered to the terminal in one efficient operation.

## Overview

The buffer system provides:
- **Double buffering**: Draw to an off-screen buffer, then display instantly
- **Flicker-free updates**: Only changed cells are updated
- **Cell-based storage**: Each cell stores a character, colors, and attributes
- **Efficient rendering**: Minimizes terminal escape sequences

## Functions

### tui_buffer_create()

Create a new screen buffer.

```php
resource tui_buffer_create(int $width, int $height)
```

**Parameters**:
- `$width`: Buffer width in columns
- `$height`: Buffer height in rows

**Returns**: Buffer resource

**Example**:
```php
// Create a full-screen buffer
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

// Create a smaller buffer for a component
$component_buf = tui_buffer_create(40, 10);
```

---

### tui_buffer_free()

Free a buffer and release its memory.

```php
void tui_buffer_free(resource $buffer)
```

**Parameters**:
- `$buffer`: Buffer resource to free

**Example**:
```php
$buf = tui_buffer_create(80, 24);
// ... use buffer ...
tui_buffer_free($buf);
$buf = null;  // Good practice: clear the variable
```

**Note**: Always free buffers when done. PHP will clean up on script exit, but explicit freeing is better for long-running applications.

---

### tui_buffer_clear()

Clear a buffer, resetting all cells to empty.

```php
void tui_buffer_clear(resource $buffer)
```

**Parameters**:
- `$buffer`: Buffer resource to clear

**Example**:
```php
// Clear before each frame
while ($running) {
    tui_buffer_clear($buf);
    draw_frame($buf);
    tui_render($buf);
    usleep(16666);  // ~60 FPS
}
```

---

### tui_buffer_write()

Write text to a buffer at a specific position.

```php
void tui_buffer_write(resource $buffer, int $x, int $y, string $text, array $style)
```

**Parameters**:
- `$buffer`: Target buffer
- `$x`: Column position (0-based)
- `$y`: Row position (0-based)
- `$text`: Text to write
- `$style`: Style array (see below)

**Example**:
```php
$style = [
    'fg' => [255, 255, 255],  // White text
    'bg' => [0, 0, 128],      // Blue background
    'bold' => true
];

tui_buffer_write($buf, 5, 2, "Hello, World!", $style);
```

---

### tui_buffer_set_cell()

Set a single cell in the buffer.

```php
void tui_buffer_set_cell(resource $buffer, int $x, int $y, string $char, array $style)
```

**Parameters**:
- `$buffer`: Target buffer
- `$x`: Column position
- `$y`: Row position
- `$char`: Single character (or multi-byte Unicode character)
- `$style`: Style array

**Example**:
```php
// Draw a border corner
tui_buffer_set_cell($buf, 0, 0, "┌", $border_style);
tui_buffer_set_cell($buf, 10, 0, "┐", $border_style);

// Draw a pattern
for ($x = 0; $x < 10; $x++) {
    for ($y = 0; $y < 10; $y++) {
        $char = ($x + $y) % 2 ? "█" : "░";
        tui_buffer_set_cell($buf, $x, $y, $char, $style);
    }
}
```

---

### tui_render()

Render a buffer to the terminal.

```php
void tui_render(resource $buffer)
```

**Parameters**:
- `$buffer`: Buffer to render

**Example**:
```php
// Draw everything to buffer first
tui_buffer_write($buf, 0, 0, "Title", $title_style);
tui_draw_box($buf, 0, 1, 40, 10, $box_style);
tui_buffer_write($buf, 2, 3, "Content here", $text_style);

// Single render call - efficient!
tui_render($buf);
```

---

## Style Reference

Styles are associative arrays with these optional keys:

```php
$style = [
    // Colors (RGB arrays)
    'fg' => [255, 255, 255],   // Foreground (text) color
    'bg' => [0, 0, 0],         // Background color

    // Text attributes (booleans)
    'bold' => true,            // Bold/bright text
    'dim' => true,             // Dimmed text
    'italic' => true,          // Italic (if supported)
    'underline' => true,       // Underlined text
    'blink' => true,           // Blinking (if supported)
    'reverse' => true,         // Swap fg/bg colors
    'hidden' => true,          // Hidden text
    'strikethrough' => true,   // Strikethrough (if supported)
];
```

### Color Examples

```php
// Named-style colors
$red = ['fg' => [255, 0, 0]];
$green = ['fg' => [0, 255, 0]];
$blue = ['fg' => [0, 0, 255]];

// Muted/pastel colors
$muted_blue = ['fg' => [100, 150, 200]];
$warm_gray = ['fg' => [180, 170, 160]];

// High contrast
$warning = ['fg' => [255, 200, 0], 'bg' => [100, 0, 0], 'bold' => true];

// Terminal-friendly (works in more terminals)
$safe_green = ['fg' => [0, 200, 0]];  // Slightly darker than pure green
```

---

## Practical Patterns

### Double Buffering Loop

```php
<?php
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

$frame = 0;
while (true) {
    // Clear previous frame
    tui_buffer_clear($buf);

    // Draw current frame
    draw_scene($buf, $frame);

    // Display
    tui_render($buf);

    // Timing
    $frame++;
    usleep(33333);  // ~30 FPS
}

tui_buffer_free($buf);
```

### Component-Based Drawing

```php
class StatusBar
{
    private int $width;
    private array $style;

    public function __construct(int $width, array $style)
    {
        $this->width = $width;
        $this->style = $style;
    }

    public function draw($buf, int $y, string $left, string $right): void
    {
        // Fill background
        $bg = str_repeat(" ", $this->width);
        tui_buffer_write($buf, 0, $y, $bg, $this->style);

        // Left-aligned text
        tui_buffer_write($buf, 1, $y, $left, $this->style);

        // Right-aligned text
        $right_x = $this->width - tui_string_width($right) - 1;
        tui_buffer_write($buf, $right_x, $y, $right, $this->style);
    }
}

// Usage
$statusbar = new StatusBar($width, ['fg' => [0, 0, 0], 'bg' => [200, 200, 200]]);
$statusbar->draw($buf, $height - 1, "Ready", "Ln 1, Col 1");
```

### Layered Rendering

```php
// Background layer
tui_fill_rect($buf, 0, 0, $width, $height, '░', $bg_style);

// Middle layer - main content
tui_draw_box($buf, 5, 2, 40, 15, $box_style);
tui_buffer_write($buf, 7, 4, "Content goes here", $text_style);

// Top layer - dialog overlay
if ($show_dialog) {
    $dialog_x = ($width - 30) / 2;
    $dialog_y = ($height - 8) / 2;

    // Dialog shadow
    tui_fill_rect($buf, $dialog_x + 1, $dialog_y + 1, 30, 8, '█', $shadow_style);

    // Dialog box
    tui_fill_rect($buf, $dialog_x, $dialog_y, 30, 8, ' ', $dialog_style);
    tui_draw_box($buf, $dialog_x, $dialog_y, 30, 8, $dialog_style);
    tui_buffer_write($buf, $dialog_x + 2, $dialog_y + 3, "Are you sure?", $dialog_style);
}
```

### Viewport/Scrolling

```php
class ScrollableList
{
    private array $items;
    private int $scroll = 0;
    private int $selected = 0;

    public function draw($buf, int $x, int $y, int $width, int $height): void
    {
        // Ensure selected item is visible
        if ($this->selected < $this->scroll) {
            $this->scroll = $this->selected;
        } elseif ($this->selected >= $this->scroll + $height) {
            $this->scroll = $this->selected - $height + 1;
        }

        // Draw visible items
        for ($i = 0; $i < $height; $i++) {
            $item_idx = $this->scroll + $i;
            if ($item_idx >= count($this->items)) break;

            $style = $item_idx === $this->selected
                ? ['fg' => [0, 0, 0], 'bg' => [100, 150, 255]]
                : ['fg' => [200, 200, 200]];

            $text = tui_truncate($this->items[$item_idx], $width);
            $text = tui_pad($text, $width);
            tui_buffer_write($buf, $x, $y + $i, $text, $style);
        }
    }
}
```

## Performance Tips

1. **Minimize buffer operations**: Draw to the buffer, then render once
2. **Reuse buffers**: Create once, clear and reuse
3. **Batch text writes**: Combine adjacent text into single writes when possible
4. **Use appropriate buffer size**: Don't create buffers larger than needed

## Related Functions

- [tui_draw_box()](../api/drawing.md#tui_draw_box) - Draw boxes on buffers
- [tui_fill_rect()](../api/drawing.md#tui_fill_rect) - Fill rectangular areas
- [tui_render()](../api/buffers.md#tui_render) - Display buffer to terminal

## See Also

- [Drawing Primitives](drawing.md) - Drawing shapes on buffers
- [Terminal Functions](terminal.md) - Getting terminal size
- [API Reference: Buffers](../api/buffers.md) - Complete API details

---

[← Text Utilities](text.md) | [Back to Documentation](../README.md) | [Next: Drawing →](drawing.md)
