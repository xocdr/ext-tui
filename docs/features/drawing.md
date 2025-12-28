# Drawing Primitives

[← Buffers](buffers.md) | [Back to Documentation](../README.md) | [Next: Canvas →](canvas.md)

---

Drawing primitives let you create lines, boxes, circles, and other shapes on buffers using Unicode characters.

## Overview

All drawing functions operate on buffers and use:
- **Character cells**: Each position holds one character
- **Unicode box-drawing**: `─│┌┐└┘├┤┬┴┼` for clean lines
- **Fill characters**: Any character for filled shapes
- **Styles**: Colors and text attributes

## Functions

### tui_draw_line()

Draw a line between two points.

```php
void tui_draw_line(resource $buffer, int $x1, int $y1, int $x2, int $y2, string $char, array $style)
```

**Parameters**:
- `$buffer`: Target buffer
- `$x1, $y1`: Start point
- `$x2, $y2`: End point
- `$char`: Character to draw with
- `$style`: Style array

**Example**:
```php
// Horizontal line
tui_draw_line($buf, 0, 5, 40, 5, '─', $style);

// Vertical line
tui_draw_line($buf, 20, 0, 20, 15, '│', $style);

// Diagonal line (Bresenham's algorithm)
tui_draw_line($buf, 0, 0, 30, 15, '●', $style);
```

---

### tui_draw_box()

Draw a box outline with Unicode box-drawing characters.

```php
void tui_draw_box(resource $buffer, int $x, int $y, int $width, int $height, array $style)
```

**Parameters**:
- `$buffer`: Target buffer
- `$x, $y`: Top-left corner
- `$width`: Box width
- `$height`: Box height
- `$style`: Style array

**Example**:
```php
// Simple box
tui_draw_box($buf, 5, 2, 30, 10, $style);
/*
┌────────────────────────────┐
│                            │
│                            │
│                            │
└────────────────────────────┘
*/

// Titled box (draw box, then overwrite top edge)
tui_draw_box($buf, 0, 0, 40, 12, $style);
tui_buffer_write($buf, 2, 0, " Settings ", $title_style);
```

---

### tui_fill_rect()

Fill a rectangular area with a character.

```php
void tui_fill_rect(resource $buffer, int $x, int $y, int $width, int $height, string $char, array $style)
```

**Parameters**:
- `$buffer`: Target buffer
- `$x, $y`: Top-left corner
- `$width`: Rectangle width
- `$height`: Rectangle height
- `$char`: Fill character
- `$style`: Style array

**Example**:
```php
// Solid fill
tui_fill_rect($buf, 10, 5, 20, 8, '█', $style);

// Shaded fill
tui_fill_rect($buf, 10, 5, 20, 8, '░', $style);

// Background color (space with bg color)
$bg_style = ['bg' => [50, 50, 100]];
tui_fill_rect($buf, 0, 0, $width, $height, ' ', $bg_style);
```

---

### tui_draw_circle()

Draw a circle outline.

```php
void tui_draw_circle(resource $buffer, int $cx, int $cy, int $radius, string $char, array $style)
```

**Parameters**:
- `$buffer`: Target buffer
- `$cx, $cy`: Center point
- `$radius`: Radius in character cells
- `$char`: Character to draw with
- `$style`: Style array

**Example**:
```php
// Circle outline
tui_draw_circle($buf, 20, 10, 8, '○', $style);

// Asterisk circle
tui_draw_circle($buf, 20, 10, 8, '*', $style);
```

**Note**: Terminal characters are typically taller than wide (~2:1 ratio), so circles may appear as ovals. For true circles, use the canvas with Braille graphics.

---

### tui_fill_circle()

Draw a filled circle.

```php
void tui_fill_circle(resource $buffer, int $cx, int $cy, int $radius, string $char, array $style)
```

**Parameters**:
- `$buffer`: Target buffer
- `$cx, $cy`: Center point
- `$radius`: Radius
- `$char`: Fill character
- `$style`: Style array

**Example**:
```php
// Solid filled circle
tui_fill_circle($buf, 20, 10, 8, '●', $style);

// Shaded circle
tui_fill_circle($buf, 20, 10, 8, '░', $style);
```

