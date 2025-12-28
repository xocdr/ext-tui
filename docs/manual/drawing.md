# Drawing

Low-level graphics using buffers, canvas, and drawing primitives.

## Overview

ext-tui provides two approaches for graphics:

1. **Components** - TuiBox/TuiText for structured layouts (recommended)
2. **Drawing API** - Buffers and primitives for custom graphics

The drawing API is useful for:
- Games and visualizations
- Custom widgets
- Charts and diagrams
- ASCII art and animations

## Buffers

Buffers are off-screen drawing surfaces.

### Creating Buffers

```php
[$width, $height] = tui_get_terminal_size();

$buffer = tui_buffer_create($width, $height);
```

### Drawing to Buffers

```php
// Clear the buffer
tui_buffer_clear($buffer);

// Draw to screen
tui_buffer_render($buffer);
```

## Drawing Primitives

### Lines

```php
tui_draw_line($buffer, $x1, $y1, $x2, $y2, [
    'char' => '─',
    'fg' => [100, 200, 255],
]);
```

### Rectangles

```php
// Outline
tui_draw_rect($buffer, $x, $y, $width, $height, [
    'char' => '█',
    'fg' => [100, 150, 255],
]);

// Filled
tui_fill_rect($buffer, $x, $y, $width, $height, [
    'char' => '░',
    'fg' => [100, 100, 100],
]);
```

### Circles

```php
// Outline
tui_draw_circle($buffer, $centerX, $centerY, $radius, [
    'char' => '●',
    'fg' => [255, 200, 0],
]);

// Filled
tui_fill_circle($buffer, $centerX, $centerY, $radius, [
    'char' => '█',
    'fg' => [100, 255, 100],
]);
```

### Ellipses

```php
tui_draw_ellipse($buffer, $centerX, $centerY, $radiusX, $radiusY, $style);
tui_fill_ellipse($buffer, $centerX, $centerY, $radiusX, $radiusY, $style);
```

### Triangles

```php
tui_draw_triangle($buffer, $x1, $y1, $x2, $y2, $x3, $y3, $style);
tui_fill_triangle($buffer, $x1, $y1, $x2, $y2, $x3, $y3, $style);
```

## Canvas

Canvas provides sub-cell resolution using Braille or block characters.

### Canvas Modes

| Mode | Resolution | Characters |
|------|------------|------------|
| `TUI_CANVAS_BRAILLE` | 2×4 dots per cell | Braille patterns (⠀⠁⠂...⣿) |
| `TUI_CANVAS_BLOCK` | 2×2 blocks per cell | Block elements (▀▄█ etc) |
| `TUI_CANVAS_ASCII` | 1×1 per cell | ASCII characters |

### Creating Canvas

```php
// Braille canvas: 40 cells × 20 cells = 80×80 pixels
$canvas = tui_canvas_create(40, 20, TUI_CANVAS_BRAILLE);

// Get actual pixel resolution
$resolution = tui_canvas_get_resolution($canvas);
// ['width' => 80, 'height' => 80]
```

### Canvas Drawing

```php
// Set individual pixels
tui_canvas_set($canvas, $x, $y);
tui_canvas_unset($canvas, $x, $y);
tui_canvas_toggle($canvas, $x, $y);

// Check pixel state
$isSet = tui_canvas_get($canvas, $x, $y);

// Clear entire canvas
tui_canvas_clear($canvas);
```

### Canvas Shapes

```php
// Line
tui_canvas_line($canvas, $x1, $y1, $x2, $y2);

// Rectangle
tui_canvas_rect($canvas, $x, $y, $width, $height);
tui_canvas_fill_rect($canvas, $x, $y, $width, $height);

// Circle
tui_canvas_circle($canvas, $centerX, $centerY, $radius);
tui_canvas_fill_circle($canvas, $centerX, $centerY, $radius);
```

### Canvas Colors

```php
// Set drawing color
tui_canvas_set_color($canvas, [255, 100, 100]);
```

### Rendering Canvas

```php
// Render to string array
$lines = tui_canvas_render($canvas, [
    'fg' => [100, 200, 255],
]);

// Display lines in buffer or component
foreach ($lines as $i => $line) {
    // Draw line at row $i
}
```

