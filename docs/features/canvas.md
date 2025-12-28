# Canvas (High-Resolution Graphics)

[← Drawing](drawing.md) | [Back to Documentation](../README.md) | [Next: Animation →](animation.md)

---

The canvas provides high-resolution graphics by using Unicode Braille patterns or block characters, allowing sub-character pixel-level drawing.

## Overview

Normal terminal drawing is limited to character-cell resolution. The canvas overcomes this by:

- **Braille mode**: 2×4 dots per character cell = 8× more resolution
- **Block mode**: 2×2 pixels per character cell = 4× more resolution
- **ASCII mode**: 1×1 (standard resolution, for compatibility)

This enables smooth curves, detailed graphs, and precise graphics.

## Resolution Comparison

```
Standard (40×20 chars):
████████████████████
█                  █
█     ██████       █
█                  █
████████████████████

Braille (80×80 virtual pixels in same space):
⡏⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⢹
⡇⠀⠀⠀⠀⠀⣀⣤⣴⣶⣶⣤⣄⠀⠀⠀⠀⠀⢸
⡇⠀⠀⠀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣷⡄⠀⠀⠀⢸
⣇⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣀⣸
```

## Canvas Modes

| Mode | Constant | Resolution | Characters Used |
|------|----------|------------|-----------------|
| Braille | `TUI_CANVAS_BRAILLE` | 2×4 per cell | `⠀⠁⠂...⣿` (256 patterns) |
| Block | `TUI_CANVAS_BLOCK` | 2×2 per cell | `▀▄▌▐█` etc (16 patterns) |
| ASCII | `TUI_CANVAS_ASCII` | 1×1 per cell | `#` and space |

## Functions

### tui_canvas_create()

Create a new canvas.

```php
resource tui_canvas_create(int $char_width, int $char_height, int $mode = TUI_CANVAS_BRAILLE)
```

**Parameters**:
- `$char_width`: Width in character cells
- `$char_height`: Height in character cells
- `$mode`: Canvas mode (see above)

**Returns**: Canvas resource

**Example**:
```php
// Create a 40×20 character canvas with Braille (80×80 pixels)
$canvas = tui_canvas_create(40, 20, TUI_CANVAS_BRAILLE);

// Get actual pixel resolution
tui_canvas_get_resolution($canvas, $pixel_width, $pixel_height);
echo "Canvas is {$pixel_width}×{$pixel_height} pixels\n";  // 80×80
```

---

### tui_canvas_free()

Free a canvas and release memory.

```php
void tui_canvas_free(resource $canvas)
```

---

### tui_canvas_clear()

Clear all pixels from the canvas.

```php
void tui_canvas_clear(resource $canvas)
```

---

### tui_canvas_set()

Set a pixel (turn it on).

```php
void tui_canvas_set(resource $canvas, int $x, int $y)
```

**Example**:
```php
// Draw individual pixels
for ($i = 0; $i < 100; $i++) {
    $x = rand(0, 79);
    $y = rand(0, 79);
    tui_canvas_set($canvas, $x, $y);
}
```

---

### tui_canvas_unset()

Unset a pixel (turn it off).

```php
void tui_canvas_unset(resource $canvas, int $x, int $y)
```

---

### tui_canvas_toggle()

Toggle a pixel's state.

```php
void tui_canvas_toggle(resource $canvas, int $x, int $y)
```

---

### tui_canvas_get()

Get a pixel's state.

```php
bool tui_canvas_get(resource $canvas, int $x, int $y)
```

**Returns**: `true` if pixel is set, `false` otherwise

---

### tui_canvas_line()

Draw a line on the canvas.

```php
void tui_canvas_line(resource $canvas, int $x1, int $y1, int $x2, int $y2)
```

**Example**:
```php
// Diagonal line
tui_canvas_line($canvas, 0, 0, 79, 79);

// Cross pattern
tui_canvas_line($canvas, 0, 40, 79, 40);  // Horizontal
tui_canvas_line($canvas, 40, 0, 40, 79);  // Vertical
```

---

### tui_canvas_rect()

Draw a rectangle outline.

```php
void tui_canvas_rect(resource $canvas, int $x, int $y, int $width, int $height)
```

**Example**:
```php
tui_canvas_rect($canvas, 10, 10, 60, 60);
```

---

### tui_canvas_fill_rect()

Draw a filled rectangle.

