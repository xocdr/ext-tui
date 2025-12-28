# API Reference: Buffer Functions

[Back to Documentation](../README.md) | [Feature Guide](../features/buffers.md)

---

## tui_buffer_create

Create a new screen buffer.

```php
resource tui_buffer_create(int $width, int $height)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$width` | int | Buffer width in columns |
| `$height` | int | Buffer height in rows |

### Return Value

Returns a buffer resource on success, or `false` on failure.

### Example

```php
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

if (!$buf) {
    die("Failed to create buffer\n");
}
```

### Notes

- Allocates memory for width × height cells
- Each cell stores: character, foreground color, background color, attributes
- Maximum dimensions: 10000×10000

### Related

- [tui_buffer_free()](buffers.md#tui_buffer_free) - Free buffer
- [tui_get_terminal_size()](terminal.md#tui_get_terminal_size) - Get dimensions

---

## tui_buffer_free

Free a buffer and release memory.

```php
void tui_buffer_free(resource $buffer)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$buffer` | resource | Buffer to free |

### Return Value

None.

### Example

```php
$buf = tui_buffer_create(80, 24);
// ... use buffer ...
tui_buffer_free($buf);
$buf = null;
```

### Notes

- Always free buffers when no longer needed
- Safe to call on already-freed buffers (no-op)

---

## tui_buffer_clear

Clear all cells in a buffer.

```php
void tui_buffer_clear(resource $buffer)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$buffer` | resource | Buffer to clear |

### Return Value

None.

### Example

```php
while ($running) {
    tui_buffer_clear($buf);
    draw_frame($buf);
    tui_render($buf);
}
```

### Notes

- Resets all cells to space with default colors
- More efficient than recreating buffer

---

## tui_buffer_write

Write text to a buffer at a specific position.

```php
void tui_buffer_write(resource $buffer, int $x, int $y, string $text, array $style)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$buffer` | resource | Target buffer |
| `$x` | int | Column position (0-based) |
| `$y` | int | Row position (0-based) |
| `$text` | string | Text to write |
| `$style` | array | Style array |

### Style Array

```php
$style = [
    'fg' => [255, 255, 255],   // RGB foreground
    'bg' => [0, 0, 0],         // RGB background
    'bold' => true,            // Bold text
    'dim' => true,             // Dimmed text
    'italic' => true,          // Italic text
    'underline' => true,       // Underlined
    'blink' => true,           // Blinking
    'reverse' => true,         // Swap fg/bg
    'hidden' => true,          // Hidden
    'strikethrough' => true,   // Strikethrough
];
```

### Return Value

None.

### Example

```php
$style = ['fg' => [100, 200, 255], 'bold' => true];
tui_buffer_write($buf, 5, 3, "Hello, World!", $style);
```

### Notes

- Text that extends beyond buffer bounds is clipped
- Handles Unicode characters correctly
- Empty style array `[]` uses default terminal colors

### Related

- [tui_buffer_set_cell()](buffers.md#tui_buffer_set_cell) - Set single cell

---

## tui_buffer_set_cell

Set a single cell in the buffer.

```php
void tui_buffer_set_cell(resource $buffer, int $x, int $y, string $char, array $style)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$buffer` | resource | Target buffer |
| `$x` | int | Column position |
| `$y` | int | Row position |
| `$char` | string | Single character (may be multi-byte UTF-8) |
| `$style` | array | Style array |

### Return Value

None.

### Example

```php
// Draw border corners
tui_buffer_set_cell($buf, 0, 0, "┌", $style);
tui_buffer_set_cell($buf, 10, 0, "┐", $style);
tui_buffer_set_cell($buf, 0, 5, "└", $style);
tui_buffer_set_cell($buf, 10, 5, "┘", $style);
```

### Notes

- For single characters, more efficient than `tui_buffer_write()`
- Character can be any valid UTF-8 character

### Related

- [tui_buffer_write()](buffers.md#tui_buffer_write) - Write text

---

## tui_buffer_render

Render a buffer to the terminal.

```php
void tui_buffer_render(resource $buffer)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$buffer` | resource | Buffer to render |

### Return Value

None.

### Example

```php
// Draw everything to buffer
tui_buffer_write($buf, 0, 0, "Header", $style);
tui_draw_rect($buf, 0, 1, 40, 10, $style);

// Single render call
tui_buffer_render($buf);
```

### Notes

- Outputs optimized escape sequences
- Moves cursor to minimize output
- Clears screen on first render
- Subsequent renders only update changed cells

### Related

- [tui_buffer_create()](buffers.md#tui_buffer_create) - Create buffer
- [tui_buffer_clear()](buffers.md#tui_buffer_clear) - Clear buffer

---

[Back to Documentation](../README.md) | [Feature Guide](../features/buffers.md)
