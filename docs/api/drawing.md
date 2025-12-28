# API Reference: Drawing Functions

[Back to Documentation](../README.md) | [Feature Guide](../features/drawing.md)

---

## tui_draw_line

Draw a line between two points.

```php
void tui_draw_line(resource $buffer, int $x1, int $y1, int $x2, int $y2, string $char, array $style)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$buffer` | resource | Target buffer |
| `$x1` | int | Start X coordinate |
| `$y1` | int | Start Y coordinate |
| `$x2` | int | End X coordinate |
| `$y2` | int | End Y coordinate |
| `$char` | string | Character to draw with |
| `$style` | array | Style array |

### Return Value

None.

### Example

```php
// Horizontal line
tui_draw_line($buf, 0, 5, 40, 5, '─', $style);

// Vertical line
tui_draw_line($buf, 20, 0, 20, 15, '│', $style);

// Diagonal line
tui_draw_line($buf, 0, 0, 30, 15, '*', $style);
```

### Notes

- Uses Bresenham's line algorithm
- Coordinates outside buffer are clipped

### Related

- [tui_canvas_line()](canvas.md#tui_canvas_line) - High-resolution line

---

## tui_draw_rect

Draw a rectangle outline with Unicode box-drawing characters.

```php
void tui_draw_rect(resource $buffer, int $x, int $y, int $width, int $height, array $style)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$buffer` | resource | Target buffer |
| `$x` | int | Left edge X coordinate |
| `$y` | int | Top edge Y coordinate |
| `$width` | int | Rectangle width |
| `$height` | int | Rectangle height |
| `$style` | array | Style array |

### Return Value

None.

### Example

```php
tui_draw_rect($buf, 5, 2, 30, 10, $style);
// ┌────────────────────────────┐
// │                            │
// │                            │
// └────────────────────────────┘
```

### Notes

- Uses Unicode box-drawing characters: `┌─┐│└┘`
- Minimum size: 2×2

### Related

- [tui_fill_rect()](drawing.md#tui_fill_rect) - Filled rectangle

---

## tui_fill_rect

Fill a rectangular area with a character.

```php
void tui_fill_rect(resource $buffer, int $x, int $y, int $width, int $height, string $char, array $style)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$buffer` | resource | Target buffer |
| `$x` | int | Left edge X coordinate |
| `$y` | int | Top edge Y coordinate |
| `$width` | int | Rectangle width |
| `$height` | int | Rectangle height |
| `$char` | string | Fill character |
| `$style` | array | Style array |

### Return Value

None.

### Example

```php
// Solid fill
tui_fill_rect($buf, 10, 5, 20, 8, '█', $style);

// Background color
$bg = ['bg' => [50, 50, 100]];
tui_fill_rect($buf, 0, 0, $width, $height, ' ', $bg);
```

### Related

- [tui_draw_box()](drawing.md#tui_draw_box) - Box outline

---

## tui_draw_circle

Draw a circle outline.

```php
void tui_draw_circle(resource $buffer, int $cx, int $cy, int $radius, string $char, array $style)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$buffer` | resource | Target buffer |
| `$cx` | int | Center X coordinate |
| `$cy` | int | Center Y coordinate |
| `$radius` | int | Radius in character cells |
| `$char` | string | Character to draw with |
| `$style` | array | Style array |

### Return Value

None.

### Example

```php
tui_draw_circle($buf, 20, 10, 8, 'o', $style);
```

### Notes

- Uses midpoint circle algorithm
- Characters are taller than wide, so circles appear as ovals
- For true circles, use [tui_canvas_circle()](canvas.md#tui_canvas_circle)

### Related

- [tui_fill_circle()](drawing.md#tui_fill_circle) - Filled circle
- [tui_canvas_circle()](canvas.md#tui_canvas_circle) - High-resolution circle

---

## tui_fill_circle

Draw a filled circle.

```php
void tui_fill_circle(resource $buffer, int $cx, int $cy, int $radius, string $char, array $style)
```

### Parameters

Same as [tui_draw_circle()](#tui_draw_circle).

### Example

```php
tui_fill_circle($buf, 20, 10, 8, '●', $style);
```

---

## tui_draw_triangle

Draw a triangle outline.

```php
void tui_draw_triangle(resource $buffer, int $x1, int $y1, int $x2, int $y2, int $x3, int $y3, string $char, array $style)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$buffer` | resource | Target buffer |
| `$x1, $y1` | int | First vertex |
| `$x2, $y2` | int | Second vertex |
| `$x3, $y3` | int | Third vertex |
| `$char` | string | Character to draw with |
| `$style` | array | Style array |

### Return Value

None.

### Example

```php
tui_draw_triangle($buf, 20, 2, 10, 12, 30, 12, '*', $style);
```

---

## tui_fill_triangle

Draw a filled triangle.

```php
void tui_fill_triangle(resource $buffer, int $x1, int $y1, int $x2, int $y2, int $x3, int $y3, string $char, array $style)
```

### Parameters

Same as [tui_draw_triangle()](#tui_draw_triangle).

### Example

```php
tui_fill_triangle($buf, 20, 2, 10, 12, 30, 12, '▲', $style);
```

### Notes

- Uses scanline fill algorithm
- Handles degenerate triangles (collinear points)

---

## tui_draw_arc

Draw an arc (partial circle).

```php
void tui_draw_arc(resource $buffer, int $cx, int $cy, int $radius, float $start_angle, float $end_angle, string $char, array $style)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$buffer` | resource | Target buffer |
| `$cx` | int | Center X coordinate |
| `$cy` | int | Center Y coordinate |
| `$radius` | int | Radius |
| `$start_angle` | float | Start angle in radians |
| `$end_angle` | float | End angle in radians |
| `$char` | string | Character to draw with |
| `$style` | array | Style array |

### Return Value

None.

### Example

```php
// Quarter circle (top-right)
tui_draw_arc($buf, 20, 10, 8, 0, M_PI / 2, '●', $style);

// Half circle (right side)
tui_draw_arc($buf, 20, 10, 8, -M_PI / 2, M_PI / 2, '●', $style);
```

### Notes

- Angles in radians (0 = right, π/2 = down)
- Arc goes counterclockwise from start to end

---

## tui_draw_bezier

Draw a quadratic Bezier curve.

```php
void tui_draw_bezier(resource $buffer, int $x1, int $y1, int $cx, int $cy, int $x2, int $y2, string $char, array $style)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$buffer` | resource | Target buffer |
| `$x1, $y1` | int | Start point |
| `$cx, $cy` | int | Control point |
| `$x2, $y2` | int | End point |
| `$char` | string | Character to draw with |
| `$style` | array | Style array |

### Return Value

None.

### Example

```php
// Curved line
tui_draw_bezier($buf, 5, 15, 20, 2, 35, 15, '·', $style);
```

### Notes

- Uses de Casteljau's algorithm
- Control point "pulls" the curve toward it

---

[Back to Documentation](../README.md) | [Feature Guide](../features/drawing.md)
