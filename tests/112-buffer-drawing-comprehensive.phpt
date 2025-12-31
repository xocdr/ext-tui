--TEST--
Buffer and drawing functions comprehensive testing
--EXTENSIONS--
tui
--FILE--
<?php
echo "=== tui_buffer_create() ===\n";
$buffer = tui_buffer_create(80, 24);
var_dump(is_resource($buffer) || is_object($buffer));

echo "\n=== tui_buffer_clear() ===\n";
tui_buffer_clear($buffer);
echo "Buffer cleared\n";

echo "\n=== Different buffer sizes ===\n";
$small = tui_buffer_create(10, 5);
echo "Created 10x5 buffer\n";

$large = tui_buffer_create(200, 50);
echo "Created 200x50 buffer\n";

echo "\n=== Drawing functions ===\n";

// Line drawing
tui_draw_line($buffer, 0, 0, 10, 10, '#');
echo "Drew diagonal line\n";

tui_draw_line($buffer, 0, 5, 20, 5, '-');
echo "Drew horizontal line\n";

tui_draw_line($buffer, 10, 0, 10, 10, '|');
echo "Drew vertical line\n";

echo "\n=== Rectangle drawing ===\n";
tui_draw_rect($buffer, 5, 5, 20, 10, '*');
echo "Drew rectangle outline\n";

tui_fill_rect($buffer, 30, 5, 10, 5, '.');
echo "Drew filled rectangle\n";

echo "\n=== Circle drawing ===\n";
tui_draw_circle($buffer, 40, 12, 5, 'o');
echo "Drew circle outline\n";

tui_fill_circle($buffer, 60, 12, 5, '@');
echo "Drew filled circle\n";

echo "\n=== Ellipse drawing ===\n";
tui_draw_ellipse($buffer, 20, 12, 8, 4, '~');
echo "Drew ellipse outline\n";

tui_fill_ellipse($buffer, 50, 12, 6, 3, '+');
echo "Drew filled ellipse\n";

echo "\n=== Triangle drawing ===\n";
tui_draw_triangle($buffer, 10, 20, 20, 10, 30, 20, '^');
echo "Drew triangle outline\n";

tui_fill_triangle($buffer, 40, 20, 50, 10, 60, 20, 'X');
echo "Drew filled triangle\n";

echo "\n=== tui_buffer_render() ===\n";
$output = tui_buffer_render($buffer);
echo "Rendered buffer, length: " . strlen($output) . "\n";

echo "\n=== Edge cases ===\n";

// Zero-size elements
tui_draw_rect($buffer, 0, 0, 0, 0, 'x');
echo "Drew zero-size rect\n";

// Negative coordinates (should be handled)
tui_draw_line($buffer, -5, -5, 5, 5, 'n');
echo "Drew with negative coords\n";

// Out of bounds
tui_draw_rect($buffer, 100, 100, 10, 10, 'o');
echo "Drew out of bounds\n";

// Single point
tui_draw_line($buffer, 5, 5, 5, 5, 'p');
echo "Drew point\n";

echo "\n=== Canvas functions ===\n";
$canvas = tui_canvas_create(40, 20);
echo "Created canvas 40x20\n";

tui_canvas_set($canvas, 5, 5);
echo "Set pixel\n";

$state = tui_canvas_get($canvas, 5, 5);
echo "Get pixel: " . ($state ? 'set' : 'unset') . "\n";

tui_canvas_toggle($canvas, 5, 5);
echo "Toggled pixel\n";

$state = tui_canvas_get($canvas, 5, 5);
echo "Get pixel after toggle: " . ($state ? 'set' : 'unset') . "\n";

tui_canvas_unset($canvas, 10, 10);
echo "Unset pixel\n";

tui_canvas_clear($canvas);
echo "Cleared canvas\n";

echo "\nDone!\n";
?>
--EXPECTF--
=== tui_buffer_create() ===
bool(true)

=== tui_buffer_clear() ===
Buffer cleared

=== Different buffer sizes ===
Created 10x5 buffer
Created 200x50 buffer

=== Drawing functions ===
Drew diagonal line
Drew horizontal line
Drew vertical line

=== Rectangle drawing ===
Drew rectangle outline
Drew filled rectangle

=== Circle drawing ===
Drew circle outline
Drew filled circle

=== Ellipse drawing ===
Drew ellipse outline
Drew filled ellipse

=== Triangle drawing ===
Drew triangle outline
Drew filled triangle

=== tui_buffer_render() ===
Rendered buffer, length: %d

=== Edge cases ===
Drew zero-size rect
Drew with negative coords
Drew out of bounds
Drew point

=== Canvas functions ===
Created canvas 40x20
Set pixel
Get pixel: set
Toggled pixel
Get pixel after toggle: unset
Unset pixel
Cleared canvas

Done!
