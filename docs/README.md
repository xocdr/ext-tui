# ext-tui Documentation

A high-performance PHP extension for building terminal user interfaces with Unicode graphics, flexbox layouts, and smooth animations.

## Quick Navigation

### Getting Started
- [Installation](getting-started/installation.md) - System requirements and installation
- [Quick Start](getting-started/quickstart.md) - Your first TUI application

### Features
- [Terminal](features/terminal.md) - Terminal detection and information
- [Text Utilities](features/text.md) - Text wrapping, truncation, and measurement
- [Buffers & Rendering](features/buffers.md) - Screen buffers and output
- [Drawing Primitives](features/drawing.md) - Lines, boxes, and shapes
- [Canvas (High-Resolution)](features/canvas.md) - Braille and block graphics
- [Animation](features/animation.md) - Frame-based animations and easing
- [Tables](features/tables.md) - Formatted data tables
- [Progress & Spinners](features/progress.md) - Progress bars and loading indicators
- [Sprites](features/sprites.md) - Movable graphical elements
- [Event Handling](features/events.md) - Keyboard and resize events

### API Reference
- [Terminal Functions](api/terminal.md)
- [Text Functions](api/text.md)
- [Buffer Functions](api/buffers.md)
- [Drawing Functions](api/drawing.md)
- [Canvas Functions](api/canvas.md)
- [Animation Functions](api/animation.md)
- [Table Functions](api/tables.md)
- [Progress Functions](api/progress.md)
- [Sprite Functions](api/sprites.md)
- [Event Functions](api/events.md)
- [Classes](api/classes.md) - TuiBox, TuiText, TuiInstance

### Examples
- [Basic Examples](examples/basic.md) - Simple usage patterns
- [Drawing Demo](examples/drawing.md) - Shapes and graphics
- [Dashboard](examples/dashboard.md) - Full dashboard application
- [Animation Demo](examples/animation.md) - Animated graphics

## Features Overview

### High-Resolution Graphics
Draw at sub-character resolution using Braille patterns (2×4 dots per cell) or block characters (2×2 pixels per cell).

```php
$canvas = tui_canvas_create(40, 20, TUI_CANVAS_BRAILLE);
tui_canvas_circle($canvas, 40, 40, 30);
tui_canvas_line($canvas, 0, 0, 79, 79);
$lines = tui_canvas_render($canvas);
```

### Flexbox Layouts
Use Yoga-powered flexbox layouts for responsive terminal UIs.

```php
$root = new TuiBox();
$root->setFlexDirection('row');
$root->setPadding(1);

$sidebar = new TuiBox();
$sidebar->setWidth(20);
$root->appendChild($sidebar);

$content = new TuiBox();
$content->setFlexGrow(1);
$root->appendChild($content);
```

### Smooth Animations
Create fluid animations with 15+ easing functions.

```php
$anim = tui_animation_create(0, 100, 60, TUI_EASE_OUT_BOUNCE);
while (!tui_animation_done($anim)) {
    $value = tui_animation_value($anim);
    // Render at position $value
    tui_animation_step($anim);
}
```

### Unicode Support
Full Unicode support with correct width calculation for CJK characters, emoji, and combining marks.

```php
$width = tui_string_width("Hello 世界! 🎉");  // Returns 15
$wrapped = tui_wrap_text($text, 40, TUI_WRAP_WORD);
```

## Requirements

- PHP 8.0 or higher
- Unix-like system (Linux, macOS, BSD)
- Terminal with UTF-8 support
- Yoga library (for flexbox layouts)

## License

MIT License - See [LICENSE](../LICENSE) for details.