## Sprites

Animated sprite support for games:

```php
// Create sprite from frames
$sprite = tui_sprite_create([
    'animations' => [
        'idle' => [
            'frames' => [
                ['cells' => [['char' => '😊', 'fg' => [255, 255, 0]]]],
                ['cells' => [['char' => '😐', 'fg' => [255, 255, 0]]]],
            ],
            'speed' => 500,  // ms per frame
        ],
    ],
]);

// Set animation
tui_sprite_set_animation($sprite, 'idle');

// Update sprite (call each frame)
tui_sprite_update($sprite);

// Set position
tui_sprite_set_position($sprite, $x, $y);

// Render to buffer
tui_sprite_render($sprite, $buffer);

// Check collision
$collides = tui_sprite_collides($sprite1, $sprite2);
```

## Tables

Render data tables:

```php
// Create table with headers
$table = tui_table_create(['Name', 'Age', 'City']);

// Add rows
tui_table_add_row($table, ['Alice', '30', 'NYC']);
tui_table_add_row($table, ['Bob', '25', 'LA']);

// Set column alignment
tui_table_set_align($table, 1, TUI_ALIGN_RIGHT);

// Render to buffer
tui_table_render_to_buffer($table, $buffer, $x, $y, [
    'border' => true,
    'header_bold' => true,
    'header_color' => [255, 200, 0],
]);
```

## Progress Indicators

### Progress Bar

```php
$bar = tui_render_progress_bar(0.75, 40, [
    'filled' => '█',
    'empty' => '░',
    'fg' => [100, 255, 100],
]);
// Returns: "████████████████████████████████░░░░░░░░"
```

### Busy Bar

Indeterminate progress:

```php
$bar = tui_render_busy_bar($frame, 30, [
    'style' => 'pulse',
]);
```

### Spinners

```php
$char = tui_spinner_frame($frame, TUI_SPINNER_DOTS);
// Returns: one of ⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏

// Get frame count for a spinner type
$count = tui_spinner_frame_count(TUI_SPINNER_DOTS);  // 10

// Render spinner with label
$output = tui_render_spinner($frame, 'Loading...', TUI_SPINNER_DOTS, [
    'fg' => [100, 200, 255],
]);
```

Available spinner types:
- `TUI_SPINNER_DOTS` - Braille dots
- `TUI_SPINNER_LINE` - Rotating line
- `TUI_SPINNER_BOUNCE` - Bouncing bar
- `TUI_SPINNER_CIRCLE` - Rotating circle

## Example: Simple Game

```php
<?php
if (!tui_is_interactive()) {
    die("Requires interactive terminal\n");
}

[$width, $height] = tui_get_terminal_size();
$buffer = tui_buffer_create($width, $height);

$playerX = $width / 2;
$playerY = $height / 2;
$running = true;

$app = function () use ($buffer, &$playerX, &$playerY) {
    tui_buffer_clear($buffer);

    // Draw border
    tui_draw_rect($buffer, 0, 0, $width, $height, [
        'char' => '█',
        'fg' => [100, 100, 100],
    ]);

    // Draw player
    tui_fill_rect($buffer, (int)$playerX, (int)$playerY, 1, 1, [
        'char' => '@',
        'fg' => [100, 255, 100],
    ]);

    tui_buffer_render($buffer);

    return null;  // No component tree
};

$instance = tui_render($app);

tui_set_input_handler($instance, function (TuiKey $key) use ($instance, &$playerX, &$playerY, &$running) {
    if ($key->escape) {
        $running = false;
        tui_unmount($instance);
        return;
    }

    if ($key->upArrow) $playerY = max(1, $playerY - 1);
    if ($key->downArrow) $playerY++;
    if ($key->leftArrow) $playerX = max(1, $playerX - 1);
    if ($key->rightArrow) $playerX++;

    tui_rerender($instance);
});

tui_wait_until_exit($instance);
```

## See Also

- [Animation](animation.md) - Smooth transitions
- [Reference: Functions](../reference/functions.md) - Drawing functions
