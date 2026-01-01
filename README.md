<p align="center">
  <img src="docs/tui-logo.svg" alt="ext-tui logo" width="200">
</p>

# ext-tui

A PHP C extension for building terminal user interfaces with component-based architecture and Yoga flexbox layout.

## Features

- **Flexbox Layout**: Facebook's Yoga layout engine for CSS-like flexbox positioning
- **Component-Based**: Build UIs with `ContainerNode` and `ContentNode` components
- **Full UTF-8 Support**: Proper handling of Unicode text including CJK wide characters
- **Rich Text Styling**: Bold, italic, underline, colors (RGB), and more
- **Keyboard Input**: Full keyboard event handling with modifiers (Ctrl, Alt, Shift)
- **Mouse Support**: Click, scroll, and drag events with hit testing
- **Terminal Resize**: Automatic layout recalculation on terminal resize
- **Clipboard**: OSC 52 clipboard read/write (works over SSH)
- **Hyperlinks**: Clickable OSC 8 terminal hyperlinks
- **Bracketed Paste**: Detect and handle pasted text
- **Focus Management**: Tab navigation, focus groups, focus traps
- **Drawing Primitives**: Lines, rectangles, circles, ellipses, triangles
- **Canvas Graphics**: High-resolution drawing with Braille, block, or ASCII modes
- **Animation**: Easing functions, interpolation, color gradients
- **Sprites**: Frame-based animated sprites with collision detection
- **Tables**: Formatted table rendering with alignment and borders
- **Progress Indicators**: Progress bars, busy bars, and spinners
- **Timers**: Interval-based callbacks for animations and updates
- **High Performance**: All rendering done in C with minimal PHP overhead

## Requirements

- PHP 8.0+
- C compiler with C++20 support (for Yoga)
- Unix-like operating system (macOS, Linux)
- Terminal with UTF-8 support

## Installation

### From Source

```bash
# Clone the repository
git clone https://github.com/xocoder/ext-tui.git
cd ext-tui

# Build the extension
phpize
./configure --enable-tui
make

# Run tests
make test

# Install (requires root)
sudo make install

# Add to php.ini
echo "extension=tui.so" >> $(php -i | grep "Loaded Configuration File" | cut -d' ' -f5)
```

### Verify Installation

```bash
php -m | grep tui
# Output: tui

php -r "var_dump(tui_get_terminal_size());"
# Output: array(2) { [0]=> int(80) [1]=> int(24) }
```

## Quick Start

### Hello World

```php
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

$instance = tui_render(function() {
    $box = new ContainerNode(['padding' => 1, 'borderStyle' => 'round']);
    $box->addChild(new ContentNode(['content' => 'Hello, World!', 'bold' => true, 'color' => [100, 255, 100]]));
    return $box;
});

$instance->waitUntilExit();
```

### Interactive Counter

```php
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

$count = 0;

$instance = tui_render(function() use (&$count) {
    $box = new ContainerNode([
        'flexDirection' => 'column',
        'padding' => 2,
        'gap' => 1
    ]);

    $box->addChild(new ContentNode(['content' => "Counter: $count", 'bold' => true]));
    $box->addChild(new ContentNode(['content' => "Press UP/DOWN to change, Ctrl+C to exit", 'dim' => true]));

    return $box;
}, [
    'fullscreen' => true,
    'exitOnCtrlC' => true
]);

tui_set_input_handler($instance, function($key) use (&$count, $instance) {
    if ($key->upArrow) $count++;
    if ($key->downArrow) $count--;
    $instance->rerender();
});

$instance->waitUntilExit();
```

### Drawing with Canvas

```php
<?php
// Create a high-resolution canvas using Braille characters
$canvas = tui_canvas_create(80, 40, "braille");

// Draw shapes
tui_canvas_circle($canvas, 40, 20, 15);
tui_canvas_line($canvas, 0, 0, 80, 40);

// Render to strings
$lines = tui_canvas_render($canvas);
foreach ($lines as $line) {
    echo $line . "\n";
}
```

### Progress Bar with Buffer

```php
<?php
$buffer = tui_buffer_create(80, 24);

for ($i = 0; $i <= 100; $i += 5) {
    tui_buffer_clear($buffer);
    tui_render_progress_bar($buffer, 0, 0, 40, $i / 100);
    tui_buffer_render($buffer);
    usleep(100000);
}
```

## Documentation

Full documentation is available in the [`docs/`](docs/) folder:

### Manual (Tutorials & Guides)

- [Getting Started](docs/manual/getting-started.md) - Installation and first app
- [Components & Layout](docs/manual/components.md) - ContainerNode, ContentNode, flexbox layouts
- [Styling](docs/manual/styling.md) - Colors, text attributes, borders
- [Input Handling](docs/manual/input.md) - Keyboard events, focus, resize
- [Drawing](docs/manual/drawing.md) - Buffers, canvas, primitives, sprites, tables
- [Animation](docs/manual/animation.md) - Easing, interpolation, effects