---

### tui_draw_triangle()

Draw a triangle outline.

```php
void tui_draw_triangle(resource $buffer, int $x1, int $y1, int $x2, int $y2, int $x3, int $y3, string $char, array $style)
```

**Parameters**:
- `$buffer`: Target buffer
- `$x1, $y1`: First vertex
- `$x2, $y2`: Second vertex
- `$x3, $y3`: Third vertex
- `$char`: Character to draw with
- `$style`: Style array

**Example**:
```php
// Equilateral-ish triangle
tui_draw_triangle($buf, 20, 2, 10, 12, 30, 12, '*', $style);
```

---

### tui_fill_triangle()

Draw a filled triangle.

```php
void tui_fill_triangle(resource $buffer, int $x1, int $y1, int $x2, int $y2, int $x3, int $y3, string $char, array $style)
```

**Parameters**: Same as `tui_draw_triangle()`

**Example**:
```php
tui_fill_triangle($buf, 20, 2, 10, 12, 30, 12, '▲', $style);
```

---

### tui_draw_arc()

Draw an arc (partial circle).

```php
void tui_draw_arc(resource $buffer, int $cx, int $cy, int $radius, float $start_angle, float $end_angle, string $char, array $style)
```

**Parameters**:
- `$buffer`: Target buffer
- `$cx, $cy`: Center point
- `$radius`: Radius
- `$start_angle`: Start angle in radians
- `$end_angle`: End angle in radians
- `$char`: Character to draw with
- `$style`: Style array

**Example**:
```php
// Quarter circle (top-right quadrant)
tui_draw_arc($buf, 20, 10, 8, 0, M_PI / 2, '●', $style);

// Half circle (top half)
tui_draw_arc($buf, 20, 10, 8, 0, M_PI, '●', $style);

// Three-quarter circle
tui_draw_arc($buf, 20, 10, 8, M_PI / 4, 2 * M_PI - M_PI / 4, '●', $style);
```

---

### tui_draw_bezier()

Draw a quadratic Bezier curve.

```php
void tui_draw_bezier(resource $buffer, int $x1, int $y1, int $cx, int $cy, int $x2, int $y2, string $char, array $style)
```

**Parameters**:
- `$buffer`: Target buffer
- `$x1, $y1`: Start point
- `$cx, $cy`: Control point
- `$x2, $y2`: End point
- `$char`: Character to draw with
- `$style`: Style array

**Example**:
```php
// Smooth curve
tui_draw_bezier($buf, 5, 15, 20, 2, 35, 15, '·', $style);
```

---

## Practical Patterns

### Dashboard Panel

```php
function draw_panel($buf, int $x, int $y, int $w, int $h, string $title, array $style): void
{
    // Draw box
    tui_draw_box($buf, $x, $y, $w, $h, $style);

    // Add title
    if ($title) {
        $title_text = " $title ";
        tui_buffer_write($buf, $x + 2, $y, $title_text, [
            'fg' => $style['fg'] ?? [255, 255, 255],
            'bold' => true
        ]);
    }
}

// Usage
draw_panel($buf, 0, 0, 40, 12, "CPU Usage", $panel_style);
draw_panel($buf, 42, 0, 38, 12, "Memory", $panel_style);
```

### Window with Shadow

```php
function draw_window($buf, int $x, int $y, int $w, int $h, string $title): void
{
    $shadow_style = ['fg' => [80, 80, 80], 'bg' => [40, 40, 40]];
    $window_style = ['fg' => [200, 200, 200], 'bg' => [60, 60, 80]];
    $title_style = ['fg' => [255, 255, 255], 'bold' => true];

    // Shadow (offset by 1,1)
    tui_fill_rect($buf, $x + 2, $y + 1, $w, $h, '░', $shadow_style);

    // Window background
    tui_fill_rect($buf, $x, $y, $w, $h, ' ', $window_style);

    // Window border
    tui_draw_box($buf, $x, $y, $w, $h, $window_style);

    // Title bar
    $title_bg = ['bg' => [100, 100, 150]];
    tui_fill_rect($buf, $x + 1, $y + 1, $w - 2, 1, ' ', $title_bg);
    tui_buffer_write($buf, $x + 2, $y + 1, $title, $title_style);

    // Close button
    tui_buffer_write($buf, $x + $w - 4, $y + 1, "[X]", ['fg' => [255, 100, 100]]);
}
```