```php
void tui_canvas_fill_rect(resource $canvas, int $x, int $y, int $width, int $height)
```

---

### tui_canvas_circle()

Draw a circle outline.

```php
void tui_canvas_circle(resource $canvas, int $cx, int $cy, int $radius)
```

**Example**:
```php
// Centered circle
tui_canvas_circle($canvas, 40, 40, 30);
```

**Note**: Unlike character-cell circles, these appear as proper circles because Braille dots have uniform spacing.

---

### tui_canvas_fill_circle()

Draw a filled circle.

```php
void tui_canvas_fill_circle(resource $canvas, int $cx, int $cy, int $radius)
```

---

### tui_canvas_render()

Render the canvas to an array of strings.

```php
array tui_canvas_render(resource $canvas)
```

**Returns**: Array of strings, one per character row

**Example**:
```php
$lines = tui_canvas_render($canvas);

// Display directly
foreach ($lines as $line) {
    echo $line . "\n";
}

// Or copy to a buffer
foreach ($lines as $i => $line) {
    tui_buffer_write($buf, $x, $y + $i, $line, $style);
}
```

---

### tui_canvas_get_resolution()

Get the pixel dimensions of a canvas.

```php
void tui_canvas_get_resolution(resource $canvas, int &$width, int &$height)
```

**Parameters**:
- `$canvas`: Canvas resource
- `$width`: Output parameter for pixel width
- `$height`: Output parameter for pixel height

**Example**:
```php
$canvas = tui_canvas_create(40, 20, TUI_CANVAS_BRAILLE);
tui_canvas_get_resolution($canvas, $w, $h);
echo "Resolution: {$w}×{$h}\n";  // "Resolution: 80×80"
```

---

### tui_canvas_set_color()

Set the drawing color (for terminals that support it).

```php
void tui_canvas_set_color(resource $canvas, int $r, int $g, int $b)
```

**Note**: Color support depends on terminal capabilities and rendering method.

---

## Practical Patterns

### Sine Wave

```php
$canvas = tui_canvas_create(60, 15, TUI_CANVAS_BRAILLE);
tui_canvas_get_resolution($canvas, $width, $height);

$center_y = $height / 2;
$amplitude = $height / 2 - 2;

$prev_y = null;
for ($x = 0; $x < $width; $x++) {
    $y = (int)($center_y + $amplitude * sin($x * 0.1));

    if ($prev_y !== null) {
        tui_canvas_line($canvas, $x - 1, $prev_y, $x, $y);
    } else {
        tui_canvas_set($canvas, $x, $y);
    }
    $prev_y = $y;
}

$lines = tui_canvas_render($canvas);
foreach ($lines as $line) {
    echo $line . "\n";
}
```

### Real-Time Graph

```php
class LiveGraph
{
    private $canvas;
    private array $data = [];
    private int $max_points;
    private int $width;
    private int $height;

    public function __construct(int $char_width, int $char_height)
    {
        $this->canvas = tui_canvas_create($char_width, $char_height, TUI_CANVAS_BRAILLE);
        tui_canvas_get_resolution($this->canvas, $this->width, $this->height);
        $this->max_points = $this->width;
    }

    public function addPoint(float $value): void
    {
        $this->data[] = $value;
        if (count($this->data) > $this->max_points) {
            array_shift($this->data);
        }
    }

    public function render(): array
    {
        tui_canvas_clear($this->canvas);

        if (count($this->data) < 2) {
            return tui_canvas_render($this->canvas);
        }

        $min = min($this->data);
        $max = max($this->data);
        $range = $max - $min ?: 1;

        $prev_x = null;
        $prev_y = null;

        foreach ($this->data as $i => $value) {
            $x = $i;
            $y = $this->height - 1 - (int)((($value - $min) / $range) * ($this->height - 1));

            if ($prev_x !== null) {
                tui_canvas_line($this->canvas, $prev_x, $prev_y, $x, $y);
            }

            $prev_x = $x;
            $prev_y = $y;
        }

        return tui_canvas_render($this->canvas);
    }
}

// Usage
$graph = new LiveGraph(60, 10);
while (true) {
    $graph->addPoint(get_cpu_usage());
    $lines = $graph->render();
    // Display $lines...
    usleep(100000);
}
```

### Scatter Plot

