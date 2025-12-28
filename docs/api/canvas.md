# API Reference: Canvas Functions

[Back to Documentation](../README.md) | [Feature Guide](../features/canvas.md)

---

## Constants

### Canvas Modes

| Constant | Value | Resolution per Cell |
|----------|-------|---------------------|
| `TUI_CANVAS_BRAILLE` | 0 | 2×4 dots (Braille patterns) |
| `TUI_CANVAS_BLOCK` | 1 | 2×2 pixels (block characters) |
| `TUI_CANVAS_ASCII` | 2 | 1×1 (standard resolution) |

---

## tui_canvas_create

Create a new high-resolution canvas.

```php
resource tui_canvas_create(int $char_width, int $char_height, int $mode = TUI_CANVAS_BRAILLE)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$char_width` | int | Width in character cells |
| `$char_height` | int | Height in character cells |
| `$mode` | int | Canvas mode constant |

### Return Value

Returns a canvas resource on success, `false` on failure.

### Example

```php
// 40×20 characters = 80×80 Braille pixels
$canvas = tui_canvas_create(40, 20, TUI_CANVAS_BRAILLE);

// 40×20 characters = 80×40 block pixels
$canvas = tui_canvas_create(40, 20, TUI_CANVAS_BLOCK);
```

### Related

- [tui_canvas_free()](#tui_canvas_free) - Free canvas
- [tui_canvas_get_resolution()](#tui_canvas_get_resolution) - Get pixel dimensions

---

## tui_canvas_free

Free a canvas and release memory.

```php
void tui_canvas_free(resource $canvas)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$canvas` | resource | Canvas to free |

---

## tui_canvas_clear

Clear all pixels from the canvas.

```php
void tui_canvas_clear(resource $canvas)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$canvas` | resource | Canvas to clear |

---

## tui_canvas_set

Set a pixel (turn it on).

```php
void tui_canvas_set(resource $canvas, int $x, int $y)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$canvas` | resource | Target canvas |
| `$x` | int | Pixel X coordinate |
| `$y` | int | Pixel Y coordinate |

### Example

```php
// Draw random dots
for ($i = 0; $i < 100; $i++) {
    tui_canvas_set($canvas, rand(0, 79), rand(0, 79));
}
```

---

## tui_canvas_unset

Unset a pixel (turn it off).

```php
void tui_canvas_unset(resource $canvas, int $x, int $y)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$canvas` | resource | Target canvas |
| `$x` | int | Pixel X coordinate |
| `$y` | int | Pixel Y coordinate |

---

## tui_canvas_toggle

Toggle a pixel's state.

```php
void tui_canvas_toggle(resource $canvas, int $x, int $y)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$canvas` | resource | Target canvas |
| `$x` | int | Pixel X coordinate |
| `$y` | int | Pixel Y coordinate |

---

## tui_canvas_get

Get a pixel's state.

```php
bool tui_canvas_get(resource $canvas, int $x, int $y)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$canvas` | resource | Source canvas |
| `$x` | int | Pixel X coordinate |
| `$y` | int | Pixel Y coordinate |

### Return Value

Returns `true` if pixel is set, `false` otherwise.

---

## tui_canvas_line

Draw a line on the canvas.

```php
void tui_canvas_line(resource $canvas, int $x1, int $y1, int $x2, int $y2)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$canvas` | resource | Target canvas |
| `$x1` | int | Start X coordinate |
| `$y1` | int | Start Y coordinate |
| `$x2` | int | End X coordinate |
| `$y2` | int | End Y coordinate |

### Example

```php
// Diagonal line
tui_canvas_line($canvas, 0, 0, 79, 79);

// Cross pattern
tui_canvas_line($canvas, 0, 40, 79, 40);
tui_canvas_line($canvas, 40, 0, 40, 79);
```

### Notes

- Uses Bresenham's line algorithm
- Operates on pixel coordinates

---

## tui_canvas_rect

Draw a rectangle outline.

```php
void tui_canvas_rect(resource $canvas, int $x, int $y, int $width, int $height)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$canvas` | resource | Target canvas |
| `$x` | int | Left edge X coordinate |
| `$y` | int | Top edge Y coordinate |
| `$width` | int | Rectangle width in pixels |
| `$height` | int | Rectangle height in pixels |

---

## tui_canvas_fill_rect

Draw a filled rectangle.

```php
void tui_canvas_fill_rect(resource $canvas, int $x, int $y, int $width, int $height)
```

### Parameters

Same as [tui_canvas_rect()](#tui_canvas_rect).

---

## tui_canvas_circle

Draw a circle outline.

```php
void tui_canvas_circle(resource $canvas, int $cx, int $cy, int $radius)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$canvas` | resource | Target canvas |
| `$cx` | int | Center X coordinate |
| `$cy` | int | Center Y coordinate |
| `$radius` | int | Radius in pixels |

### Example

```php
tui_canvas_circle($canvas, 40, 40, 30);
```

### Notes

- Uses midpoint circle algorithm
- Appears as true circle (unlike character-cell circles)

---

## tui_canvas_fill_circle

Draw a filled circle.

```php
void tui_canvas_fill_circle(resource $canvas, int $cx, int $cy, int $radius)
```

### Parameters

Same as [tui_canvas_circle()](#tui_canvas_circle).

---

## tui_canvas_render

Render the canvas to an array of strings.

```php
array tui_canvas_render(resource $canvas)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$canvas` | resource | Canvas to render |

### Return Value

Returns an array of strings, one per character row.

### Example

```php
$lines = tui_canvas_render($canvas);

// Display directly
foreach ($lines as $line) {
    echo $line . "\n";
}

// Or write to buffer
foreach ($lines as $i => $line) {
    tui_buffer_write($buf, 0, $i, $line, $style);
}
```

---

## tui_canvas_get_resolution

Get the pixel dimensions of a canvas.

```php
void tui_canvas_get_resolution(resource $canvas, int &$width, int &$height)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$canvas` | resource | Source canvas |
| `&$width` | int | Output: pixel width |
| `&$height` | int | Output: pixel height |

### Example

```php
$canvas = tui_canvas_create(40, 20, TUI_CANVAS_BRAILLE);
tui_canvas_get_resolution($canvas, $w, $h);
echo "Resolution: {$w}×{$h}\n";  // "Resolution: 80×80"
```

---

## tui_canvas_set_color

Set the drawing color for colored output.

```php
void tui_canvas_set_color(resource $canvas, int $r, int $g, int $b)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$canvas` | resource | Target canvas |
| `$r` | int | Red component (0-255) |
| `$g` | int | Green component (0-255) |
| `$b` | int | Blue component (0-255) |

### Notes

- Color support depends on terminal and rendering method
- May be applied when rendering to buffer with style

---

[Back to Documentation](../README.md) | [Feature Guide](../features/canvas.md)
