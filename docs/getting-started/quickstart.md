# Quick Start

[← Installation](installation.md) | [Back to Documentation](../README.md) | [Next: Terminal →](../features/terminal.md)

---

This guide will get you up and running with ext-tui in minutes.

## Hello, Terminal!

Let's start with a simple program that displays styled text:

```php
<?php
// Get terminal dimensions
[$width, $height] = tui_get_terminal_size();

// Create a buffer for the entire screen
$buf = tui_buffer_create($width, $height);

// Define a style
$style = [
    'fg' => [100, 200, 255],  // Light blue foreground
    'bg' => [30, 30, 30],     // Dark gray background
    'bold' => true
];

// Draw a box
tui_draw_box($buf, 2, 1, $width - 4, 5, $style);

// Add centered text
$message = "Welcome to ext-tui!";
$x = (int)(($width - strlen($message)) / 2);
tui_buffer_write($buf, $x, 3, $message, $style);

// Render to terminal
tui_render($buf);
```

## Basic Concepts

### 1. Buffers

All drawing happens on buffers, not directly to the screen. This enables double-buffering for flicker-free updates.

```php
// Create a buffer
$buf = tui_buffer_create(80, 24);

// Draw on the buffer
tui_buffer_write($buf, 0, 0, "Hello");

// Render to screen
tui_render($buf);

// Free when done
tui_buffer_free($buf);
```

### 2. Styles

Styles control colors and text attributes:

```php
$style = [
    'fg' => [255, 255, 255],   // RGB foreground color
    'bg' => [0, 0, 0],         // RGB background color
    'bold' => true,            // Bold text
    'italic' => true,          // Italic text
    'underline' => true,       // Underlined text
    'dim' => true,             // Dimmed text
    'blink' => true,           // Blinking text
    'reverse' => true,         // Swap fg/bg
];
```

### 3. Coordinates

Coordinates start at (0, 0) in the top-left corner:

```
(0,0) ─────────────────────→ x
  │
  │   (5,2) = column 5, row 2
  │
  ↓
  y
```

## Drawing Shapes

### Lines

```php
// Horizontal line
tui_draw_line($buf, 0, 5, 40, 5, '─', $style);

// Vertical line
tui_draw_line($buf, 20, 0, 20, 10, '│', $style);

// Diagonal line
tui_draw_line($buf, 0, 0, 20, 10, '╲', $style);
```

### Boxes

```php
// Simple box
tui_draw_box($buf, 5, 2, 30, 10, $style);

// Filled rectangle
tui_fill_rect($buf, 10, 5, 15, 5, '░', $style);
```

### Circles

```php
// Circle outline
tui_draw_circle($buf, 40, 12, 8, '*', $style);

// Filled circle
tui_fill_circle($buf, 40, 12, 8, '●', $style);
```

## High-Resolution Graphics

For smoother graphics, use the canvas with Braille or block characters:

```php
// Create canvas (each character cell = 2x4 Braille dots)
$canvas = tui_canvas_create(40, 20, TUI_CANVAS_BRAILLE);

// Draw at pixel level (80x80 virtual pixels)
tui_canvas_line($canvas, 0, 0, 79, 79);
tui_canvas_circle($canvas, 40, 40, 30);

// Render to strings
$lines = tui_canvas_render($canvas);

// Display
foreach ($lines as $i => $line) {
    tui_buffer_write($buf, 0, $i, $line, $style);
}

tui_canvas_free($canvas);
```

## Flexbox Layouts

Use TuiBox for responsive layouts:

```php
// Create root container
$root = new TuiBox();
$root->setWidth($width);
$root->setHeight($height);
$root->setFlexDirection('row');
$root->setPadding(1);

// Sidebar (fixed width)
$sidebar = new TuiBox();
$sidebar->setWidth(20);
$sidebar->setBorderWidth(1);
$root->appendChild($sidebar);

// Content area (flexible)
$content = new TuiBox();
$content->setFlexGrow(1);
$content->setBorderWidth(1);
$root->appendChild($content);

// Calculate layout
$root->calculateLayout();

// Get computed positions
$sidebarLayout = $sidebar->getComputedLayout();
// ['x' => 1, 'y' => 1, 'width' => 20, 'height' => 22]
```

