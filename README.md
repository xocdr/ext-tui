# ext-tui

A PHP extension for building terminal user interfaces with a React-like API and Yoga flexbox layout engine.

## Features

- **Flexbox Layout**: Uses Facebook's Yoga layout engine for CSS-like flexbox positioning
- **React-like Components**: Build UIs with `TuiBox` and `TuiText` components
- **Full UTF-8 Support**: Proper handling of Unicode text including CJK wide characters
- **Rich Text Styling**: Bold, italic, underline, colors (RGB), and more
- **Keyboard Input**: Full keyboard event handling with modifiers (Ctrl, Meta, Shift)
- **Terminal Resize**: Automatic layout recalculation on terminal resize
- **High Performance**: All rendering done in C with minimal PHP overhead

## Requirements

- PHP 8.0+
- C compiler with C++20 support (for Yoga)
- Unix-like operating system (macOS, Linux)

## Installation

### From Source

```bash
# Clone the repository
git clone https://github.com/your-org/ext-tui.git
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

$instance = tui_render(function() {
    $box = new TuiBox(['padding' => 1]);
    $box->addChild(new TuiText("Hello, World!", ['bold' => true, 'color' => '#00ff00']));
    return $box;
});

$instance->waitUntilExit();
```

### Interactive Counter

```php
<?php

$count = 0;

$instance = tui_render(function() use (&$count) {
    $box = new TuiBox([
        'flexDirection' => 'column',
        'padding' => 2,
        'gap' => 1
    ]);

    $box->addChild(new TuiText("Counter: $count", ['bold' => true]));
    $box->addChild(new TuiText("Press UP/DOWN to change, Ctrl+C to exit", ['dim' => true]));

    return $box;
}, [
    'fullscreen' => true,
    'exitOnCtrlC' => true
]);

// Handle input (in your event loop)
// $instance->waitUntilExit();
```

## API Reference

### Functions

#### `tui_render(callable $component, array $options = []): TuiInstance`

Renders a TUI component to the terminal.

**Options:**
- `fullscreen` (bool): Use alternate screen buffer (default: true)
- `exitOnCtrlC` (bool): Exit on Ctrl+C (default: true)

#### `tui_rerender(TuiInstance $instance): void`

Forces a re-render of the component tree.

#### `tui_unmount(TuiInstance $instance): void`

Stops the TUI and restores terminal state.

#### `tui_wait_until_exit(TuiInstance $instance): void`

Blocks until the TUI exits (via Ctrl+C or `$instance->exit()`).

#### `tui_get_terminal_size(): array`

Returns `[width, height]` of the terminal.

#### `tui_is_interactive(): bool`

Returns true if running in an interactive terminal.

#### `tui_is_ci(): bool`

Returns true if running in a CI environment.

#### `tui_string_width(string $text): int`

Returns the display width of text (handles Unicode correctly).

#### `tui_wrap_text(string $text, int $width): array`

Wraps text to fit within width, returns array of lines.

#### `tui_truncate(string $text, int $width, string $ellipsis = '...'): string`

Truncates text to width with ellipsis.

### Classes

#### TuiBox

Container component with flexbox layout.

**Constructor:**
```php
new TuiBox(array $props = [])
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `flexDirection` | string | `'column'` | `'row'`, `'column'`, `'row-reverse'`, `'column-reverse'` |
| `alignItems` | string | null | `'flex-start'`, `'center'`, `'flex-end'`, `'stretch'` |
| `justifyContent` | string | null | `'flex-start'`, `'center'`, `'flex-end'`, `'space-between'`, `'space-around'`, `'space-evenly'` |
| `flexGrow` | int | 0 | Flex grow factor |
| `flexShrink` | int | 1 | Flex shrink factor |
| `width` | int\|string | null | Width in cells or percentage (e.g., `'100%'`) |
| `height` | int\|string | null | Height in cells or percentage |
| `padding` | int | 0 | Padding on all sides |
| `paddingTop` | int | 0 | Top padding |
| `paddingBottom` | int | 0 | Bottom padding |
| `paddingLeft` | int | 0 | Left padding |
| `paddingRight` | int | 0 | Right padding |
| `paddingX` | int | 0 | Left and right padding |
| `paddingY` | int | 0 | Top and bottom padding |
| `margin` | int | 0 | Margin on all sides |
| `marginTop` | int | 0 | Top margin |
| `marginBottom` | int | 0 | Bottom margin |
| `marginLeft` | int | 0 | Left margin |
| `marginRight` | int | 0 | Right margin |
| `marginX` | int | 0 | Left and right margin |
| `marginY` | int | 0 | Top and bottom margin |
| `gap` | int | 0 | Gap between children |
| `borderStyle` | string | null | Border style (future) |
| `borderColor` | string | null | Border color (future) |
| `children` | array | [] | Child components |

**Methods:**
```php
addChild(TuiBox|TuiText $child): self
```

#### TuiText

Text component with styling.

**Constructor:**
```php
new TuiText(string $content = '', array $props = [])
```

**Properties:**
| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `content` | string | `''` | Text content |
| `color` | string | null | Foreground color (`'#RRGGBB'` or `[r, g, b]`) |
| `backgroundColor` | string | null | Background color |
| `bold` | bool | false | Bold text |
| `dim` | bool | false | Dim text |
| `italic` | bool | false | Italic text |
| `underline` | bool | false | Underlined text |
| `inverse` | bool | false | Inverted colors |
| `strikethrough` | bool | false | Strikethrough text |
| `wrap` | string | null | Text wrap mode (future) |

#### TuiInstance

Represents a running TUI application.

**Methods:**
```php
rerender(): void        // Force re-render
unmount(): void         // Stop and cleanup
waitUntilExit(): void   // Block until exit
exit(int $code = 0): void  // Request exit
```

#### TuiKey

Keyboard event object passed to input handlers.

**Properties:**
| Property | Type | Description |
|----------|------|-------------|
| `key` | string | The pressed key character |
| `upArrow` | bool | Up arrow pressed |
| `downArrow` | bool | Down arrow pressed |
| `leftArrow` | bool | Left arrow pressed |
| `rightArrow` | bool | Right arrow pressed |
| `return` | bool | Enter/Return pressed |
| `escape` | bool | Escape pressed |
| `backspace` | bool | Backspace pressed |
| `delete` | bool | Delete pressed |
| `tab` | bool | Tab pressed |
| `ctrl` | bool | Ctrl modifier held |
| `meta` | bool | Meta/Alt modifier held |
| `shift` | bool | Shift modifier held |

## Examples

See the `examples/` directory for more examples:

- `hello.php` - Basic hello world
- `counter.php` - Interactive counter with keyboard input
- `layout.php` - Flexbox layout demonstration
- `colors.php` - Color and style showcase

## Documentation

Full documentation is available in the `docs/` folder:

- [API Reference](docs/api.md)
- [Layout Guide](docs/layout.md)
- [Styling Guide](docs/styling.md)
- [Input Handling](docs/input.md)

## License

MIT License - see [LICENSE](LICENSE)

## Credits

- [Yoga](https://yogalayout.com/) - Facebook's flexbox layout engine
- Inspired by [Ink](https://github.com/vadimdemedes/ink) for Node.js