### Reference

- [Functions](docs/reference/functions.md) - Complete function reference
- [Classes](docs/reference/classes.md) - ContainerNode, ContentNode, Key, Instance
- [Constants](docs/reference/constants.md) - TUI_EASE_*, TUI_CANVAS_*, etc.

### Specifications

- [ext-tui Specification](docs/specs/ext-tui-specs.md) - Complete C extension specification
- [xocdr/tui Specification](https://github.com/xocdr/tui/blob/main/docs/specs/xocdr-tui-specs.md) - PHP library specification

## API Overview

### Application Lifecycle

```php
tui_render(callable $component, array $options = []): TuiInstance
tui_rerender(TuiInstance $instance): void
tui_unmount(TuiInstance $instance): void
tui_wait_until_exit(TuiInstance $instance): void
```

### Event Handlers

```php
tui_set_input_handler(TuiInstance $instance, callable $handler): void
tui_set_focus_handler(TuiInstance $instance, callable $handler): void
tui_set_resize_handler(TuiInstance $instance, callable $handler): void
tui_set_tick_handler(TuiInstance $instance, callable $handler): void
```

### Focus Management

```php
tui_focus_next(TuiInstance $instance): void
tui_focus_prev(TuiInstance $instance): void
tui_get_focused_node(TuiInstance $instance): ?array
```

### Timers

```php
tui_add_timer(TuiInstance $instance, int $intervalMs, callable $callback): int
tui_remove_timer(TuiInstance $instance, int $timerId): void
```

### Terminal Info

```php
tui_get_terminal_size(): array           // [width, height]
tui_get_size(TuiInstance $instance): ?array
tui_is_interactive(): bool
tui_is_ci(): bool
```

### Text Utilities

```php
tui_string_width(string $text): int
tui_wrap_text(string $text, int $width): array
tui_truncate(string $text, int $width, string $ellipsis = "...", string $position = "end"): string
tui_pad(string $text, int $width, string $align = "l", string $pad_char = " "): string
```

### Drawing Buffers

```php
tui_buffer_create(int $width, int $height): resource
tui_buffer_clear(resource $buffer): void
tui_buffer_render(resource $buffer): void
```

### Drawing Primitives

```php
tui_draw_line(resource $buffer, int $x1, int $y1, int $x2, int $y2, array $style): void
tui_draw_rect(resource $buffer, int $x, int $y, int $w, int $h, array $style): void
tui_fill_rect(resource $buffer, int $x, int $y, int $w, int $h, array $style): void
tui_draw_circle(resource $buffer, int $cx, int $cy, int $r, array $style): void
tui_fill_circle(resource $buffer, int $cx, int $cy, int $r, array $style): void
tui_draw_ellipse(resource $buffer, int $cx, int $cy, int $rx, int $ry, array $style): void
tui_fill_ellipse(resource $buffer, int $cx, int $cy, int $rx, int $ry, array $style): void
tui_draw_triangle(resource $buffer, int $x1, int $y1, int $x2, int $y2, int $x3, int $y3, array $style): void
tui_fill_triangle(resource $buffer, int $x1, int $y1, int $x2, int $y2, int $x3, int $y3, array $style): void
```

### Canvas (High-Resolution Drawing)

```php
tui_canvas_create(int $width, int $height, string $mode = "braille"): resource
tui_canvas_set(resource $canvas, int $x, int $y): void
tui_canvas_unset(resource $canvas, int $x, int $y): void
tui_canvas_toggle(resource $canvas, int $x, int $y): void
tui_canvas_get(resource $canvas, int $x, int $y): bool
tui_canvas_clear(resource $canvas): void
tui_canvas_line(resource $canvas, int $x1, int $y1, int $x2, int $y2): void
tui_canvas_rect(resource $canvas, int $x, int $y, int $w, int $h): void
tui_canvas_fill_rect(resource $canvas, int $x, int $y, int $w, int $h): void
tui_canvas_circle(resource $canvas, int $cx, int $cy, int $r): void
tui_canvas_fill_circle(resource $canvas, int $cx, int $cy, int $r): void
tui_canvas_set_color(resource $canvas, int $r, int $g, int $b): void
tui_canvas_get_resolution(resource $canvas): array
tui_canvas_render(resource $canvas): array
```

### Animation

```php
tui_ease(float $t, string $easing = "linear"): float
tui_lerp(float $a, float $b, float $t): float
tui_lerp_color(array $a, array $b, float $t): string
tui_gradient(array $from, array $to, int $steps): array
tui_color_from_hex(string $hex): array
```

### Tables

```php
tui_table_create(array $headers): resource
tui_table_add_row(resource $table, array $cells): void
tui_table_set_align(resource $table, int $column, bool $right_align): void
tui_table_render_to_buffer(resource $buffer, resource $table, int $x, int $y, string $border = "single"): int
```

### Progress Indicators

```php
tui_render_progress_bar(resource $buffer, int $x, int $y, int $width, float $progress): void
tui_render_busy_bar(resource $buffer, int $x, int $y, int $width, int $frame, string $style_name = "pulse"): void
tui_spinner_frame(string $type, int $frame): string
tui_spinner_frame_count(string $type): int
tui_render_spinner(resource $buffer, int $x, int $y, string $type, int $frame): void
```

### Sprites

```php
tui_sprite_create(array $frames, string $name = "default", bool $loop = true): resource
tui_sprite_update(resource $sprite, int $delta_ms): void
tui_sprite_set_animation(resource $sprite, string $name): bool
tui_sprite_set_position(resource $sprite, int $x, int $y): void
tui_sprite_flip(resource $sprite, bool $flipped): void
tui_sprite_set_visible(resource $sprite, bool $visible): void
tui_sprite_render(resource $buffer, resource $sprite): void
tui_sprite_get_bounds(resource $sprite): array
tui_sprite_collides(resource $a, resource $b): bool
```

## Classes

All classes are in the `Xocdr\Tui\Ext` namespace.

### ContainerNode

Flexbox container component.

```php
use Xocdr\Tui\Ext\ContainerNode;

$box = new ContainerNode([
    'flexDirection' => 'column',     // 'row', 'column', 'row-reverse', 'column-reverse'
    'alignItems' => 'center',        // 'flex-start', 'center', 'flex-end', 'stretch'
    'justifyContent' => 'center',    // 'flex-start', 'center', 'flex-end', 'space-between', 'space-around', 'space-evenly'
    'width' => '100%',
    'height' => 10,
    'padding' => 1,
    'gap' => 1,
    'borderStyle' => 'round',        // 'single', 'double', 'round', 'bold'
    'borderColor' => [100, 150, 255],
    'focusable' => true,
]);
$box->addChild($child);
```

### ContentNode

Text display component.

```php
use Xocdr\Tui\Ext\ContentNode;

$text = new ContentNode([
    'content' => 'Hello!',
    'color' => '#00ff00',            // or [0, 255, 0]
    'backgroundColor' => [50, 50, 50],
    'bold' => true,
    'italic' => true,
    'underline' => true,
    'dim' => false,
    'inverse' => false,
    'strikethrough' => false,
]);
```

### Instance

Running TUI application handle.

```php
$instance->rerender();        // Force re-render
$instance->unmount();         // Stop and cleanup
$instance->waitUntilExit();   // Block until exit
$instance->exit(0);           // Request exit with code
```

### Key

Keyboard event object (passed to input handlers).

```php
$key->key           // string: character pressed
$key->upArrow       // bool
$key->downArrow     // bool
$key->leftArrow     // bool
$key->rightArrow    // bool
$key->return        // bool (enter key)
$key->escape        // bool
$key->backspace     // bool
$key->delete        // bool
$key->tab           // bool
$key->ctrl          // bool
$key->alt           // bool
$key->meta          // bool
$key->shift         // bool
```

## Constants

### Mouse Tracking Modes

`TUI_MOUSE_MODE_OFF`, `TUI_MOUSE_MODE_CLICK`, `TUI_MOUSE_MODE_BUTTON`, `TUI_MOUSE_MODE_ALL`

### String-Based Options

Many functions use string values instead of constants:

- **Easing:** `"linear"`, `"in_quad"`, `"out_quad"`, `"in_out_quad"`, `"in_cubic"`, `"out_cubic"`, etc.
- **Canvas modes:** `"braille"`, `"block"`, `"ascii"`
- **Spinner types:** `"dots"`, `"line"`, `"bounce"`, `"circle"`
- **Clipboard targets:** `"clipboard"`, `"primary"`, `"secondary"`

See [Constants Reference](docs/reference/constants.md) for complete list.

## Known Limitations

- **macOS/Linux only**: Uses POSIX APIs (termios, poll, signals)
- **No Windows support**: Consider WSL for Windows users

See [docs/limitations.md](docs/limitations.md) for full details.

## Related Projects

- **xocdr/tui** - PHP library with hooks wrapping ext-tui
- **xocdr/tui-widgets** - Pre-built widget components

## License

MIT License - see [LICENSE](LICENSE)

## Credits

- [Yoga](https://yogalayout.com/) - Facebook's flexbox layout engine
- Inspired by [Ink](https://github.com/vadimdemedes/ink) for Node.js