### Bar Chart

```php
function draw_bar_chart($buf, int $x, int $y, int $width, int $height, array $values, array $labels): void
{
    $max_value = max($values);
    $bar_width = (int)(($width - 2) / count($values));
    $chart_height = $height - 2;  // Leave room for labels

    $bar_style = ['fg' => [100, 200, 100]];
    $label_style = ['fg' => [180, 180, 180]];

    // Draw bars
    foreach ($values as $i => $value) {
        $bar_height = (int)(($value / $max_value) * $chart_height);
        $bar_x = $x + 1 + ($i * $bar_width);
        $bar_y = $y + $chart_height - $bar_height;

        // Draw bar
        for ($row = 0; $row < $bar_height; $row++) {
            tui_fill_rect($buf, $bar_x, $bar_y + $row, $bar_width - 1, 1, '█', $bar_style);
        }

        // Draw label
        $label = tui_truncate($labels[$i] ?? '', $bar_width - 1);
        tui_buffer_write($buf, $bar_x, $y + $height - 1, $label, $label_style);
    }

    // Draw axis
    tui_draw_line($buf, $x, $y, $x, $y + $height - 2, '│', $label_style);
    tui_draw_line($buf, $x, $y + $height - 2, $x + $width - 1, $y + $height - 2, '─', $label_style);
}

// Usage
$values = [25, 60, 45, 80, 30];
$labels = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri'];
draw_bar_chart($buf, 5, 2, 40, 15, $values, $labels);
```

### Pie Chart (ASCII)

```php
function draw_pie_indicator($buf, int $x, int $y, float $percentage): void
{
    // Simple percentage circle using characters
    $chars = ['○', '◔', '◑', '◕', '●'];
    $index = (int)($percentage * 4);
    $index = max(0, min(4, $index));

    $style = ['fg' => [100, 200, 255]];
    tui_buffer_write($buf, $x, $y, $chars[$index], $style);
}
```

### Gradient Fill

```php
function gradient_fill($buf, int $x, int $y, int $width, int $height, array $from_color, array $to_color): void
{
    for ($row = 0; $row < $height; $row++) {
        $t = $height > 1 ? $row / ($height - 1) : 0;

        $color = [
            (int)($from_color[0] + ($to_color[0] - $from_color[0]) * $t),
            (int)($from_color[1] + ($to_color[1] - $from_color[1]) * $t),
            (int)($from_color[2] + ($to_color[2] - $from_color[2]) * $t),
        ];

        tui_fill_rect($buf, $x, $y + $row, $width, 1, ' ', ['bg' => $color]);
    }
}

// Usage: Blue to purple gradient
gradient_fill($buf, 0, 0, 40, 20, [50, 50, 200], [150, 50, 200]);
```

## Box Drawing Characters Reference

```
Single line:
┌─┬─┐  ─ horizontal
├─┼─┤  │ vertical
└─┴─┘  ┌┐└┘ corners
       ├┤┬┴┼ junctions

Double line:
╔═╦═╗
╠═╬═╣
╚═╩═╝

Rounded:
╭─╮
│ │
╰─╯

Block shading:
░ light   (25%)
▒ medium  (50%)
▓ dark    (75%)
█ solid   (100%)
```

## Related Functions

- [tui_buffer_write()](../api/buffers.md#tui_buffer_write) - Write text
- [tui_buffer_set_cell()](../api/buffers.md#tui_buffer_set_cell) - Set individual cells
- [tui_canvas_line()](../api/canvas.md#tui_canvas_line) - High-res lines

## See Also

- [Canvas](canvas.md) - High-resolution graphics with Braille
- [Buffers](buffers.md) - Buffer management
- [API Reference: Drawing](../api/drawing.md) - Complete API details

---

[← Buffers](buffers.md) | [Back to Documentation](../README.md) | [Next: Canvas →](canvas.md)
