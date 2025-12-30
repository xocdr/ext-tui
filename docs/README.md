# ext-tui Documentation

A PHP C extension for building terminal user interfaces with component-based architecture and Yoga flexbox layout.

## Quick Links

- [Getting Started](manual/getting-started.md) - Installation and first app
- [API Reference](reference/functions.md) - Complete function reference
- [Full Specification](specs/ext-tui-specs.md) - Complete technical specification

## Manual

User-friendly guides with examples:

| Guide | Description |
|-------|-------------|
| [Getting Started](manual/getting-started.md) | Installation, hello world, core concepts |
| [Components & Layout](manual/components.md) | Box, Text, flexbox layouts |
| [Styling](manual/styling.md) | Colors, text attributes, borders, hyperlinks |
| [Input Handling](manual/input.md) | Keyboard, mouse, focus, clipboard, history |
| [Drawing](manual/drawing.md) | Buffers, canvas, primitives, sprites, tables |
| [Animation](manual/animation.md) | Easing, interpolation, effects |

## Reference

Complete API documentation:

| Reference | Description |
|-----------|-------------|
| [Functions](reference/functions.md) | All functions with signatures |
| [Classes](reference/classes.md) | Box, Text, Key, Instance |
| [Constants](reference/constants.md) | TUI_EASE_*, TUI_CANVAS_*, etc. |

## Specifications

| Document | Description |
|----------|-------------|
| [ext-tui-specs.md](specs/ext-tui-specs.md) | Complete C extension specification |
| [xocdr/tui Specification](https://github.com/xocdr/tui/blob/main/docs/specs/xocdr-tui-specs.md) | PHP library specification (xocdr/tui) |

## Other

| Document | Description |
|----------|-------------|
| [limitations.md](limitations.md) | Known limitations and planned features |
| [architecture.md](architecture.md) | Internal architecture overview |
| [CLAUDE.md](CLAUDE.md) | AI assistant context |

## Quick Example

```php
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

if (!tui_is_interactive()) {
    die("Requires interactive terminal\n");
}

$app = function () {
    return new Box([
        'flexDirection' => 'column',
        'padding' => 1,
        'borderStyle' => 'round',
        'children' => [
            new Text([
                'content' => 'Hello, ext-tui!',
                'color' => [100, 200, 255],
                'bold' => true,
            ]),
        ],
    ]);
};

$instance = tui_render($app);
tui_wait_until_exit($instance);
```

## Requirements

- PHP 8.0+
- Unix-like system (Linux, macOS, BSD)
- Terminal with UTF-8 support
- Yoga is vendored (no external dependency)

## Related Projects

- **xocdr/tui** - PHP library with hooks wrapping ext-tui
- **xocdr/tui-widgets** - Pre-built widget components
