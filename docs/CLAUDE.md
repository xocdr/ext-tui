# ext-tui - Claude AI Guide

Quick reference for using ext-tui in PHP projects.

## Installation

```bash
phpize && ./configure --enable-tui && make && sudo make install
echo "extension=tui.so" >> /path/to/php.ini
```

## Core Concepts

- **Xocdr\Tui\Ext\ContainerNode**: Container with flexbox layout (like `<div>`)
- **Xocdr\Tui\Ext\ContentNode**: Text with styling (like `<span>`)
- **tui_render()**: Starts TUI, returns Instance
- **Xocdr\Tui\Ext\Instance**: Controls running TUI (rerender, exit)
- **Xocdr\Tui\Ext\Key**: Keyboard event with key info

## Minimal Example

```php
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

$instance = tui_render(function() {
    $box = new ContainerNode(['padding' => 1]);
    $box->addChild(new ContentNode(['content' => "Hello!", 'bold' => true]));
    return $box;
});
$instance->waitUntilExit();
```

## ContainerNode Properties

| Property | Values | Description |
|----------|--------|-------------|
| `flexDirection` | `column`, `row`, `column-reverse`, `row-reverse` | Child flow |
| `alignItems` | `flex-start`, `center`, `flex-end`, `stretch` | Cross-axis align |
| `justifyContent` | `flex-start`, `center`, `flex-end`, `space-between`, `space-around`, `space-evenly` | Main-axis align |
| `flexGrow` | int | Grow factor |
| `flexShrink` | int | Shrink factor |
| `width` | int or `"100%"` | Width |
| `height` | int or `"100%"` | Height |
| `padding` | int | All-side padding |
| `paddingX/Y` | int | Horizontal/vertical padding |
| `margin` | int | All-side margin |
| `marginX/Y` | int | Horizontal/vertical margin |
| `gap` | int | Child spacing |
| `showCursor` | bool | Show cursor when focused |

## ContentNode Properties

| Property | Values | Description |
|----------|--------|-------------|
| `content` | string | Text content |
| `color` | `"#rrggbb"` or `[r,g,b]` | Foreground |
| `backgroundColor` | `"#rrggbb"` or `[r,g,b]` | Background |
| `bold` | bool | Bold |
| `dim` | bool | Dim |
| `italic` | bool | Italic |
| `underline` | bool | Underline |
| `inverse` | bool | Swap fg/bg |
| `strikethrough` | bool | Strikethrough |

## Functions

```php
tui_render(callable $component, array $options = []): Xocdr\Tui\Ext\Instance
tui_rerender(Xocdr\Tui\Ext\Instance $instance): void
tui_unmount(Xocdr\Tui\Ext\Instance $instance): void
tui_wait_until_exit(Xocdr\Tui\Ext\Instance $instance): void
tui_get_terminal_size(): [int $width, int $height]
tui_is_interactive(): bool
tui_is_ci(): bool
tui_string_width(string $text): int
tui_wrap_text(string $text, int $width): array
tui_truncate(string $text, int $width, string $ellipsis = '...'): string
```

## Instance Methods

```php
$instance->rerender();         // Re-render
$instance->unmount();          // Stop TUI
$instance->waitUntilExit();    // Block until exit
$instance->exit(int $code = 0); // Request exit
```

## Key Properties

```php
$key->key          // string: character pressed
$key->upArrow      // bool
$key->downArrow    // bool
$key->leftArrow    // bool
$key->rightArrow   // bool
$key->return       // bool (enter key)
$key->escape       // bool
$key->backspace    // bool
$key->delete       // bool
$key->tab          // bool
$key->ctrl         // bool
$key->meta         // bool
$key->shift        // bool
```

## Common Patterns

### Centered Layout

```php
use Xocdr\Tui\Ext\ContainerNode;

new ContainerNode([
    'width' => '100%',
    'height' => '100%',
    'alignItems' => 'center',
    'justifyContent' => 'center'
])
```

### Header + Content + Footer

```php
use Xocdr\Tui\Ext\ContainerNode;

$app = new ContainerNode(['height' => '100%']);
$app->addChild(new ContainerNode(['height' => 3]));      // header
$app->addChild(new ContainerNode(['flexGrow' => 1]));    // content
$app->addChild(new ContainerNode(['height' => 1]));      // footer
```

### Sidebar Layout

```php
use Xocdr\Tui\Ext\ContainerNode;

$row = new ContainerNode(['flexDirection' => 'row', 'height' => '100%']);
$row->addChild(new ContainerNode(['width' => 20]));      // sidebar
$row->addChild(new ContainerNode(['flexGrow' => 1]));    // main
```

### Styled Text

```php
use Xocdr\Tui\Ext\ContentNode;

new ContentNode([
    'content' => "Error!",
    'color' => '#ff0000',
    'bold' => true
])
```

### Using Color Enum

```php
use Xocdr\Tui\Ext\Color;

// Get color by name
Color::Red->value         // "#ff0000"
Color::Red->toRgb()       // ['r' => 255, 'g' => 0, 'b' => 0]
Color::Blue->toAnsi()     // ANSI escape for foreground
Color::Green->toAnsiBg()  // ANSI escape for background
Color::fromName("coral")  // Color::Coral
```

### Equal Columns

```php
use Xocdr\Tui\Ext\ContainerNode;

$row = new ContainerNode(['flexDirection' => 'row', 'gap' => 1]);
$row->addChild(new ContainerNode(['flexGrow' => 1]));
$row->addChild(new ContainerNode(['flexGrow' => 1]));
$row->addChild(new ContainerNode(['flexGrow' => 1]));
```

## Options for tui_render()

```php
[
    'fullscreen' => true,   // Use alternate screen
    'exitOnCtrlC' => true   // Auto-exit on Ctrl+C
]
```

## Notes

- Returns ContainerNode or ContentNode from component callback
- Use `addChild()` to build tree
- Call `rerender()` after state changes
- `waitUntilExit()` blocks until Ctrl+C or `exit()`
- Colors work on modern terminals with RGB support