```php
function scatter_plot(array $points, int $char_width, int $char_height): array
{
    $canvas = tui_canvas_create($char_width, $char_height, TUI_CANVAS_BRAILLE);
    tui_canvas_get_resolution($canvas, $width, $height);

    // Find data bounds
    $min_x = $max_x = $points[0][0];
    $min_y = $max_y = $points[0][1];
    foreach ($points as [$x, $y]) {
        $min_x = min($min_x, $x);
        $max_x = max($max_x, $x);
        $min_y = min($min_y, $y);
        $max_y = max($max_y, $y);
    }

    $range_x = $max_x - $min_x ?: 1;
    $range_y = $max_y - $min_y ?: 1;

    // Draw axes
    tui_canvas_line($canvas, 0, $height - 1, $width - 1, $height - 1);  // X axis
    tui_canvas_line($canvas, 0, 0, 0, $height - 1);  // Y axis

    // Plot points
    foreach ($points as [$x, $y]) {
        $px = (int)((($x - $min_x) / $range_x) * ($width - 2)) + 1;
        $py = $height - 2 - (int)((($y - $min_y) / $range_y) * ($height - 2));

        // Draw a small dot (3x3 for visibility)
        for ($dx = -1; $dx <= 1; $dx++) {
            for ($dy = -1; $dy <= 1; $dy++) {
                tui_canvas_set($canvas, $px + $dx, $py + $dy);
            }
        }
    }

    $result = tui_canvas_render($canvas);
    tui_canvas_free($canvas);
    return $result;
}
```

### Animated Loading Spinner

```php
function draw_arc_spinner(int $frame): array
{
    $canvas = tui_canvas_create(5, 5, TUI_CANVAS_BRAILLE);
    tui_canvas_get_resolution($canvas, $w, $h);

    $cx = $w / 2;
    $cy = $h / 2;
    $radius = min($w, $h) / 2 - 2;

    // Draw arc that rotates
    $start = ($frame * 0.2);
    $length = M_PI * 1.5;

    for ($a = 0; $a < $length; $a += 0.1) {
        $angle = $start + $a;
        $x = (int)($cx + $radius * cos($angle));
        $y = (int)($cy + $radius * sin($angle));
        tui_canvas_set($canvas, $x, $y);
    }

    $result = tui_canvas_render($canvas);
    tui_canvas_free($canvas);
    return $result;
}
```

### Block Mode for Retro Graphics

```php
// Block mode is great for retro pixel-art style
$canvas = tui_canvas_create(20, 10, TUI_CANVAS_BLOCK);
tui_canvas_get_resolution($canvas, $w, $h);  // 40×20 pixels

// Draw a simple sprite
$sprite = [
    "  ####  ",
    " #    # ",
    "#  ##  #",
    "# #  # #",
    "#      #",
    "# #  # #",
    " # ## # ",
    "  ####  ",
];

$start_x = 16;
$start_y = 6;

foreach ($sprite as $row => $line) {
    for ($col = 0; $col < strlen($line); $col++) {
        if ($line[$col] === '#') {
            tui_canvas_set($canvas, $start_x + $col, $start_y + $row);
        }
    }
}

$lines = tui_canvas_render($canvas);
```

## Performance Tips

1. **Batch operations**: Draw all primitives before rendering
2. **Reuse canvases**: Clear and redraw rather than recreating
3. **Choose appropriate mode**: Block mode is faster than Braille for larger areas
4. **Minimize resolution**: Use the smallest canvas that provides adequate quality

## Braille Character Reference

Braille patterns encode 8 dots in a 2×4 grid:

```
Dot positions:    Bit values:
 1  4             0x01  0x08
 2  5             0x02  0x10
 3  6             0x04  0x20
 7  8             0x40  0x80

Unicode: U+2800 + (bit pattern)
Example: ⣿ = U+28FF = all dots set
```

## Related Functions

- [tui_draw_circle()](../api/drawing.md#tui_draw_circle) - Character-cell circles
- [tui_animation_create()](../api/animation.md#tui_animation_create) - Animate canvas drawings
- [tui_sprite_create()](../api/sprites.md#tui_sprite_create) - Sprites from canvas data

## See Also

- [Drawing Primitives](drawing.md) - Character-cell drawing
- [Animation](animation.md) - Animating graphics
- [API Reference: Canvas](../api/canvas.md) - Complete API details

---

[← Drawing](drawing.md) | [Back to Documentation](../README.md) | [Next: Animation →](animation.md)