## Animation

Create smooth animations with easing:

```php
// Animate from x=0 to x=60 over 30 frames
$anim = tui_animation_create(0, 60, 30, TUI_EASE_OUT_ELASTIC);

while (!tui_animation_done($anim)) {
    $x = (int)tui_animation_value($anim);

    tui_buffer_clear($buf);
    tui_buffer_write($buf, $x, 10, "●", $style);
    tui_render($buf);

    tui_animation_step($anim);
    usleep(33333);  // ~30 FPS
}

tui_animation_free($anim);
```

## Progress Indicators

```php
// Progress bar
for ($i = 0; $i <= 100; $i += 5) {
    tui_buffer_clear($buf);
    $bar = tui_progress_bar($i / 100, 40, $style);
    tui_buffer_write($buf, 5, 5, $bar, $style);
    tui_render($buf);
    usleep(100000);
}

// Spinner
$frame = 0;
while ($working) {
    $spinner = tui_spinner($frame++, TUI_SPINNER_DOTS);
    tui_buffer_write($buf, 5, 5, $spinner . " Loading...", $style);
    tui_render($buf);
    usleep(80000);
}
```

## Tables

```php
$headers = ['Name', 'Age', 'City'];
$rows = [
    ['Alice', '30', 'New York'],
    ['Bob', '25', 'London'],
    ['Charlie', '35', 'Tokyo'],
];

$table = tui_table_create($headers, $rows, [
    'border' => true,
    'header_style' => ['bold' => true, 'fg' => [255, 255, 0]],
]);

$lines = tui_table_render($table, 50);
foreach ($lines as $i => $line) {
    tui_buffer_write($buf, 2, 2 + $i, $line, $style);
}
```

## Event Loop

Handle keyboard and resize events:

```php
// Set up event handlers
tui_on_key(function($key) use ($buf) {
    if ($key === 'q') {
        tui_stop();
        return;
    }
    tui_buffer_write($buf, 0, 0, "Key: $key  ", []);
    tui_render($buf);
});

tui_on_resize(function($w, $h) {
    // Handle terminal resize
    global $buf;
    tui_buffer_free($buf);
    $buf = tui_buffer_create($w, $h);
});

// Enter raw mode and start event loop
tui_run();
```

## Complete Example

Here's a complete mini-application:

```php
<?php
// Check environment
if (!tui_is_interactive()) {
    die("This program requires an interactive terminal\n");
}

[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

$title_style = ['fg' => [255, 200, 0], 'bold' => true];
$box_style = ['fg' => [100, 150, 255]];
$text_style = ['fg' => [200, 200, 200]];

// Draw UI
tui_draw_box($buf, 0, 0, $width, $height, $box_style);
tui_buffer_write($buf, 2, 0, " My TUI App ", $title_style);

$message = "Press 'q' to quit";
tui_buffer_write($buf, 2, 2, $message, $text_style);

// Initial render
tui_render($buf);

// Event loop
tui_on_key(function($key) use (&$buf, $text_style) {
    if ($key === 'q') {
        tui_stop();
        return;
    }
    tui_buffer_write($buf, 2, 4, "Last key: $key    ", $text_style);
    tui_render($buf);
});

tui_run();

// Cleanup
tui_buffer_free($buf);
echo "Goodbye!\n";
```

## Next Steps

- [Terminal Functions](../features/terminal.md) - Learn about terminal detection
- [Drawing Primitives](../features/drawing.md) - Master shape drawing
- [Canvas Graphics](../features/canvas.md) - High-resolution graphics
- [Examples](../examples/basic.md) - More code examples

---

[← Installation](installation.md) | [Back to Documentation](../README.md) | [Next: Terminal →](../features/terminal.md)
