# Getting Started with xocdr/ext-tui

This guide will help you install ext-tui and create your first terminal UI application.

## System Requirements

- **PHP**: 8.0 or higher
- **OS**: Linux, macOS, or BSD (Windows not supported)
- **Terminal**: UTF-8 capable terminal emulator
- **Build tools**: C compiler (gcc/clang), make, CMake 3.13+

> **Note**: The Yoga layout library is vendored within ext-tui. No separate installation required.

## Installation

### From Source

```bash
# Clone the repository
git clone https://github.com/xocdr/ext-tui.git
cd ext-tui

# Build the extension
phpize
./configure
make

# Run tests (optional)
make test

# Install system-wide
sudo make install

# Enable in PHP
echo "extension=tui.so" >> $(php -r "echo php_ini_loaded_file();")
```

### Verify Installation

```bash
php -m | grep tui
# Output: tui

php -r "var_dump(tui_get_terminal_size());"
# Output: array(2) { [0]=> int(80) [1]=> int(24) }
```

## Hello World

Create a simple TUI application:

```php
<?php
if (!tui_is_interactive()) {
    die("Requires an interactive terminal\n");
}

$app = function () {
    return new TuiBox([
        'flexDirection' => 'column',
        'padding' => 1,
        'borderStyle' => 'round',
        'children' => [
            new TuiText([
                'content' => 'Hello, ext-tui!',
                'color' => [100, 200, 255],
                'bold' => true,
            ]),
            new TuiText([
                'content' => 'Press Ctrl+C to exit.',
                'dim' => true,
            ]),
        ],
    ]);
};

$instance = tui_render($app);
tui_wait_until_exit($instance);
```

Run it:

```bash
php hello.php
```

## Interactive Counter

Add keyboard input handling:

```php
<?php
if (!tui_is_interactive()) {
    die("Requires an interactive terminal\n");
}

$counter = 0;

$app = function () use (&$counter) {
    return new TuiBox([
        'flexDirection' => 'column',
        'padding' => 1,
        'borderStyle' => 'single',
        'children' => [
            new TuiText([
                'content' => "Count: $counter",
                'color' => [100, 255, 100],
                'bold' => true,
            ]),
            new TuiText([
                'content' => 'UP/DOWN to change, ESC to exit',
                'dim' => true,
            ]),
        ],
    ]);
};

$instance = tui_render($app);

tui_set_input_handler($instance, function (TuiKey $key) use ($instance, &$counter) {
    if ($key->escape) {
        tui_unmount($instance);
        return;
    }

    if ($key->upArrow) {
        $counter++;
        tui_rerender($instance);
    } elseif ($key->downArrow) {
        $counter = max(0, $counter - 1);
        tui_rerender($instance);
    }
});

tui_wait_until_exit($instance);
echo "Final count: $counter\n";
```

## Core Concepts

### Components

All UIs are built with two component types:

- **TuiBox** - Container with flexbox layout
- **TuiText** - Styled text content

Components are created with a properties array:

```php
$box = new TuiBox([
    'width' => 40,
    'height' => 10,
    'padding' => 1,
    'borderStyle' => 'single',
]);

$text = new TuiText([
    'content' => 'Hello',
    'bold' => true,
    'color' => [255, 200, 0],
]);
```

### Application Lifecycle

1. **Define component** - Create a function that returns a component tree
2. **Render** - Call `tui_render($app)` to start the TUI
3. **Handle input** - Register handlers with `tui_set_input_handler()`
4. **Update state** - Modify state and call `tui_rerender($instance)`
5. **Exit** - Call `tui_unmount($instance)` or let Ctrl+C exit

### The TuiInstance

`tui_render()` returns a `TuiInstance` that you pass to other functions:

```php
$instance = tui_render($app);

tui_set_input_handler($instance, $handler);
tui_set_resize_handler($instance, $resizeHandler);
tui_rerender($instance);
tui_unmount($instance);
tui_wait_until_exit($instance);
```

## Using xocdr/tui (PHP Library)

For a higher-level API with hooks and advanced widgets on top of ext-tui, use the [xocdr/tui](https://github.com/xocdr/tui) PHP library:

```php
<?php
require 'vendor/autoload.php';

use Tui\Components\Box;
use Tui\Components\Text;
use Tui\Tui;
use function Tui\Hooks\useState;
use function Tui\Hooks\useInput;
use function Tui\Hooks\useApp;

$app = function () {
    [$count, $setCount] = useState(0);
    ['exit' => $exit] = useApp();

    useInput(function ($input, $key) use ($setCount, $exit) {
        if ($key->upArrow) $setCount(fn($n) => $n + 1);
        if ($key->escape) $exit();
    });

    return Box::column([
        Text::create("Count: $count")->bold(),
        Text::create("UP to increment, ESC to exit")->dim(),
    ]);
};

Tui::render($app)->waitUntilExit();
```

See the [xocdr/tui specification](../specs/xocdr-tui-specs.md) for complete documentation.

## Next Steps

- [Components & Layout](components.md) - Flexbox layouts
- [Styling](styling.md) - Colors and text attributes
- [Input Handling](input.md) - Keyboard events
- [Drawing](drawing.md) - Canvas and buffer graphics
- [Animation](animation.md) - Smooth transitions
