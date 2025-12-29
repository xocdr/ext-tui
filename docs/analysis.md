# Tui: Terminal UI Extension for PHP

🐦 A native PHP extension that brings Ink-like terminal UI capabilities to PHP, with an embedded Yoga layout engine and React-inspired component model.

*Named after the [Tui](https://en.wikipedia.org/wiki/Tui_(bird)) - a New Zealand bird known for its beautiful, complex songs.*

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Installation](#installation)
4. [API Reference](#api-reference)
5. [Components](#components)
6. [Hooks](#hooks)
7. [Context System](#context-system)
8. [Styling](#styling)
9. [Layout System](#layout-system)
10. [Input Handling](#input-handling)
11. [Reconciler](#reconciler)
12. [Rendering Pipeline](#rendering-pipeline)
13. [Implementation Details](#implementation-details)
14. [Build System](#build-system)
15. [Testing](#testing)
16. [Roadmap](#roadmap)

---

## Overview

### What is Tui?

Tui is a native PHP extension (written in C) that provides a terminal-based UI framework inspired by [Ink](https://github.com/vadimdemedes/ink). It allows developers to build interactive command-line applications using a declarative, component-based approach.

### Package Structure

| Package | Type | Description |
|---------|------|-------------|
| `ext-tui` | C Extension | Core engine (Yoga, reconciler, terminal I/O) |
| `tui/tui` | Composer | Components, hooks, fluent API |
| `tui/widgets` | Composer | Spinner, ProgressBar, Select, Table, etc. |

### Goals

- **Full Ink feature parity** - All components, hooks, and styling options
- **Native performance** - C extension with embedded Yoga layout engine
- **PHP-native API** - Idiomatic PHP without requiring React/JSX knowledge
- **Layered architecture** - C extension + Composer package for DX
- **Cross-platform** - Linux, macOS, Windows (via WSL)

### Why a C Extension?

| Approach | Performance | Event Loop | Yoga Integration | Complexity |
|----------|-------------|------------|------------------|------------|
| Pure PHP + ReactPHP | Slow | PHP userland | FFI overhead | Medium |
| PHP FFI + Yoga | Medium | PHP userland | External lib | Medium |
| **C Extension** | **Fastest** | **Native** | **Embedded** | High |

Benefits:
- No FFI overhead for Yoga layout calculations
- Native `select()`/`poll()`/`epoll()` event loop
- Direct terminal control via termios
- Zero garbage collection pauses during render
- Single binary, no external dependencies

---

## Architecture

### High-Level Overview

```
┌─────────────────────────────────────────────────────────────┐
│                      PHP Application                         │
│  ┌─────────────────────────────────────────────────────┐    │
│  │  Tui::render(fn() => Box::create()->child(...))     │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                tui/tui (Composer Package)                    │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
│  │  Components  │  │    Hooks     │  │  Fluent API  │       │
│  │  Box, Text   │  │  useState    │  │  Builder     │       │
│  └──────────────┘  └──────────────┘  └──────────────┘       │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                   ext-tui (C Extension)                      │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
│  │  PHP Class   │  │   Reconciler │  │  Event Loop  │       │
│  │   Bindings   │  │   (Diffing)  │  │  (poll/epoll)│       │
│  └──────────────┘  └──────────────┘  └──────────────┘       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
│  │    Yoga      │  │   Terminal   │  │    ANSI      │       │
│  │   (Layout)   │  │   (termios)  │  │  (Renderer)  │       │
│  └──────────────┘  └──────────────┘  └──────────────┘       │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      Terminal (stdout)                       │
└─────────────────────────────────────────────────────────────┘
```

### Directory Structure

```
tui/
├── ext/                      # C Extension (ext-tui)
│   ├── config.m4             # PHP extension build config
│   ├── config.w32            # Windows build config
│   ├── php_tui.h             # Main extension header
│   ├── php_tui.c             # Extension entry point
│   └── src/
│       ├── app.c             # App class implementation
│       ├── app.h
│       ├── components/
│       │   ├── box.c         # Box component
│   │   ├── box.h
│   │   ├── text.c            # Text component
│   │   ├── text.h
│   │   ├── newline.c
│   │   ├── spacer.c
│   │   ├── static.c
│   │   └── transform.c
│   ├── hooks/
│   │   ├── state.c           # useState implementation
│   │   ├── input.c           # useInput implementation
│   │   ├── focus.c           # useFocus implementation
│   │   └── hooks.h
│   ├── reconciler/
│   │   ├── reconciler.c      # Tree diffing algorithm
│   │   ├── reconciler.h
│   │   ├── node.c            # Virtual node management
│   │   └── node.h
│   ├── layout/
│   │   ├── yoga_wrapper.c    # Yoga integration
│   │   ├── yoga_wrapper.h
│   │   └── styles.c          # Style application
│   ├── render/
│   │   ├── renderer.c        # Main render loop
│   │   ├── renderer.h
│   │   ├── ansi.c            # ANSI escape sequences
│   │   ├── ansi.h
│   │   ├── output.c          # Output diffing
│   │   └── border.c          # Border rendering
│   ├── terminal/
│   │   ├── terminal.c        # Terminal control
│   │   ├── terminal.h
│   │   ├── input.c           # Input parsing
│   │   └── input.h
│   ├── event_loop/
│   │   ├── loop.c            # Main event loop
│   │   ├── loop.h
│   │   └── signals.c         # Signal handling
│   └── util/
│       ├── unicode.c         # Unicode width (wcwidth)
│       ├── hash.c            # Fast hashing
│       └── memory.c          # Memory pool
├── yoga/                     # Embedded Yoga source
│   ├── yoga/
│   │   ├── Yoga.h
│   │   ├── Yoga.cpp
│   │   └── ...
│   └── CMakeLists.txt
├── tests/
│   ├── 001-basic.phpt
│   ├── 002-box.phpt
│   └── ...
└── examples/
    ├── counter.php
    ├── todo.php
    └── ...
```

### Core Data Structures

```c
// Virtual DOM node
typedef struct ink_node {
    uint32_t id;                    // Unique node ID
    ink_node_type type;             // INK_BOX, INK_TEXT, etc.

    // Properties
    ink_style style;                // Flexbox + visual styles
    char *text;                     // For text nodes
    zval attributes;                // Other props

    // Tree structure
    struct ink_node *parent;
    struct ink_node *first_child;
    struct ink_node *last_child;
    struct ink_node *next_sibling;
    struct ink_node *prev_sibling;

    // Yoga
    YGNodeRef yoga_node;

    // Rendering
    uint32_t props_hash;            // For fast diffing
    uint8_t dirty;                  // Needs re-layout

    // Special flags
    uint8_t is_static;              // <Static> component
    zval transform_fn;              // <Transform> callback
} ink_node;

// Application state
typedef struct ink_app {
    // Node tree
    ink_node *root;
    ink_node *static_node;

    // Rendering
    zval render_fn;                 // PHP render callback
    char **output_buffer;           // Current output lines
    int output_lines;
    int output_cols;
    uint8_t render_scheduled;

    // Terminal
    struct termios orig_termios;
    int is_raw_mode;
    int is_alternate_screen;

    // Event handling
    ink_handler *input_handlers;
    ink_handler *key_handlers;      // Specific key bindings
    int handler_count;

    // State management
    ink_state *states;
    int state_count;
    int state_index;                // Current hook index

    // Focus
    ink_node *focused_node;
    ink_node **focus_order;
    int focus_count;

    // Control
    uint8_t running;
    uint8_t exit_requested;
    uint8_t resize_pending;
    int exit_code;
} ink_app;

// Style properties
typedef struct ink_style {
    // Flexbox (Yoga)
    YGFlexDirection flex_direction;
    YGWrap flex_wrap;
    YGJustify justify_content;
    YGAlign align_items;
    YGAlign align_self;
    float flex_grow;
    float flex_shrink;
    float flex_basis;

    // Dimensions
    YGValue width;
    YGValue height;
    YGValue min_width;
    YGValue max_width;
    YGValue min_height;
    YGValue max_height;

    // Spacing
    YGValue padding[4];             // top, right, bottom, left
    YGValue margin[4];
    float gap;                      // shorthand for column_gap + row_gap
    float column_gap;
    float row_gap;

    // Position
    YGPositionType position;
    YGValue inset[4];               // top, right, bottom, left

    // Visual
    ink_color fg_color;
    ink_color bg_color;
    uint8_t bold;
    uint8_t italic;
    uint8_t underline;
    uint8_t dim_color;              // dimColor in API
    uint8_t inverse;
    uint8_t strikethrough;

    // Border
    ink_border_style border_style;
    ink_color border_color;
    ink_color border_colors[4];     // top, right, bottom, left (individual colors)
    uint8_t border_sides[4];        // top, right, bottom, left (visibility)
    uint8_t border_dim[4];          // top, right, bottom, left (dim flags)
    uint8_t border_dim_color;       // shorthand for all sides

    // Text
    ink_wrap_mode text_wrap;        // textWrap in styles.d.ts
    ink_overflow overflow;
    ink_overflow overflow_x;
    ink_overflow overflow_y;
} ink_style;
```

---

## Installation

### From Source

```bash
# Clone repository
git clone https://github.com/tui-php/tui.git
cd tui/ext

# Build
phpize
./configure --enable-tui
make
make test

# Install
sudo make install

# Enable extension
echo "extension=tui.so" | sudo tee /etc/php/conf.d/tui.ini
```

### Requirements

- PHP 8.1+
- C compiler (gcc/clang)
- PHP development headers (`php-dev` / `php-devel`)
- Linux or macOS (Windows via WSL)

---

## API Reference

### Ink\render()

The main entry point - a function that mounts a component and starts the render loop.

```php
namespace Ink;

/**
 * Mount a component and render output to the terminal.
 *
 * @param Node $node The root node to render
 * @param array $options {
 *     @type bool $fullscreen Use alternate screen buffer (default: false)
 *     @type resource $stdin Custom stdin stream (default: STDIN)
 *     @type resource $stdout Custom stdout stream (default: STDOUT)
 *     @type resource $stderr Custom stderr stream (default: STDERR)
 *     @type bool $exitOnCtrlC Exit on Ctrl+C (default: true)
 *     @type bool $debug Disable render throttling (default: false)
 *     @type bool $patchConsole Intercept echo/print output (default: true)
 * }
 * @return Instance Control object for the rendered instance
 */
function render(Node $node, array $options = []): Instance;
```

### Ink\Instance

Control object returned by `render()` for managing the rendered application.

```php
namespace Ink;

class Instance {
    /**
     * Re-render with a new component tree.
     */
    public function rerender(Node $node): void;

    /**
     * Unmount the component tree and restore terminal.
     */
    public function unmount(): void;

    /**
     * Block until the app exits (via exit() or Ctrl+C).
     */
    public function waitUntilExit(): void;

    /**
     * Clear current output from terminal.
     */
    public function clear(): void;

    /**
     * Cleanup instance from internal registry.
     */
    public function cleanup(): void;
}
```

### Instance Caching

Tui caches instances per stdout stream, so consecutive `render()` calls reuse the
same instance rather than creating new ones:

```php
// Both calls use the same instance
$instance1 = render(fn($app) => Box::create()->child(Text::create('Hello')));
$instance2 = render(fn($app) => Box::create()->child(Text::create('World')));

// $instance1 === $instance2 (same instance)
// Use cleanup() to remove from cache and allow new instance
$instance1->cleanup();
```

### CI Detection

Tui detects CI environments and adjusts behavior:

```php
// Automatic detection via environment variables
// CI=true, GITHUB_ACTIONS, TRAVIS, CIRCLECI, GITLAB_CI, etc.

// In CI mode:
// - No ANSI escape codes for clearing (CI logs don't handle well)
// - Static output written immediately
// - Final output written on exit
// - No throttling
```

### Cursor Control

The cursor is automatically hidden during render and restored on exit:

```php
// Automatic behavior
render(function($app) {
    // Cursor is hidden here
    return Box::create()->child(Text::create('Working...'));
});
// Cursor restored on unmount

// Manual control via render options
render($node, [
    'showCursor' => true,  // Keep cursor visible
]);
```

### Ink\App (Component Context)

Unlike React's Ink where hooks are called within component functions, Tui provides
an `App` context object that is passed to render callbacks for state management.

```php
namespace Ink;

class App {
    /**
     * Request application exit.
     *
     * @param \Throwable|null $error Optional error to trigger
     */
    public function exit(?\Throwable $error = null): void;

    /**
     * Create a state variable.
     * Returns [currentValue, setterFunction].
     *
     * @param mixed $initial Initial value
     * @return array{0: mixed, 1: callable}
     */
    public function useState(mixed $initial): array;

    /**
     * Register an input handler.
     * Called when user presses keys.
     *
     * @param callable $handler fn(string $input, Key $key)
     * @param array $options { @type bool $isActive Whether handler is active (default: true) }
     */
    public function useInput(callable $handler, array $options = []): void;

    /**
     * Make this component focusable.
     *
     * @param array $options {
     *     @type bool $autoFocus Auto-focus on mount (default: false)
     *     @type bool $isActive Whether focusable (default: true)
     *     @type string $id Custom focus ID
     * }
     * @return Focus Focus context with isFocused property
     */
    public function useFocus(array $options = []): Focus;

    /**
     * Get the focus manager for programmatic focus control.
     */
    public function useFocusManager(): FocusManager;

    /**
     * Access stdin stream and raw mode control.
     */
    public function useStdin(): StdinContext;

    /**
     * Access stdout stream and terminal dimensions.
     */
    public function useStdout(): StdoutContext;

    /**
     * Access stderr stream.
     */
    public function useStderr(): StderrContext;

    /**
     * Force an immediate re-render.
     */
    public function rerender(): void;

    /**
     * Get terminal dimensions.
     *
     * @return array{width: int, height: int}
     */
    public function getTerminalSize(): array;

    /**
     * Measure a node's computed layout dimensions.
     *
     * @param Node $node
     * @return array{width: int, height: int}
     */
    public function measureElement(Node $node): array;
}
```

### Usage Pattern

```php
use function Ink\render;
use Ink\{Box, Text};

// The render callback receives the App context
$instance = render(function(Ink\App $app) {
    // State management
    [$count, $setCount] = $app->useState(0);

    // Input handling
    $app->useInput(function($input, $key) use ($setCount, $app) {
        if ($input === 'q') {
            $app->exit();
        }
        if ($key->upArrow) {
            $setCount(fn($c) => $c + 1);
        }
    });

    // Return the UI tree
    return Box::create(['padding' => 1])
        ->child(Text::create("Count: {$count}"));
});

// Control the running app
$instance->waitUntilExit();
```

### Ink\Node

Base class for all renderable nodes.

```php
namespace Ink;

abstract class Node {
    /**
     * Create a new node with the given style.
     */
    public static function create(array $style = []): static;

    /**
     * Add a child node.
     */
    public function child(Node|string $child): static;

    /**
     * Add multiple children.
     */
    public function children(array $children): static;

    /**
     * Set a style property.
     */
    public function style(string $property, mixed $value): static;

    /**
     * Merge multiple styles.
     */
    public function styles(array $styles): static;

    /**
     * Set a unique key for reconciliation.
     */
    public function key(string|int $key): static;

    /**
     * Set a ref for element access.
     */
    public function ref(callable $callback): static;
}
```

---

## Components

### Box

Container component with flexbox layout.

```php
use Ink\Box;

// Basic box
Box::create();

// With styles
Box::create([
    'flexDirection' => 'column',
    'padding' => 1,
    'borderStyle' => 'round',
    'borderColor' => 'cyan',
]);

// With children
Box::create(['padding' => 1])
    ->child(Text::create('Hello'))
    ->child(Text::create('World'));

// Fluent style API
Box::create()
    ->style('flexDirection', 'row')
    ->style('gap', 2)
    ->children([
        Text::create('Left'),
        Spacer::create(),
        Text::create('Right'),
    ]);
```

**Box Style Properties:**

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `flexDirection` | `'row'` \| `'column'` \| `'row-reverse'` \| `'column-reverse'` | `'row'` | Main axis direction |
| `flexWrap` | `'nowrap'` \| `'wrap'` \| `'wrap-reverse'` | `'nowrap'` | Whether flex items wrap |
| `alignItems` | `'flex-start'` \| `'center'` \| `'flex-end'` \| `'stretch'` | | Cross-axis alignment |
| `alignSelf` | Same as alignItems + `'auto'` | | Override parent's alignItems |
| `justifyContent` | `'flex-start'` \| `'center'` \| `'flex-end'` \| `'space-between'` \| `'space-around'` \| `'space-evenly'` | | Main axis distribution |
| `flexGrow` | `float` | `0` | Grow factor |
| `flexShrink` | `float` | `1` | Shrink factor |
| `flexBasis` | `int` \| `string` | | Base size |
| `width` | `int` \| `string` | | Width (number or percentage) |
| `height` | `int` \| `string` | | Height (number or percentage) |
| `minWidth` | `int` \| `string` | | Minimum width |
| `maxWidth` | `int` \| `string` | | Maximum width |
| `minHeight` | `int` \| `string` | | Minimum height |
| `maxHeight` | `int` \| `string` | | Maximum height |
| `padding` | `int` | | All sides padding |
| `paddingTop` | `int` | | Top padding |
| `paddingRight` | `int` | | Right padding |
| `paddingBottom` | `int` | | Bottom padding |
| `paddingLeft` | `int` | | Left padding |
| `paddingX` | `int` | | Horizontal padding |
| `paddingY` | `int` | | Vertical padding |
| `margin` | `int` | | All sides margin |
| `marginTop` | `int` | | Top margin |
| `marginRight` | `int` | | Right margin |
| `marginBottom` | `int` | | Bottom margin |
| `marginLeft` | `int` | | Left margin |
| `marginX` | `int` | | Horizontal margin |
| `marginY` | `int` | | Vertical margin |
| `gap` | `int` | | Gap between children (shorthand for columnGap + rowGap) |
| `columnGap` | `int` | | Gap between columns |
| `rowGap` | `int` | | Gap between rows |
| `position` | `'relative'` \| `'absolute'` | `'relative'` | Position type |
| `borderStyle` | `'single'` \| `'double'` \| `'round'` \| `'bold'` \| `'singleDouble'` \| `'doubleSingle'` \| `'classic'` \| `BoxStyle` | | Border style |
| `borderColor` | `string` | | Border color (all sides) |
| `borderTopColor` | `string` | | Top border color |
| `borderBottomColor` | `string` | | Bottom border color |
| `borderLeftColor` | `string` | | Left border color |
| `borderRightColor` | `string` | | Right border color |
| `borderDimColor` | `bool` | `false` | Dim all border colors |
| `borderTopDimColor` | `bool` | `false` | Dim top border color |
| `borderBottomDimColor` | `bool` | `false` | Dim bottom border color |
| `borderLeftDimColor` | `bool` | `false` | Dim left border color |
| `borderRightDimColor` | `bool` | `false` | Dim right border color |
| `borderTop` | `bool` | `true` | Show top border |
| `borderRight` | `bool` | `true` | Show right border |
| `borderBottom` | `bool` | `true` | Show bottom border |
| `borderLeft` | `bool` | `true` | Show left border |
| `overflow` | `'visible'` \| `'hidden'` | `'visible'` | Overflow behavior (both axes) |
| `overflowX` | `'visible'` \| `'hidden'` | `'visible'` | Horizontal overflow |
| `overflowY` | `'visible'` \| `'hidden'` | `'visible'` | Vertical overflow |
| `display` | `'flex'` \| `'none'` | `'flex'` | Display mode |

### Text

Text content with styling.

```php
use Ink\Text;

// Plain text
Text::create('Hello World');

// Styled text
Text::create('Error!', [
    'color' => 'red',
    'bold' => true,
]);

// Background color
Text::create('Highlight', [
    'backgroundColor' => 'yellow',
    'color' => 'black',
]);

// Nested text with different styles
Text::create()
    ->child('Normal ')
    ->child(Text::create('Bold', ['bold' => true]))
    ->child(' Normal');

// Wrapping
Text::create($longText, [
    'wrap' => 'wrap',        // or 'truncate', 'truncate-start', 'truncate-middle'
]);
```

**Text Style Properties:**

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `color` | `string` | | Foreground color |
| `backgroundColor` | `string` | | Background color |
| `bold` | `bool` | `false` | Bold text |
| `italic` | `bool` | `false` | Italic text |
| `underline` | `bool` | `false` | Underlined text |
| `strikethrough` | `bool` | `false` | Strikethrough text |
| `dimColor` | `bool` | `false` | Dimmed text (applies chalk.dim) |
| `inverse` | `bool` | `false` | Inverse colors |
| `wrap` | `'wrap'` \| `'truncate'` \| `'truncate-start'` \| `'truncate-middle'` \| `'truncate-end'` | `'wrap'` | Text wrapping mode |

**Note:** The `wrap` property maps to `textWrap` in the underlying style system.

**Color Values:**

```php
// Named colors
'black', 'red', 'green', 'yellow', 'blue', 'magenta', 'cyan', 'white'
'gray', 'grey', 'blackBright', 'redBright', 'greenBright', 'yellowBright',
'blueBright', 'magentaBright', 'cyanBright', 'whiteBright'

// Hex colors
'#ff0000', '#00ff00', '#0000ff'

// RGB
'rgb(255, 0, 0)'

// ANSI 256
'ansi256(196)'
```

### Newline

Insert a blank line.

```php
use Ink\Newline;

Box::create()
    ->child(Text::create('Line 1'))
    ->child(Newline::create())     // One blank line
    ->child(Newline::create(2))    // Two blank lines
    ->child(Text::create('Line 2'));
```

### Spacer

Flexible space that expands to fill available space.

```php
use Ink\{Box, Text, Spacer};

// Push content to edges
Box::create(['flexDirection' => 'row', 'width' => '100%'])
    ->child(Text::create('Left'))
    ->child(Spacer::create())
    ->child(Text::create('Right'));
```

### Static

Permanently render output above everything else. Useful for displaying completed tasks,
logs, or other content that shouldn't change after being rendered.

```php
use Ink\{Box, Text, StaticOutput};

// Static takes an array of items and a render function
// Only NEW items are rendered (not previously seen ones)
Box::create()
    ->child(
        StaticOutput::create([
            'items' => $completedItems,
            'render' => fn($item, $index) =>
                Text::create("✓ {$item}", ['color' => 'green'])
        ])
    )
    ->child(Text::create('Current: Processing...'));
```

**Static Properties:**

| Property | Type | Description |
|----------|------|-------------|
| `items` | `array` | Array of items to render |
| `render` | `callable` | `fn($item, $index) => Node` - renders each item |
| `style` | `array` | Optional styles (defaults to `position: absolute, flexDirection: column`) |

**Note:** Static output is rendered above the main output and persists between renders.
Only newly added items are rendered - items already seen in previous renders are skipped.

### Transform

Apply string transformation to children.

```php
use Ink\{Transform, Text};

// Uppercase
Transform::create(fn($s) => strtoupper($s))
    ->child(Text::create('hello'));  // Outputs: HELLO

// Custom gradient (pseudo)
Transform::create(fn($s) => applyGradient($s))
    ->child(Text::create('Gradient Text'));
```

### ErrorOverview

Display errors with stack traces and source code excerpts. Used internally when
errors occur, but can also be used manually:

```php
use Ink\{Box, ErrorOverview};

try {
    riskyOperation();
} catch (\Throwable $e) {
    // Renders nicely formatted error with:
    // - Error message with red background
    // - Source file location
    // - Code excerpt with line highlighting
    // - Stack trace
    return ErrorOverview::create($e);
}
```

**ErrorOverview automatically shows:**

```
 ERROR  Division by zero

src/Calculator.php:42:15

  40│   public function divide($a, $b) {
  41│       if ($b === 0) {
  42│           throw new \Exception('Division by zero');
  43│       }
  44│       return $a / $b;

- divide (src/Calculator.php:42:15)
- calculate (src/Math.php:23:8)
- main (bin/app.php:10:1)
```

---

## Hooks

### useState

Manage component state with automatic re-renders.

```php
use function Ink\render;
use Ink\{Box, Text};

$instance = render(function(Ink\App $app) {
    // Simple state - returns [value, setter]
    [$count, $setCount] = $app->useState(0);

    // Update with value
    $setCount(5);

    // Update with callback (receives previous value)
    $setCount(fn($prev) => $prev + 1);

    // Object/array state
    [$form, $setForm] = $app->useState(['name' => '', 'email' => '']);
    $setForm(fn($f) => [...$f, 'name' => 'John']);

    return Box::create()
        ->child(Text::create("Count: {$count}"));
});
```

**Important:** State is tracked by call order (like React hooks). Always call `useState`
in the same order on every render - don't put it inside conditionals.

### useInput

Handle keyboard input.

```php
$app->useInput(function(string $input, Ink\Key $key) use ($setCount, $app) {
    // Character input
    if ($input === 'q') {
        $app->exit();
    }

    // Arrow keys (using boolean flags)
    if ($key->upArrow) {
        $setCount(fn($c) => $c + 1);
    }

    if ($key->downArrow) {
        $setCount(fn($c) => $c - 1);
    }

    // Modifiers with character
    if ($key->ctrl && $input === 'c') {
        $app->exit();
    }

    // Navigation keys
    if ($key->return) {
        // Handle Enter key
    }

    if ($key->escape) {
        // Handle Escape key
    }
});
```

**Key Properties:**

```php
class Key {
    // Arrow keys
    public bool $upArrow;
    public bool $downArrow;
    public bool $leftArrow;
    public bool $rightArrow;

    // Navigation
    public bool $pageUp;
    public bool $pageDown;
    public bool $home;
    public bool $end;

    // Action keys
    public bool $return;      // Enter/Return key
    public bool $escape;
    public bool $tab;
    public bool $backspace;
    public bool $delete;

    // Modifiers
    public bool $ctrl;        // Ctrl key held
    public bool $shift;       // Shift key held
    public bool $meta;        // Alt/Option/Meta key held
}
```

**Note:** The `$input` parameter contains the actual character(s) typed. For special keys
(arrows, function keys, etc.), `$input` will be empty and you should check the boolean flags.

### useFocus

Make a component focusable and track focus state. When user presses Tab,
focus moves between focusable components in render order.

```php
use function Ink\render;
use Ink\{Box, Text};

$instance = render(function(Ink\App $app) {
    // Make this "component" focusable
    $focus = $app->useFocus(['autoFocus' => true]);

    return Box::create([
        'borderStyle' => 'round',
        'borderColor' => $focus->isFocused ? 'blue' : 'gray',
    ])->child(Text::create('Input field'));
});

// Multiple focusable items with custom IDs
$instance = render(function(Ink\App $app) {
    $items = ['Item 1', 'Item 2', 'Item 3'];
    $nodes = [];

    foreach ($items as $i => $item) {
        $focus = $app->useFocus(['id' => "item-{$i}"]);

        $nodes[] = Box::create([
            'borderStyle' => $focus->isFocused ? 'double' : 'single',
        ])->child(Text::create($item));
    }

    return Box::create(['flexDirection' => 'column'])
        ->children($nodes);
});
```

**Focus Properties:**

```php
class Focus {
    public bool $isFocused;   // Whether this component is focused

    /**
     * Focus a specific component by ID.
     * @param string $id The focus ID to target
     */
    public function focus(string $id): void;
}
```

### useFocusManager

Control focus navigation programmatically.

```php
use function Ink\render;
use Ink\{Box, Text};

$instance = render(function(Ink\App $app) {
    $focusManager = $app->useFocusManager();

    $app->useInput(function($input, $key) use ($focusManager) {
        // Custom focus navigation
        if ($key->downArrow) {
            $focusManager->focusNext();
        }
        if ($key->upArrow) {
            $focusManager->focusPrevious();
        }
    });

    return Box::create()->child(Text::create('...'));
});
```

**FocusManager Methods:**

```php
class FocusManager {
    /**
     * Move focus to next focusable component.
     */
    public function focusNext(): void;

    /**
     * Move focus to previous focusable component.
     */
    public function focusPrevious(): void;

    /**
     * Focus a specific component by ID.
     */
    public function focus(string $id): void;

    /**
     * Enable focus management (default).
     */
    public function enableFocus(): void;

    /**
     * Disable focus management (Tab won't move focus).
     */
    public function disableFocus(): void;
}
```

### useStdin / useStdout / useStderr

Access raw stdin/stdout/stderr streams.

```php
$stdin = $app->useStdin();
$stdout = $app->useStdout();
$stderr = $app->useStderr();

// Check if running in TTY
if ($stdout->isTTY) {
    // Interactive mode
}

// Get terminal size
$width = $stdout->columns;
$height = $stdout->rows;

// Write to stderr
$stderr->write("Error message\n");
```

**StdinContext Properties:**

```php
class StdinContext {
    public resource $stdin;           // Raw stdin stream
    public bool $isRawModeSupported;  // Whether raw mode is available

    public function setRawMode(bool $mode): void;
}
```

**StdoutContext Properties:**

```php
class StdoutContext {
    public resource $stdout;    // Raw stdout stream
    public int $columns;        // Terminal width
    public int $rows;           // Terminal height
    public bool $isTTY;         // Is a TTY

    public function write(string $data): void;
}
```

**StderrContext Properties:**

```php
class StderrContext {
    public resource $stderr;    // Raw stderr stream
    public int $columns;        // Terminal width (same as stdout)
    public int $rows;           // Terminal height
    public bool $isTTY;         // Is a TTY

    public function write(string $data): void;
}
```

---

## Context System

In React/Ink, contexts provide a way to pass data through the component tree without
manually passing props. Tui provides similar functionality through context objects
that are accessible via the `App` instance.

### Available Contexts

| Context | Hook | Purpose |
|---------|------|---------|
| `StdinContext` | `$app->useStdin()` | Raw stdin access, raw mode control |
| `StdoutContext` | `$app->useStdout()` | Terminal dimensions, stdout access |
| `StderrContext` | `$app->useStderr()` | Stderr stream access |
| `FocusContext` | `$app->useFocus()` | Focus state for interactive components |
| `AppContext` | `$app` (implicit) | Exit function, app lifecycle |

### How Contexts Work in Tui

Unlike React where contexts use a Provider/Consumer pattern, Tui contexts are
accessed through the `App` instance that's passed to the render callback:

```php
use function Ink\render;
use Ink\{Box, Text};

$instance = render(function(Ink\App $app) {
    // Access contexts through $app
    $stdin = $app->useStdin();
    $stdout = $app->useStdout();
    $stderr = $app->useStderr();
    $focus = $app->useFocus();
    $focusManager = $app->useFocusManager();

    // Use terminal dimensions from stdout context
    $width = $stdout->columns;
    $height = $stdout->rows;

    // Check if we're in a TTY
    if (!$stdout->isTTY) {
        // Non-interactive mode - maybe just output and exit
    }

    // Raw mode control via stdin context
    if ($stdin->isRawModeSupported) {
        $stdin->setRawMode(true);
    }

    return Box::create(['width' => $width])
        ->child(Text::create("Terminal: {$width}x{$height}"));
});
```

### AppContext

The `App` object itself serves as the AppContext, providing:

```php
class App {
    /**
     * Exit the application.
     * In Ink, this is accessed via useApp().exit()
     */
    public function exit(?\Throwable $error = null): void;
}
```

### FocusContext (Internal)

The focus system is managed internally by the extension. When you call `useFocus()`,
the extension:

1. Registers the component in the focus order
2. Tracks which component has focus (by ID)
3. Handles Tab key to cycle focus
4. Returns a `Focus` object with `isFocused` state

```php
// Internal focus tracking (C implementation)
typedef struct {
    char *id;                    // Focus ID (auto-generated or custom)
    uint8_t is_active;           // Whether this focus registration is active
    uint8_t auto_focus;          // Should auto-focus on mount
} ink_focus_entry;

typedef struct {
    ink_focus_entry *entries;    // All registered focusable components
    int entry_count;
    char *active_id;             // Currently focused component ID
    uint8_t focus_enabled;       // Global focus enable/disable
} ink_focus_context;
```

### Context Lifecycle

Contexts are created when `render()` is called and destroyed on `unmount()`:

```
render() called
    │
    ├── Create ink_app instance
    ├── Initialize StdinContext (get stdin handle, check TTY)
    ├── Initialize StdoutContext (get dimensions via ioctl)
    ├── Initialize StderrContext
    ├── Initialize FocusContext (empty focus list)
    │
    ▼
Event loop running
    │
    ├── Components call useFocus() → register in FocusContext
    ├── Components call useInput() → register input handlers
    ├── Terminal resize → update StdoutContext.columns/rows
    │
    ▼
unmount() called
    │
    ├── Cleanup all contexts
    ├── Restore terminal mode
    └── Free resources
```

---

## Styling

### Colors

```php
// Foreground
Text::create('Red', ['color' => 'red']);
Text::create('Hex', ['color' => '#ff6600']);
Text::create('RGB', ['color' => 'rgb(100, 200, 50)']);

// Background
Text::create('Highlight', [
    'color' => 'black',
    'backgroundColor' => 'yellow',
]);

// 256 colors
Text::create('256', ['color' => 'ansi256(196)']);
```

### Text Decoration

```php
Text::create('Bold', ['bold' => true]);
Text::create('Italic', ['italic' => true]);
Text::create('Underline', ['underline' => true]);
Text::create('Dim', ['dim' => true]);
Text::create('Inverse', ['inverse' => true]);
Text::create('Strike', ['strikethrough' => true]);

// Combined
Text::create('Bold Underline', ['bold' => true, 'underline' => true]);
```

### Borders

```php
// Border styles
Box::create(['borderStyle' => 'single']);   // ┌─┐
Box::create(['borderStyle' => 'double']);   // ╔═╗
Box::create(['borderStyle' => 'round']);    // ╭─╮
Box::create(['borderStyle' => 'bold']);     // ┏━┓
Box::create(['borderStyle' => 'classic']);  // +-+

// Border color
Box::create([
    'borderStyle' => 'round',
    'borderColor' => 'cyan',
]);

// Partial borders
Box::create([
    'borderStyle' => 'single',
    'borderTop' => true,
    'borderBottom' => true,
    'borderLeft' => false,
    'borderRight' => false,
]);
```

### Dimensions

```php
// Fixed size
Box::create(['width' => 20, 'height' => 5]);

// Percentage
Box::create(['width' => '50%', 'height' => '100%']);

// Min/max constraints
Box::create([
    'width' => '100%',
    'minWidth' => 20,
    'maxWidth' => 80,
]);
```

---

## Layout System

Tui uses Yoga, Facebook's cross-platform flexbox implementation.

### Flex Direction

```php
// Horizontal (default)
Box::create(['flexDirection' => 'row'])
    ->child(Box::create()->child(Text::create('1')))
    ->child(Box::create()->child(Text::create('2')))
    ->child(Box::create()->child(Text::create('3')));
// Output: 1 2 3

// Vertical
Box::create(['flexDirection' => 'column'])
    ->child(Box::create()->child(Text::create('1')))
    ->child(Box::create()->child(Text::create('2')))
    ->child(Box::create()->child(Text::create('3')));
// Output:
// 1
// 2
// 3
```

### Alignment

```php
// Main axis (justifyContent)
Box::create([
    'flexDirection' => 'row',
    'width' => '100%',
    'justifyContent' => 'space-between',
]);

// Cross axis (alignItems)
Box::create([
    'flexDirection' => 'row',
    'height' => 10,
    'alignItems' => 'center',
]);
```

### Flex Grow/Shrink

```php
Box::create(['flexDirection' => 'row', 'width' => '100%'])
    ->child(Box::create(['width' => 10])->child(Text::create('Fixed')))
    ->child(Box::create(['flexGrow' => 1])->child(Text::create('Grows')))
    ->child(Box::create(['width' => 10])->child(Text::create('Fixed')));
```

### Padding & Margin

```php
// All sides
Box::create(['padding' => 2, 'margin' => 1]);

// Individual sides
Box::create([
    'paddingTop' => 1,
    'paddingRight' => 2,
    'paddingBottom' => 1,
    'paddingLeft' => 2,
]);

// Shorthand
Box::create([
    'paddingX' => 2,  // left + right
    'paddingY' => 1,  // top + bottom
]);
```

### Gap

```php
Box::create([
    'flexDirection' => 'column',
    'gap' => 1,
])
->child(Text::create('Line 1'))
->child(Text::create('Line 2'))
->child(Text::create('Line 3'));
```

---

## Input Handling

### Key Parsing

The extension parses raw stdin bytes into structured key events:

```c
// src/terminal/input.c

typedef struct {
    char name[32];
    uint8_t ctrl;
    uint8_t alt;
    uint8_t shift;
    uint8_t meta;
    char sequence[16];
} ink_key_event;

ink_key_event* parse_key(const char *buf, size_t len) {
    ink_key_event *event = alloc_key_event();

    // Escape sequences
    if (len >= 3 && buf[0] == '\x1b' && buf[1] == '[') {
        switch (buf[2]) {
            case 'A': strcpy(event->name, "up"); break;
            case 'B': strcpy(event->name, "down"); break;
            case 'C': strcpy(event->name, "right"); break;
            case 'D': strcpy(event->name, "left"); break;
            case 'H': strcpy(event->name, "home"); break;
            case 'F': strcpy(event->name, "end"); break;
            // ... more sequences
        }
    }
    // Control characters
    else if (len == 1 && buf[0] < 32) {
        event->ctrl = 1;
        event->name[0] = buf[0] + 96;  // Ctrl+A = 1 -> 'a'
        event->name[1] = '\0';
    }
    // Regular characters
    else {
        memcpy(event->name, buf, len);
        event->name[len] = '\0';
    }

    return event;
}
```

### Escape Sequence Reference

| Sequence | Key |
|----------|-----|
| `\x1b[A` | Up |
| `\x1b[B` | Down |
| `\x1b[C` | Right |
| `\x1b[D` | Left |
| `\x1b[H` | Home |
| `\x1b[F` | End |
| `\x1b[5~` | Page Up |
| `\x1b[6~` | Page Down |
| `\x1b[2~` | Insert |
| `\x1b[3~` | Delete |
| `\x1bOP` | F1 |
| `\x1bOQ` | F2 |
| `\x1bOR` | F3 |
| `\x1bOS` | F4 |
| `\x1b[15~` | F5 |
| `\x1b[17~` | F6 |
| ... | ... |

---

## Reconciler and Render Tree

This section documents the reconciler architecture and how the render tree port from Ink's React-based system to Tui's C-based system works.

### Ink's React Reconciler Architecture

Ink uses React's `react-reconciler` package to create a custom renderer. This handles the virtual DOM diffing automatically through React's reconciliation algorithm.

```
┌─────────────────────────────────────────────────────────────────────────┐
│                       INK (JavaScript)                                   │
│                                                                          │
│   ┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐   │
│   │  React JSX      │───▶│  react-reconciler │───▶│  Virtual DOM    │   │
│   │  Components     │    │  (fiber diffing)  │    │  (ink nodes)    │   │
│   └─────────────────┘    └──────────────────┘    └─────────────────┘   │
│                                                           │              │
│                                                           ▼              │
│   ┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐   │
│   │  Terminal       │◀───│  Output Buffer   │◀───│  Yoga Layout    │   │
│   │  (stdout)       │    │  (2D char grid)  │    │  Engine         │   │
│   └─────────────────┘    └──────────────────┘    └─────────────────┘   │
└─────────────────────────────────────────────────────────────────────────┘
```

### Tui Reconciler Architecture

Tui cannot use React's reconciler since it's a C extension. Instead, it implements a simplified reconciler that achieves the same result.

```
┌─────────────────────────────────────────────────────────────────────────┐
│                       PHP-INK (C Extension)                              │
│                                                                          │
│   ┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐   │
│   │  PHP Closures   │───▶│  Custom          │───▶│  ink_node Tree  │   │
│   │  (render funcs) │    │  Reconciler (C)  │    │  (C structs)    │   │
│   └─────────────────┘    └──────────────────┘    └─────────────────┘   │
│                                                           │              │
│                                                           ▼              │
│   ┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐   │
│   │  Terminal       │◀───│  Output Buffer   │◀───│  Yoga Layout    │   │
│   │  (stdout)       │    │  (2D char grid)  │    │  (via php-yoga) │   │
│   └─────────────────┘    └──────────────────┘    └─────────────────┘   │
└─────────────────────────────────────────────────────────────────────────┘
```

### Node Types

Ink uses these virtual DOM node types:

| Ink Node Type | Tui Equivalent | Description |
|---------------|-------------------|-------------|
| `ink-root` | `INK_NODE_ROOT` | Root container |
| `ink-box` | `INK_NODE_BOX` | Flexbox container (has YogaNode) |
| `ink-text` | `INK_NODE_TEXT` | Text container with measure function |
| `ink-virtual-text` | `INK_NODE_VIRTUAL_TEXT` | Nested text (no YogaNode) |
| `#text` | `INK_NODE_TEXT_VALUE` | Raw text content |

### Ink's Reconciler Host Config

Ink implements these React reconciler methods:

```javascript
// Key methods from Ink's reconciler.js

createInstance(type, props, root, hostContext) {
    // Create ink node with YogaNode
    const node = createNode(type);
    applyStyles(node.yogaNode, props.style);
    return node;
}

createTextInstance(text, root, hostContext) {
    // Text must be inside <Text> component
    if (!hostContext.isInsideText) {
        throw new Error('Text must be inside <Text>');
    }
    return createTextNode(text);
}

appendChildNode(parent, child) {
    // Add to childNodes array and Yoga tree
    parent.childNodes.push(child);
    parent.yogaNode.insertChild(child.yogaNode, ...);
}

removeChildNode(parent, child) {
    // Remove from childNodes and cleanup Yoga
    parent.childNodes.splice(index, 1);
    child.yogaNode.freeRecursive();
}

prepareUpdate(node, type, oldProps, newProps) {
    // Diff props to determine what changed
    const props = diff(oldProps, newProps);
    const style = diff(oldProps.style, newProps.style);
    return { props, style };
}

commitUpdate(node, { props, style }) {
    // Apply prop and style changes
    for (const [key, value] of Object.entries(props)) {
        setAttribute(node, key, value);
    }
    applyStyles(node.yogaNode, style);
}

resetAfterCommit(rootNode) {
    // Trigger render after commit phase
    rootNode.onRender();
}
```

### Tui C Reconciler

Since Tui cannot use React, it implements its own reconciler:

```c
// src/reconciler/reconciler.h

typedef enum {
    INK_NODE_ROOT,
    INK_NODE_BOX,
    INK_NODE_TEXT,
    INK_NODE_VIRTUAL_TEXT,
    INK_NODE_TEXT_VALUE,
} ink_node_type;

typedef struct ink_node {
    ink_node_type type;
    char *node_name;

    // Props and styles
    ink_style *style;
    zend_array *attributes;
    uint32_t props_hash;        // Fast change detection

    // Text content (for TEXT_VALUE nodes)
    char *text_value;
    size_t text_len;

    // Yoga layout node (NULL for virtual-text and text-value)
    YGNodeRef yoga_node;

    // Tree structure
    struct ink_node *parent;
    struct ink_node *first_child;
    struct ink_node *last_child;
    struct ink_node *next_sibling;
    struct ink_node *prev_sibling;
    size_t child_count;

    // Transform function (for <Transform>)
    zval *transform_func;

    // Static flag (for <Static>)
    bool is_static;

    // Internal tracking
    bool is_dirty;
    char *key;                  // Optional key for list reconciliation
} ink_node;
```

### Reconciliation Algorithm

```
┌──────────────────────────────────────────────────────────────────────────┐
│                     RECONCILIATION FLOW                                   │
├──────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│   OLD TREE                          NEW TREE                              │
│   ========                          ========                              │
│   ink-root                          ink-root                              │
│     │                                 │                                   │
│     ├─ ink-box (A)                   ├─ ink-box (A)  ←─ Same key, DIFF   │
│     │    └─ ink-text                 │    └─ ink-text     props/children │
│     │         └─ "Hello"             │         └─ "Hello!"               │
│     │                                 │                                   │
│     ├─ ink-box (B)                   ├─ ink-box (C)  ←─ New node, INSERT │
│     │    └─ ink-text                 │    └─ ink-text                    │
│     │         └─ "World"             │         └─ "New"                  │
│     │                                 │                                   │
│     └─ ink-box (C)                   └─ ink-box (B)  ←─ Moved, MOVE      │
│          └─ ink-text                      └─ ink-text                    │
│               └─ "!"                           └─ "World"                │
│                                                                           │
│   Node (B) not in new ──────────────────────────────────► DELETE         │
│                                                                           │
└──────────────────────────────────────────────────────────────────────────┘
```

### C Implementation

```c
// src/reconciler/reconciler.c

typedef enum {
    UPDATE_NONE,
    UPDATE_PROPS,      // Style/attributes changed
    UPDATE_REPLACE,    // Node type changed
    UPDATE_INSERT,     // New node
    UPDATE_DELETE,     // Node removed
    UPDATE_MOVE,       // Node reordered
    UPDATE_TEXT,       // Text content changed
} update_type;

typedef struct update_op {
    update_type type;
    ink_node *old_node;
    ink_node *new_node;
    ink_node *parent;
    int index;
    struct update_op *next;
} update_op;

// Main reconciliation entry point
update_op* reconcile(ink_node *old_root, ink_node *new_root) {
    if (!old_root && !new_root) return NULL;
    if (!old_root) return create_op(UPDATE_INSERT, NULL, new_root, NULL, 0);
    if (!new_root) return create_op(UPDATE_DELETE, old_root, NULL, NULL, 0);

    return reconcile_node(old_root, new_root);
}

update_op* reconcile_node(ink_node *old_node, ink_node *new_node) {
    update_op *ops = NULL;

    // Type changed - replace entire subtree
    if (old_node->type != new_node->type) {
        return create_op(UPDATE_REPLACE, old_node, new_node, old_node->parent, 0);
    }

    // Check props via hash (fast path)
    if (old_node->props_hash != new_node->props_hash) {
        ops = create_op(UPDATE_PROPS, old_node, new_node, NULL, 0);
    }

    // Text value changed
    if (old_node->type == INK_NODE_TEXT_VALUE) {
        if (strcmp(old_node->text_value, new_node->text_value) != 0) {
            ops = merge_ops(ops, create_op(UPDATE_TEXT, old_node, new_node, NULL, 0));
        }
        return ops;
    }

    // Reconcile children
    update_op *child_ops = reconcile_children(old_node, new_node);
    return merge_ops(ops, child_ops);
}

update_op* reconcile_children(ink_node *old_parent, ink_node *new_parent) {
    update_op *ops = NULL;

    // Build key->node maps for efficient matching
    hashmap *old_keyed = hashmap_new();
    hashmap *new_keyed = hashmap_new();

    int old_index = 0;
    for (ink_node *n = old_parent->first_child; n; n = n->next_sibling) {
        char *key = n->key ? n->key : make_index_key(old_index++);
        hashmap_put(old_keyed, key, n);
    }

    int new_index = 0;
    for (ink_node *n = new_parent->first_child; n; n = n->next_sibling) {
        char *key = n->key ? n->key : make_index_key(new_index++);
        hashmap_put(new_keyed, key, n);
    }

    // Process new children - find matches or mark as INSERT
    new_index = 0;
    for (ink_node *new_child = new_parent->first_child; new_child;
         new_child = new_child->next_sibling) {

        char *key = new_child->key ? new_child->key : make_index_key(new_index);
        ink_node *old_child = hashmap_get(old_keyed, key);

        if (old_child) {
            // Found match - recursively reconcile
            update_op *child_ops = reconcile_node(old_child, new_child);
            ops = merge_ops(ops, child_ops);
            hashmap_remove(old_keyed, key);  // Mark as matched
        } else {
            // New node - insert
            ops = append_op(ops, UPDATE_INSERT, NULL, new_child,
                           new_parent, new_index);
        }
        new_index++;
    }

    // Remaining old children are deletions
    hashmap_iter iter;
    hashmap_iter_init(old_keyed, &iter);
    while (hashmap_iter_next(&iter)) {
        ink_node *old_child = iter.value;
        ops = append_op(ops, UPDATE_DELETE, old_child, NULL, old_parent, 0);
    }

    hashmap_free(old_keyed);
    hashmap_free(new_keyed);

    return ops;
}
```

### Applying Updates to Yoga Tree

```c
// src/reconciler/apply.c

void apply_updates(update_op *ops) {
    for (update_op *op = ops; op; op = op->next) {
        switch (op->type) {
            case UPDATE_INSERT:
                apply_insert(op);
                break;
            case UPDATE_DELETE:
                apply_delete(op);
                break;
            case UPDATE_REPLACE:
                apply_replace(op);
                break;
            case UPDATE_PROPS:
                apply_props(op);
                break;
            case UPDATE_TEXT:
                apply_text(op);
                break;
            case UPDATE_MOVE:
                apply_move(op);
                break;
        }
    }
}

static void apply_insert(update_op *op) {
    ink_node *node = op->new_node;
    ink_node *parent = op->parent;

    // Create Yoga node if needed
    if (node->type != INK_NODE_VIRTUAL_TEXT &&
        node->type != INK_NODE_TEXT_VALUE) {
        node->yoga_node = YGNodeNew();
        apply_styles_to_yoga(node->yoga_node, node->style);
    }

    // Set text measure function
    if (node->type == INK_NODE_TEXT) {
        YGNodeSetMeasureFunc(node->yoga_node, measure_text_node);
        YGNodeSetContext(node->yoga_node, node);
    }

    // Insert into parent's Yoga tree
    if (parent->yoga_node && node->yoga_node) {
        YGNodeInsertChild(parent->yoga_node, node->yoga_node, op->index);
    }

    // Link into DOM tree
    insert_child_node(parent, node, op->index);
}

static void apply_delete(update_op *op) {
    ink_node *node = op->old_node;
    ink_node *parent = op->parent;

    // Remove from Yoga tree
    if (parent->yoga_node && node->yoga_node) {
        YGNodeRemoveChild(parent->yoga_node, node->yoga_node);
    }

    // Cleanup Yoga node recursively
    if (node->yoga_node) {
        YGNodeFreeRecursive(node->yoga_node);
    }

    // Unlink from DOM tree
    remove_child_node(parent, node);

    // Free node memory
    ink_node_free(node);
}

static void apply_props(update_op *op) {
    ink_node *old = op->old_node;
    ink_node *new = op->new_node;

    // Copy new style to existing node
    if (old->style && new->style) {
        memcpy(old->style, new->style, sizeof(ink_style));
    }

    // Apply to Yoga node
    if (old->yoga_node) {
        apply_styles_to_yoga(old->yoga_node, old->style);
    }

    // Update props hash
    old->props_hash = new->props_hash;

    // Mark text nodes dirty for re-measurement
    if (old->type == INK_NODE_TEXT || old->type == INK_NODE_VIRTUAL_TEXT) {
        mark_node_dirty(old);
    }
}

static void apply_text(update_op *op) {
    ink_node *node = op->old_node;

    // Free old text
    if (node->text_value) {
        efree(node->text_value);
    }

    // Copy new text
    node->text_value = estrdup(op->new_node->text_value);
    node->text_len = op->new_node->text_len;

    // Mark parent text node dirty
    mark_node_dirty(node);
}
```

### Props Diffing

Ink's JavaScript implementation:

```javascript
const diff = (before, after) => {
    if (before === after) return;
    if (!before) return after;

    const changed = {};
    let isChanged = false;

    // Find deleted keys
    for (const key of Object.keys(before)) {
        if (after ? !Object.hasOwn(after, key) : true) {
            changed[key] = undefined;
            isChanged = true;
        }
    }

    // Find changed keys
    if (after) {
        for (const key of Object.keys(after)) {
            if (after[key] !== before[key]) {
                changed[key] = after[key];
                isChanged = true;
            }
        }
    }

    return isChanged ? changed : undefined;
};
```

Tui C implementation using FNV-1a hashing for fast comparison:

```c
// src/reconciler/props.c

// Fast path: compare props via hash
uint32_t hash_props(ink_style *style, zend_array *attrs) {
    uint32_t hash = 2166136261u;  // FNV offset basis

    // Hash all style properties
    hash = fnv1a_update(hash, &style->flex_direction, sizeof(style->flex_direction));
    hash = fnv1a_update(hash, &style->justify_content, sizeof(style->justify_content));
    hash = fnv1a_update(hash, &style->align_items, sizeof(style->align_items));
    hash = fnv1a_update(hash, &style->flex_grow, sizeof(style->flex_grow));
    hash = fnv1a_update(hash, &style->flex_shrink, sizeof(style->flex_shrink));
    hash = fnv1a_update(hash, &style->width, sizeof(style->width));
    hash = fnv1a_update(hash, &style->height, sizeof(style->height));
    hash = fnv1a_update(hash, &style->margin, sizeof(style->margin));
    hash = fnv1a_update(hash, &style->padding, sizeof(style->padding));
    hash = fnv1a_update(hash, &style->fg_color, sizeof(style->fg_color));
    hash = fnv1a_update(hash, &style->bg_color, sizeof(style->bg_color));
    hash = fnv1a_update(hash, &style->border_style, sizeof(style->border_style));
    hash = fnv1a_update(hash, &style->text_wrap, sizeof(style->text_wrap));
    hash = fnv1a_update(hash, &style->overflow, sizeof(style->overflow));
    // ... all other properties

    // Hash attribute keys and values
    if (attrs) {
        zend_string *key;
        zval *val;
        ZEND_HASH_FOREACH_STR_KEY_VAL(attrs, key, val) {
            hash = fnv1a_update(hash, ZSTR_VAL(key), ZSTR_LEN(key));
            hash = hash_zval(hash, val);
        } ZEND_HASH_FOREACH_END();
    }

    return hash;
}

static inline uint32_t fnv1a_update(uint32_t hash, const void *data, size_t len) {
    const uint8_t *bytes = (const uint8_t *)data;
    for (size_t i = 0; i < len; i++) {
        hash ^= bytes[i];
        hash *= 16777619u;  // FNV prime
    }
    return hash;
}

static uint32_t hash_zval(uint32_t hash, zval *val) {
    switch (Z_TYPE_P(val)) {
        case IS_LONG:
            return fnv1a_update(hash, &Z_LVAL_P(val), sizeof(zend_long));
        case IS_DOUBLE:
            return fnv1a_update(hash, &Z_DVAL_P(val), sizeof(double));
        case IS_STRING:
            return fnv1a_update(hash, Z_STRVAL_P(val), Z_STRLEN_P(val));
        case IS_TRUE:
            return fnv1a_update(hash, "\x01", 1);
        case IS_FALSE:
            return fnv1a_update(hash, "\x00", 1);
        default:
            return hash;
    }
}
```

### Dirty Marking and Text Measurement

```c
// src/reconciler/dirty.c

// Mark a text node dirty to trigger re-measurement
void mark_node_dirty(ink_node *node) {
    // Find closest ancestor with Yoga node
    YGNodeRef yoga_node = find_closest_yoga_node(node);
    if (yoga_node) {
        YGNodeMarkDirty(yoga_node);
    }
}

static YGNodeRef find_closest_yoga_node(ink_node *node) {
    if (!node || !node->parent) return NULL;
    if (node->yoga_node) return node->yoga_node;
    return find_closest_yoga_node(node->parent);
}

// Text measurement function for Yoga
static YGSize measure_text_node(
    YGNodeConstRef node,
    float width,
    YGMeasureMode widthMode,
    float height,
    YGMeasureMode heightMode
) {
    ink_node *ink = (ink_node *)YGNodeGetContext(node);

    // Squash all text children into single string
    char *text = squash_text_nodes(ink);
    text_dimensions dims = measure_text(text);

    // Text fits - return natural size
    if (dims.width <= width) {
        efree(text);
        return (YGSize){ .width = dims.width, .height = dims.height };
    }

    // Need to wrap - calculate wrapped dimensions
    char *wrapped = wrap_text(text, (int)width, ink->style->text_wrap);
    dims = measure_text(wrapped);

    efree(text);
    efree(wrapped);

    return (YGSize){ .width = dims.width, .height = dims.height };
}

// Collect all text from node and children
char *squash_text_nodes(ink_node *node) {
    smart_str result = {0};
    squash_text_recursive(node, &result);
    smart_str_0(&result);
    return result.s ? ZSTR_VAL(result.s) : estrdup("");
}

static void squash_text_recursive(ink_node *node, smart_str *result) {
    if (node->type == INK_NODE_TEXT_VALUE) {
        smart_str_appends(result, node->text_value);
        return;
    }

    for (ink_node *child = node->first_child; child; child = child->next_sibling) {
        squash_text_recursive(child, result);
    }
}
```

### Update Operations Lifecycle

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    UPDATE LIFECYCLE                                      │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  1. PHP calls render($callback)                                          │
│     └─ Callback returns new virtual tree                                 │
│                                                                          │
│  2. Reconciler diffs old_tree vs new_tree                               │
│     └─ Produces linked list of update_op                                │
│                                                                          │
│  3. Apply updates to ink_node tree                                       │
│     ├─ INSERT: Create node, link to parent                              │
│     ├─ DELETE: Unlink, free memory                                      │
│     ├─ REPLACE: Delete old, insert new                                  │
│     ├─ PROPS: Update style/attrs in place                               │
│     └─ TEXT: Update text_value                                          │
│                                                                          │
│  4. Apply updates to Yoga tree                                           │
│     ├─ YGNodeInsertChild for INSERT                                     │
│     ├─ YGNodeRemoveChild for DELETE                                     │
│     ├─ apply_styles_to_yoga for PROPS                                   │
│     └─ YGNodeMarkDirty for TEXT                                         │
│                                                                          │
│  5. Calculate layout                                                     │
│     └─ YGNodeCalculateLayout(root, maxWidth, maxHeight)                 │
│                                                                          │
│  6. Render to output buffer                                              │
│     └─ render_node_to_output(root, buffer)                              │
│                                                                          │
│  7. Diff output buffer                                                   │
│     └─ Compare with previous output                                     │
│                                                                          │
│  8. Write to terminal                                                    │
│     └─ Only write changed lines                                         │
│                                                                          │
│  9. Store current tree as old_tree for next render                      │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Rendering Pipeline

### Overview

```
┌─────────────────────────────────────────────────────────────┐
│                     1. LAYOUT PHASE                         │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Yoga calculates positions and dimensions           │   │
│  │  YGNodeCalculateLayout(root, maxWidth, maxHeight)   │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     2. RENDER PHASE                         │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Traverse tree, render each node to 2D char buffer  │   │
│  │  Apply styles (colors, borders, text)               │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     3. DIFF PHASE                           │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Compare new buffer with previous buffer            │   │
│  │  Identify changed lines/regions                     │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     4. OUTPUT PHASE                         │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Generate ANSI escape sequences                     │   │
│  │  Write only changed content to stdout               │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

### Output Buffer

```c
// 2D character buffer with style info
typedef struct {
    uint32_t codepoint;      // Unicode codepoint
    ink_color fg;
    ink_color bg;
    uint8_t bold : 1;
    uint8_t italic : 1;
    uint8_t underline : 1;
    uint8_t dim : 1;
    uint8_t inverse : 1;
    uint8_t strikethrough : 1;
} ink_cell;

typedef struct {
    ink_cell *cells;
    int width;
    int height;
} ink_buffer;

// Render node to buffer
void render_node(ink_node *node, ink_buffer *buffer, int x, int y) {
    // Get computed layout from Yoga
    int left = (int)YGNodeLayoutGetLeft(node->yoga_node);
    int top = (int)YGNodeLayoutGetTop(node->yoga_node);
    int width = (int)YGNodeLayoutGetWidth(node->yoga_node);
    int height = (int)YGNodeLayoutGetHeight(node->yoga_node);

    int abs_x = x + left;
    int abs_y = y + top;

    // Apply background
    if (node->style.bg_color.set) {
        fill_background(buffer, abs_x, abs_y, width, height, node->style.bg_color);
    }

    // Render border
    if (node->style.border_style != BORDER_NONE) {
        render_border(buffer, abs_x, abs_y, width, height, &node->style);
    }

    // Render text content
    if (node->type == INK_TEXT && node->text) {
        render_text(buffer, abs_x, abs_y, width, node->text, &node->style);
    }

    // Recurse to children
    for (ink_node *child = node->first_child; child; child = child->next_sibling) {
        render_node(child, buffer, abs_x, abs_y);
    }
}
```

### Output Diffing

```c
void output_diff(ink_buffer *old, ink_buffer *new, FILE *out) {
    for (int y = 0; y < new->height; y++) {
        int line_changed = 0;

        // Check if line changed
        for (int x = 0; x < new->width; x++) {
            if (!cells_equal(&old->cells[y * old->width + x],
                            &new->cells[y * new->width + x])) {
                line_changed = 1;
                break;
            }
        }

        if (!line_changed) continue;

        // Move cursor to line start
        fprintf(out, "\x1b[%d;1H", y + 1);

        // Output line with minimal style changes
        ink_color current_fg = {0};
        ink_color current_bg = {0};
        uint8_t current_attrs = 0;

        for (int x = 0; x < new->width; x++) {
            ink_cell *cell = &new->cells[y * new->width + x];

            // Update style if changed
            if (!color_equal(&current_fg, &cell->fg)) {
                output_fg_color(out, &cell->fg);
                current_fg = cell->fg;
            }
            if (!color_equal(&current_bg, &cell->bg)) {
                output_bg_color(out, &cell->bg);
                current_bg = cell->bg;
            }
            // ... attributes ...

            // Output character
            output_codepoint(out, cell->codepoint);
        }
    }

    fflush(out);
}
```

### ANSI Escape Sequences

```c
// src/render/ansi.c

void output_fg_color(FILE *out, ink_color *color) {
    if (color->type == COLOR_NAMED) {
        fprintf(out, "\x1b[%dm", 30 + color->value);
    } else if (color->type == COLOR_256) {
        fprintf(out, "\x1b[38;5;%dm", color->value);
    } else if (color->type == COLOR_RGB) {
        fprintf(out, "\x1b[38;2;%d;%d;%dm",
                color->r, color->g, color->b);
    }
}

void output_bg_color(FILE *out, ink_color *color) {
    if (color->type == COLOR_NAMED) {
        fprintf(out, "\x1b[%dm", 40 + color->value);
    } else if (color->type == COLOR_256) {
        fprintf(out, "\x1b[48;5;%dm", color->value);
    } else if (color->type == COLOR_RGB) {
        fprintf(out, "\x1b[48;2;%d;%d;%dm",
                color->r, color->g, color->b);
    }
}

void output_attrs(FILE *out, uint8_t attrs) {
    if (attrs & ATTR_BOLD) fprintf(out, "\x1b[1m");
    if (attrs & ATTR_DIM) fprintf(out, "\x1b[2m");
    if (attrs & ATTR_ITALIC) fprintf(out, "\x1b[3m");
    if (attrs & ATTR_UNDERLINE) fprintf(out, "\x1b[4m");
    if (attrs & ATTR_INVERSE) fprintf(out, "\x1b[7m");
    if (attrs & ATTR_STRIKETHROUGH) fprintf(out, "\x1b[9m");
}

void reset_style(FILE *out) {
    fprintf(out, "\x1b[0m");
}

void clear_screen(FILE *out) {
    fprintf(out, "\x1b[2J\x1b[H");
}

void move_cursor(FILE *out, int x, int y) {
    fprintf(out, "\x1b[%d;%dH", y + 1, x + 1);
}

void hide_cursor(FILE *out) {
    fprintf(out, "\x1b[?25l");
}

void show_cursor(FILE *out) {
    fprintf(out, "\x1b[?25h");
}

void enter_alternate_screen(FILE *out) {
    fprintf(out, "\x1b[?1049h");
}

void exit_alternate_screen(FILE *out) {
    fprintf(out, "\x1b[?1049l");
}
```

---

## Drawing Primitives

Low-level drawing functions exposed by the C extension for rendering shapes, sprites, and graphics. These are used internally by `tui/widgets` but can also be called directly for performance-critical rendering.

### Buffer Drawing API

```c
// src/drawing/primitives.h

/**
 * Draw a single point on the buffer.
 */
void tui_draw_point(tui_buffer *buf, int x, int y, tui_style *style);

/**
 * Draw a line using Bresenham's algorithm.
 */
void tui_draw_line(tui_buffer *buf, int x1, int y1, int x2, int y2,
                   uint32_t char_code, tui_style *style);

/**
 * Draw a rectangle outline.
 */
void tui_draw_rect(tui_buffer *buf, int x, int y, int width, int height,
                   tui_border_style border, tui_style *style);

/**
 * Draw a filled rectangle.
 */
void tui_fill_rect(tui_buffer *buf, int x, int y, int width, int height,
                   uint32_t fill_char, tui_style *style);

/**
 * Draw a circle using midpoint circle algorithm.
 */
void tui_draw_circle(tui_buffer *buf, int cx, int cy, int radius,
                     uint32_t char_code, tui_style *style);

/**
 * Draw a filled circle.
 */
void tui_fill_circle(tui_buffer *buf, int cx, int cy, int radius,
                     uint32_t fill_char, tui_style *style);

/**
 * Draw an ellipse.
 */
void tui_draw_ellipse(tui_buffer *buf, int cx, int cy, int rx, int ry,
                      uint32_t char_code, tui_style *style);

/**
 * Draw a triangle.
 */
void tui_draw_triangle(tui_buffer *buf, int x1, int y1, int x2, int y2,
                       int x3, int y3, uint32_t char_code, tui_style *style);

/**
 * Fill a triangle using scanline algorithm.
 */
void tui_fill_triangle(tui_buffer *buf, int x1, int y1, int x2, int y2,
                       int x3, int y3, uint32_t fill_char, tui_style *style);
```

### PHP Bindings

```php
namespace Tui;

/**
 * Draw a line on a buffer/canvas.
 */
function tui_draw_line(
    mixed $buffer,
    int $x1, int $y1,
    int $x2, int $y2,
    string $char = '█',
    ?array $style = null
): void;

/**
 * Draw a rectangle outline.
 */
function tui_draw_rect(
    mixed $buffer,
    int $x, int $y,
    int $width, int $height,
    string $borderStyle = 'single', // single|double|round|bold
    ?array $style = null
): void;

/**
 * Fill a rectangular region.
 */
function tui_fill_rect(
    mixed $buffer,
    int $x, int $y,
    int $width, int $height,
    string $char = '█',
    ?array $style = null
): void;

/**
 * Draw a circle.
 */
function tui_draw_circle(
    mixed $buffer,
    int $cx, int $cy,
    int $radius,
    string $char = '●',
    ?array $style = null
): void;

/**
 * Fill a circle.
 */
function tui_fill_circle(
    mixed $buffer,
    int $cx, int $cy,
    int $radius,
    string $char = '█',
    ?array $style = null
): void;
```

### Sprite Rendering

Native sprite blitting for animated ASCII art with high performance.

```c
// src/drawing/sprite.h

/**
 * Sprite frame data.
 */
typedef struct {
    char **lines;           // Array of UTF-8 strings
    int width;              // Max width in chars
    int height;             // Number of lines
    tui_color color;        // Frame color
    int duration;           // Hold time in frames
} tui_sprite_frame;

/**
 * Sprite animation.
 */
typedef struct {
    tui_sprite_frame *frames;
    int frame_count;
    int current_frame;
    int frame_timer;        // Ticks since frame start
    bool loop;
    int frame_rate_ms;
} tui_sprite_animation;

/**
 * Sprite instance.
 */
typedef struct {
    tui_sprite_animation **animations;  // Named animations
    char **animation_names;
    int animation_count;
    char *current_animation;
    int x, y;
    bool visible;
    bool flipped;           // Horizontal flip
    tui_color default_color;
} tui_sprite;

/**
 * Blit a sprite frame to buffer at position.
 */
void tui_blit_sprite(tui_buffer *buf, tui_sprite *sprite);

/**
 * Update sprite animation (advance frames).
 */
void tui_update_sprite(tui_sprite *sprite, int delta_ms);

/**
 * Flip sprite horizontally (swaps directional chars).
 */
void tui_flip_sprite(tui_sprite *sprite, bool flipped);
```

### PHP Sprite API

```php
namespace Tui;

/**
 * Create a sprite from frames.
 *
 * @param array<string, array> $animations Named animation arrays
 * @return resource Sprite handle
 */
function tui_sprite_create(array $animations): mixed;

/**
 * Update sprite animation state.
 *
 * @param resource $sprite Sprite handle
 * @param int $deltaMs Milliseconds since last update
 */
function tui_sprite_update(mixed $sprite, int $deltaMs): void;

/**
 * Set current animation.
 */
function tui_sprite_set_animation(mixed $sprite, string $name): void;

/**
 * Set sprite position.
 */
function tui_sprite_set_position(mixed $sprite, int $x, int $y): void;

/**
 * Set horizontal flip.
 */
function tui_sprite_flip(mixed $sprite, bool $flipped): void;

/**
 * Blit sprite to current render buffer.
 */
function tui_sprite_render(mixed $sprite): void;

/**
 * Get sprite bounding box.
 *
 * @return array{x: int, y: int, width: int, height: int}
 */
function tui_sprite_get_bounds(mixed $sprite): array;

/**
 * Check collision between two sprites (AABB).
 */
function tui_sprite_collides(mixed $sprite1, mixed $sprite2): bool;
```

### Canvas (Braille/Block Drawing)

High-resolution canvas using braille characters (2x4 dots per cell) or block characters (2x2 blocks per cell).

```c
// src/drawing/canvas.h

typedef enum {
    TUI_CANVAS_BRAILLE,     // ⠀⠁⠂⠃... 2x4 resolution
    TUI_CANVAS_BLOCK,       // ▀▄█▌▐ 2x2 resolution
    TUI_CANVAS_ASCII        // ASCII chars
} tui_canvas_mode;

typedef struct {
    uint8_t *pixels;        // Bit array for set pixels
    int pixel_width;        // Width in pixels
    int pixel_height;       // Height in pixels
    int char_width;         // Width in terminal chars
    int char_height;        // Height in terminal chars
    tui_canvas_mode mode;
    tui_color color;
} tui_canvas;

/**
 * Create a canvas.
 */
tui_canvas* tui_canvas_create(int width, int height, tui_canvas_mode mode);

/**
 * Set a pixel.
 */
void tui_canvas_set(tui_canvas *canvas, int x, int y);

/**
 * Clear a pixel.
 */
void tui_canvas_clear(tui_canvas *canvas, int x, int y);

/**
 * Draw line on canvas (pixel coords).
 */
void tui_canvas_line(tui_canvas *canvas, int x1, int y1, int x2, int y2);

/**
 * Draw circle on canvas.
 */
void tui_canvas_circle(tui_canvas *canvas, int cx, int cy, int radius, bool filled);

/**
 * Render canvas to buffer as characters.
 */
void tui_canvas_render(tui_canvas *canvas, tui_buffer *buf, int x, int y);

/**
 * Free canvas.
 */
void tui_canvas_free(tui_canvas *canvas);
```

### PHP Canvas API

```php
namespace Tui;

/**
 * Create a drawing canvas.
 *
 * @param int $width Width in terminal characters
 * @param int $height Height in terminal characters
 * @param string $mode 'braille'|'block'|'ascii'
 * @return resource Canvas handle
 */
function tui_canvas_create(int $width, int $height, string $mode = 'braille'): mixed;

/**
 * Set a pixel on the canvas.
 */
function tui_canvas_set(mixed $canvas, int $x, int $y): void;

/**
 * Clear a pixel.
 */
function tui_canvas_clear(mixed $canvas, int $x, int $y): void;

/**
 * Draw a line (in pixel coordinates).
 */
function tui_canvas_line(mixed $canvas, int $x1, int $y1, int $x2, int $y2): void;

/**
 * Draw a rectangle.
 */
function tui_canvas_rect(mixed $canvas, int $x, int $y, int $w, int $h, bool $filled = false): void;

/**
 * Draw a circle.
 */
function tui_canvas_circle(mixed $canvas, int $cx, int $cy, int $radius, bool $filled = false): void;

/**
 * Plot a function y = f(x).
 *
 * @param callable $fn Function taking x and returning y
 */
function tui_canvas_plot(mixed $canvas, callable $fn): void;

/**
 * Get pixel resolution of canvas.
 *
 * @return array{width: int, height: int}
 */
function tui_canvas_get_resolution(mixed $canvas): array;

/**
 * Render canvas and get character output.
 *
 * @return array<string> Array of lines
 */
function tui_canvas_render(mixed $canvas): array;
```

### Braille Character Mapping

```c
// Braille Unicode block: U+2800 to U+28FF
// Each character represents a 2x4 dot matrix:
//   ⠁ ⠂    (0,0) (1,0)
//   ⠄ ⠂    (0,1) (1,1)
//   ⠠ ⠐    (0,2) (1,2)
//   ⡀ ⢀    (0,3) (1,3)

static const uint8_t braille_dots[2][4] = {
    {0x01, 0x02, 0x04, 0x40},  // Left column
    {0x08, 0x10, 0x20, 0x80}   // Right column
};

uint32_t pixels_to_braille(uint8_t pixels[2][4]) {
    uint32_t codepoint = 0x2800;  // Braille base
    for (int col = 0; col < 2; col++) {
        for (int row = 0; row < 4; row++) {
            if (pixels[col][row]) {
                codepoint |= braille_dots[col][row];
            }
        }
    }
    return codepoint;
}
```

### Usage in PHP Widgets

The `tui/widgets` package wraps these primitives in convenient PHP classes:

```php
use Tui\Widgets\Canvas;

// Canvas widget uses native tui_canvas_* functions internally
$canvas = Canvas::create(40, 10, CanvasMode::Braille);
$canvas->line(0, 0, 79, 39);  // Calls tui_canvas_line()
$canvas->circle(40, 20, 15);  // Calls tui_canvas_circle()

// Sprite widget uses native tui_sprite_* functions
$sprite = Sprite::create([...]);  // Creates tui_sprite resource
// Animation updates handled by tui_sprite_update() in event loop
```

This hybrid approach provides:
- **Performance**: Compute-intensive drawing in C
- **Flexibility**: Easy sprite/animation definition in PHP
- **Integration**: Native sprites participate in the render/diff cycle

### Animation Utilities

Low-level animation timing and easing functions.

```c
// src/drawing/animation.h

/**
 * Easing function type.
 */
typedef enum {
    TUI_EASE_LINEAR,
    TUI_EASE_IN_QUAD,
    TUI_EASE_OUT_QUAD,
    TUI_EASE_IN_OUT_QUAD,
    TUI_EASE_IN_CUBIC,
    TUI_EASE_OUT_CUBIC,
    TUI_EASE_IN_OUT_CUBIC,
    TUI_EASE_IN_ELASTIC,
    TUI_EASE_OUT_ELASTIC,
    TUI_EASE_OUT_BOUNCE,
} tui_easing;

/**
 * Apply easing function to progress value.
 *
 * @param t Progress value 0.0 to 1.0
 * @param easing Easing function
 * @return Eased value 0.0 to 1.0
 */
double tui_ease(double t, tui_easing easing);

/**
 * Interpolate between two values.
 */
double tui_lerp(double a, double b, double t);

/**
 * Interpolate between two colors.
 */
tui_color tui_lerp_color(tui_color a, tui_color b, double t);
```

```php
namespace Tui;

/**
 * Apply easing function.
 *
 * @param float $t Progress (0.0 - 1.0)
 * @param string $easing Easing name
 * @return float Eased value
 */
function tui_ease(float $t, string $easing = 'linear'): float;

/**
 * Linear interpolation.
 */
function tui_lerp(float $a, float $b, float $t): float;

/**
 * Interpolate between colors.
 *
 * @param string $colorA Hex color
 * @param string $colorB Hex color
 * @param float $t Progress
 * @return string Interpolated hex color
 */
function tui_lerp_color(string $colorA, string $colorB, float $t): string;

/**
 * Generate gradient colors.
 *
 * @param array<string> $colors Array of hex colors
 * @param int $steps Number of output colors
 * @return array<string> Interpolated colors
 */
function tui_gradient(array $colors, int $steps): array;
```

### Text Utilities

Text measurement and wrapping (requires Unicode width tables).

```c
// src/text/measure.h

/**
 * Get display width of a Unicode codepoint.
 * Uses wcwidth() internally.
 */
int tui_char_width(uint32_t codepoint);

/**
 * Get display width of a UTF-8 string.
 */
int tui_string_width(const char *str);

/**
 * Wrap text to fit within width.
 *
 * @param text Input text
 * @param width Max width in columns
 * @param wrap_mode 'word' or 'char'
 * @return Array of lines
 */
char** tui_wrap_text(const char *text, int width, const char *wrap_mode);

/**
 * Truncate text with ellipsis.
 */
char* tui_truncate(const char *text, int width, const char *ellipsis);
```

```php
namespace Tui;

/**
 * Get display width of string (handles Unicode, emoji, CJK).
 */
function tui_string_width(string $text): int;

/**
 * Wrap text to width.
 *
 * @return array<string> Lines
 */
function tui_wrap_text(string $text, int $width, string $mode = 'word'): array;

/**
 * Truncate with ellipsis.
 */
function tui_truncate(string $text, int $width, string $ellipsis = '…'): string;

/**
 * Pad string to width (accounting for Unicode width).
 */
function tui_pad(string $text, int $width, string $align = 'left', string $char = ' '): string;
```

### Progress/Animation Rendering

Native progress bar and busy indicator rendering.

```c
// src/drawing/progress.h

typedef enum {
    TUI_PROGRESS_BAR,       // Standard progress bar
    TUI_PROGRESS_SPINNER,   // Spinning indicator
    TUI_BUSY_PULSE,         // Bouncing block
    TUI_BUSY_SNAKE,         // Growing snake
    TUI_BUSY_GRADIENT,      // Moving gradient
    TUI_BUSY_WAVE,          // Wave pattern
    TUI_BUSY_SHIMMER,       // Shimmer effect
    TUI_BUSY_RAINBOW,       // Rainbow gradient
} tui_progress_style;

/**
 * Render progress bar to buffer.
 */
void tui_render_progress(
    tui_buffer *buf,
    int x, int y, int width,
    double progress,        // 0.0 - 1.0
    const char *filled,     // Fill character
    const char *empty,      // Empty character
    tui_style *style
);

/**
 * Render busy/indeterminate bar to buffer.
 */
void tui_render_busy(
    tui_buffer *buf,
    int x, int y, int width,
    int frame,              // Animation frame
    tui_progress_style style,
    tui_color *gradient,    // Gradient colors (for gradient/rainbow)
    int gradient_count,
    tui_style *base_style
);

/**
 * Render spinner character.
 */
void tui_render_spinner(
    tui_buffer *buf,
    int x, int y,
    int frame,
    const char **frames,    // Spinner frame chars
    int frame_count,
    tui_style *style
);
```

```php
namespace Tui;

/**
 * Render a progress bar.
 */
function tui_render_progress(
    int $x, int $y, int $width,
    float $progress,
    string $filled = '█',
    string $empty = '░',
    ?array $style = null
): void;

/**
 * Render an animated busy bar.
 */
function tui_render_busy(
    int $x, int $y, int $width,
    int $frame,
    string $style = 'gradient',
    ?array $gradientColors = null
): void;

/**
 * Get spinner frame for current time.
 *
 * @param array<string> $frames Spinner characters
 * @param int $intervalMs Frame interval
 * @return string Current frame character
 */
function tui_spinner_frame(array $frames, int $intervalMs = 80): string;
```

### Table Rendering

Table layout and rendering (compute-intensive column width calculation).

```c
// src/drawing/table.h

typedef struct {
    char **headers;
    int header_count;
    char ***rows;
    int row_count;
    int *column_widths;     // Calculated widths
    bool *column_align_right;
} tui_table;

/**
 * Calculate optimal column widths for table.
 */
void tui_table_layout(tui_table *table, int max_width);

/**
 * Render table to buffer.
 */
void tui_render_table(
    tui_buffer *buf,
    tui_table *table,
    int x, int y,
    const char *border_style,  // 'single', 'double', 'round', 'none'
    tui_style *header_style,
    tui_style *cell_style
);
```

```php
namespace Tui;

/**
 * Calculate column widths for table data.
 *
 * @param array<string> $headers
 * @param array<array<string>> $rows
 * @param int $maxWidth Max total width
 * @return array<int> Column widths
 */
function tui_table_layout(array $headers, array $rows, int $maxWidth): array;

/**
 * Render table directly to buffer.
 */
function tui_render_table(
    array $headers,
    array $rows,
    int $x, int $y,
    string $borderStyle = 'single',
    ?array $headerStyle = null,
    ?array $cellStyle = null
): void;
```

### Summary: Extension vs Composer

```
┌─────────────────────────────────────────────────────────────────────────┐
│                          ext-tui (C Extension)                           │
├─────────────────────────────────────────────────────────────────────────┤
│  DRAWING           │  ANIMATION         │  TEXT              │  WIDGETS │
│  ─────────────────────────────────────────────────────────────────────  │
│  tui_draw_line()   │  tui_ease()        │  tui_string_width()│ Table    │
│  tui_draw_rect()   │  tui_lerp()        │  tui_wrap_text()   │ layout   │
│  tui_draw_circle() │  tui_lerp_color()  │  tui_truncate()    │          │
│  tui_fill_*()      │  tui_gradient()    │  tui_pad()         │ Progress │
│  tui_canvas_*()    │  tui_sprite_*()    │                    │ render   │
│  tui_blit_sprite() │  Frame timing      │                    │          │
│  Braille mapping   │  Collision detect  │                    │ Spinner  │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                        tui/widgets (Composer Package)                    │
├─────────────────────────────────────────────────────────────────────────┤
│  WIDGET CLASSES    │  CONTENT           │  THEMING           │  LIBRARY │
│  ─────────────────────────────────────────────────────────────────────  │
│  Canvas::create()  │  Sprites::cat()    │  Theme definitions │  Built-in│
│  Sprite::create()  │  Sprites::fire()   │  Color palettes    │  sprites │
│  Table::create()   │  ASCII art data    │  Style presets     │          │
│  ProgressBar       │  Markdown parse    │                    │  Form    │
│  BusyBar           │  Syntax highlight  │                    │  fields  │
│  Shape, Line       │                    │                    │          │
│  TodoList          │                    │                    │  Select  │
│  Collapsible       │                    │                    │  widgets │
└─────────────────────────────────────────────────────────────────────────┘
```

The Composer widgets call extension functions for performance-critical operations:

```php
// Example: Canvas widget wrapping extension functions
class Canvas implements Component
{
    private mixed $handle;  // C extension resource

    public static function create(int $width, int $height): self
    {
        $canvas = new self();
        $canvas->handle = tui_canvas_create($width, $height, 'braille');
        return $canvas;
    }

    public function line(int $x1, int $y1, int $x2, int $y2): self
    {
        tui_canvas_line($this->handle, $x1, $y1, $x2, $y2);
        return $this;
    }

    public function render(): Component
    {
        $lines = tui_canvas_render($this->handle);
        return Box::column(array_map(fn($l) => Text::create($l), $lines));
    }
}
```

---

## Implementation Details

### Event Loop

```c
// src/event_loop/loop.c

void ink_run_loop(ink_app *app) {
    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    // Initial render
    do_render(app);

    while (app->running) {
        // Handle pending signals
        if (app->resize_pending) {
            app->resize_pending = 0;
            update_terminal_size(app);
            app->render_scheduled = 1;
        }

        if (app->exit_requested) {
            break;
        }

        // Poll for input with timeout for render throttling
        int timeout = app->render_scheduled ? 0 : 16;  // ~60fps
        int ret = poll(fds, 1, timeout);

        if (ret > 0 && (fds[0].revents & POLLIN)) {
            // Read input
            char buf[64];
            ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));

            if (n > 0) {
                ink_key_event *event = parse_key(buf, n);
                dispatch_input(app, buf, n, event);
                free_key_event(event);
            }
        }

        // Render if scheduled
        if (app->render_scheduled) {
            app->render_scheduled = 0;
            do_render(app);
        }
    }

    cleanup_terminal(app);
}

void do_render(ink_app *app) {
    // Reset hook index
    app->state_index = 0;

    // Call PHP render function
    zval new_tree;
    ZVAL_NULL(&new_tree);

    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    fci.size = sizeof(fci);
    fci.retval = &new_tree;
    fci.param_count = 0;
    fci.params = NULL;
    ZVAL_COPY(&fci.function_name, &app->render_fn);

    zend_call_function(&fci, &fcc);

    if (Z_TYPE(new_tree) != IS_OBJECT) {
        php_error_docref(NULL, E_WARNING, "Render function must return a Node");
        return;
    }

    // Convert PHP node to C node tree
    ink_node *new_root = php_node_to_ink_node(&new_tree);

    // Reconcile
    update_op *ops = reconcile(app->root, new_root);
    apply_updates(app, ops);
    free_updates(ops);

    // Update root
    if (app->root != new_root) {
        free_node_tree(app->root);
        app->root = new_root;
    }

    // Calculate layout
    int width, height;
    get_terminal_size(&width, &height);
    YGNodeCalculateLayout(app->root->yoga_node, width, height, YGDirectionLTR);

    // Render to buffer
    ink_buffer *new_buffer = create_buffer(width, height);
    render_node(app->root, new_buffer, 0, 0);

    // Output diff
    output_diff(app->output_buffer, new_buffer, stdout);

    // Swap buffers
    free_buffer(app->output_buffer);
    app->output_buffer = new_buffer;

    zval_ptr_dtor(&new_tree);
}
```

### Terminal Control

```c
// src/terminal/terminal.c

int enter_raw_mode(ink_app *app) {
    if (app->is_raw_mode) return 0;

    if (tcgetattr(STDIN_FILENO, &app->orig_termios) < 0) {
        return -1;
    }

    struct termios raw = app->orig_termios;

    // Input modes
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // Output modes
    raw.c_oflag &= ~(OPOST);

    // Control modes
    raw.c_cflag |= (CS8);

    // Local modes
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    // Control characters
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) < 0) {
        return -1;
    }

    app->is_raw_mode = 1;

    // Hide cursor
    hide_cursor(stdout);

    return 0;
}

int exit_raw_mode(ink_app *app) {
    if (!app->is_raw_mode) return 0;

    // Show cursor
    show_cursor(stdout);

    // Restore terminal
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &app->orig_termios) < 0) {
        return -1;
    }

    app->is_raw_mode = 0;
    return 0;
}

void get_terminal_size(int *width, int *height) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) < 0) {
        *width = 80;
        *height = 24;
    } else {
        *width = ws.ws_col;
        *height = ws.ws_row;
    }
}

void enter_alternate_screen(ink_app *app) {
    if (app->is_alternate_screen) return;

    fprintf(stdout, "\x1b[?1049h");
    fflush(stdout);

    app->is_alternate_screen = 1;
}

void exit_alternate_screen(ink_app *app) {
    if (!app->is_alternate_screen) return;

    fprintf(stdout, "\x1b[?1049l");
    fflush(stdout);

    app->is_alternate_screen = 0;
}
```

### Signal Handling

```c
// src/event_loop/signals.c

static ink_app *signal_app = NULL;

void handle_sigwinch(int sig) {
    (void)sig;
    if (signal_app) {
        signal_app->resize_pending = 1;
    }
}

void handle_sigint(int sig) {
    (void)sig;
    if (signal_app) {
        signal_app->exit_requested = 1;
    }
}

void handle_sigterm(int sig) {
    (void)sig;
    if (signal_app) {
        signal_app->exit_requested = 1;
    }
}

void setup_signal_handlers(ink_app *app) {
    signal_app = app;

    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    // Window resize
    sa.sa_handler = handle_sigwinch;
    sigaction(SIGWINCH, &sa, NULL);

    // Interrupt (Ctrl+C) - only if exitOnCtrlC is false
    if (!app->exit_on_ctrl_c) {
        sa.sa_handler = handle_sigint;
        sigaction(SIGINT, &sa, NULL);
    }

    // Terminate
    sa.sa_handler = handle_sigterm;
    sigaction(SIGTERM, &sa, NULL);
}

void restore_signal_handlers(void) {
    signal(SIGWINCH, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal_app = NULL;
}
```

### PHP Class Bindings

```c
// src/render.c - Top-level render function

PHP_FUNCTION(render) {
    zval *node_or_callback;
    zval *options = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_ZVAL(node_or_callback)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(options)
    ZEND_PARSE_PARAMETERS_END();

    // Create app instance
    ink_app *app = ink_app_create();

    // Parse options
    if (options) {
        zval *val;
        if ((val = zend_hash_str_find(Z_ARRVAL_P(options), "fullscreen", 10))) {
            app->use_alternate_screen = zend_is_true(val);
        }
        if ((val = zend_hash_str_find(Z_ARRVAL_P(options), "exitOnCtrlC", 11))) {
            app->exit_on_ctrl_c = zend_is_true(val);
        }
        if ((val = zend_hash_str_find(Z_ARRVAL_P(options), "debug", 5))) {
            app->debug_mode = zend_is_true(val);
        }
    }

    // Store render callback (receives App context)
    ZVAL_COPY(&app->render_fn, node_or_callback);

    // Enter raw mode and start loop
    enter_raw_mode(app);
    if (app->use_alternate_screen) {
        enter_alternate_screen(app);
    }
    setup_signal_handlers(app);

    // Start in background thread or return Instance immediately
    app->running = 1;

    // Return Instance object for control
    object_init_ex(return_value, ink_instance_ce);
    ink_instance_object *instance = ink_instance_from_obj(Z_OBJ_P(return_value));
    instance->app = app;
}

// src/app.c - App context (passed to render callback)

zend_class_entry *ink_app_ce;
zend_object_handlers ink_app_handlers;

PHP_METHOD(App, exit) {
    zval *error = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_OBJECT_OF_CLASS_OR_NULL(error, zend_ce_throwable)
    ZEND_PARSE_PARAMETERS_END();

    ink_app_object *intern = ink_app_from_obj(Z_OBJ_P(ZEND_THIS));
    intern->app->exit_requested = 1;

    if (error) {
        ZVAL_COPY(&intern->app->exit_error, error);
    }
}

PHP_METHOD(App, useState) {
    zval *initial;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(initial)
    ZEND_PARSE_PARAMETERS_END();

    ink_app_object *intern = ink_app_from_obj(Z_OBJ_P(ZEND_THIS));
    ink_app *app = intern->app;

    // Get or create state slot (tracked by call order)
    int index = app->state_index++;

    if (index >= app->state_count) {
        // First render - initialize state
        app->states = erealloc(app->states, (index + 1) * sizeof(ink_state));
        app->state_count = index + 1;

        ink_state *state = &app->states[index];
        ZVAL_COPY(&state->value, initial);
        state->app = app;
        state->index = index;

        // Create setter closure that triggers re-render
        create_state_setter(state);
    }

    ink_state *state = &app->states[index];

    // Return [value, setter]
    array_init(return_value);
    Z_TRY_ADDREF(state->value);
    add_next_index_zval(return_value, &state->value);
    Z_TRY_ADDREF(state->setter);
    add_next_index_zval(return_value, &state->setter);
}

PHP_METHOD(App, useInput) {
    zval *handler;
    zval *options = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_FUNC(handler)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(options)
    ZEND_PARSE_PARAMETERS_END();

    ink_app_object *intern = ink_app_from_obj(Z_OBJ_P(ZEND_THIS));

    // Check isActive option
    zend_bool is_active = 1;
    if (options) {
        zval *val = zend_hash_str_find(Z_ARRVAL_P(options), "isActive", 8);
        if (val) {
            is_active = zend_is_true(val);
        }
    }

    if (is_active) {
        ink_register_input_handler(intern->app, handler);
    }
}

PHP_METHOD(App, useStderr) {
    ZEND_PARSE_PARAMETERS_NONE();

    ink_app_object *intern = ink_app_from_obj(Z_OBJ_P(ZEND_THIS));

    // Return StderrContext object
    object_init_ex(return_value, ink_stderr_context_ce);
    ink_stderr_context_object *ctx = ink_stderr_context_from_obj(Z_OBJ_P(return_value));
    ctx->app = intern->app;
}

// Method registration
static const zend_function_entry ink_app_methods[] = {
    PHP_ME(App, exit, arginfo_app_exit, ZEND_ACC_PUBLIC)
    PHP_ME(App, useState, arginfo_app_use_state, ZEND_ACC_PUBLIC)
    PHP_ME(App, useInput, arginfo_app_use_input, ZEND_ACC_PUBLIC)
    PHP_ME(App, useFocus, arginfo_app_use_focus, ZEND_ACC_PUBLIC)
    PHP_ME(App, useFocusManager, arginfo_app_use_focus_manager, ZEND_ACC_PUBLIC)
    PHP_ME(App, useStdin, arginfo_app_use_stdin, ZEND_ACC_PUBLIC)
    PHP_ME(App, useStdout, arginfo_app_use_stdout, ZEND_ACC_PUBLIC)
    PHP_ME(App, useStderr, arginfo_app_use_stderr, ZEND_ACC_PUBLIC)
    PHP_ME(App, rerender, arginfo_app_rerender, ZEND_ACC_PUBLIC)
    PHP_ME(App, getTerminalSize, arginfo_app_get_terminal_size, ZEND_ACC_PUBLIC)
    PHP_ME(App, measureElement, arginfo_app_measure_element, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

// src/instance.c - Instance control object

static const zend_function_entry ink_instance_methods[] = {
    PHP_ME(Instance, rerender, arginfo_instance_rerender, ZEND_ACC_PUBLIC)
    PHP_ME(Instance, unmount, arginfo_instance_unmount, ZEND_ACC_PUBLIC)
    PHP_ME(Instance, waitUntilExit, arginfo_instance_wait, ZEND_ACC_PUBLIC)
    PHP_ME(Instance, clear, arginfo_instance_clear, ZEND_ACC_PUBLIC)
    PHP_ME(Instance, cleanup, arginfo_instance_cleanup, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
```

---

## Build System

### config.m4

```m4
PHP_ARG_ENABLE([ink],
  [whether to enable ink support],
  [AS_HELP_STRING([--enable-ink],
    [Enable ink terminal UI support])],
  [no])

if test "$PHP_TUI" != "no"; then
  dnl Check for C++ compiler (needed for Yoga)
  AC_PROG_CXX

  dnl Yoga sources
  YOGA_SOURCES="yoga/yoga/Yoga.cpp yoga/yoga/YGNode.cpp yoga/yoga/YGStyle.cpp \
                yoga/yoga/YGValue.cpp yoga/yoga/YGConfig.cpp yoga/yoga/YGLayout.cpp \
                yoga/yoga/YGEnums.cpp yoga/yoga/Utils.cpp yoga/yoga/log.cpp"

  dnl Extension sources
  INK_SOURCES="php_tui.c \
               src/app.c \
               src/components/box.c \
               src/components/text.c \
               src/components/newline.c \
               src/components/spacer.c \
               src/components/static.c \
               src/components/transform.c \
               src/hooks/state.c \
               src/hooks/input.c \
               src/hooks/focus.c \
               src/reconciler/reconciler.c \
               src/reconciler/node.c \
               src/layout/yoga_wrapper.c \
               src/layout/styles.c \
               src/render/renderer.c \
               src/render/ansi.c \
               src/render/output.c \
               src/render/border.c \
               src/terminal/terminal.c \
               src/terminal/input.c \
               src/event_loop/loop.c \
               src/event_loop/signals.c \
               src/util/unicode.c \
               src/util/hash.c \
               src/util/memory.c"

  PHP_REQUIRE_CXX()
  PHP_ADD_LIBRARY(stdc++, 1, INK_SHARED_LIBADD)
  PHP_SUBST(INK_SHARED_LIBADD)

  PHP_NEW_EXTENSION(ink, $INK_SOURCES $YOGA_SOURCES, $ext_shared,, -std=c11 -Wall -Wextra)
  PHP_ADD_BUILD_DIR($ext_builddir/src)
  PHP_ADD_BUILD_DIR($ext_builddir/src/components)
  PHP_ADD_BUILD_DIR($ext_builddir/src/hooks)
  PHP_ADD_BUILD_DIR($ext_builddir/src/reconciler)
  PHP_ADD_BUILD_DIR($ext_builddir/src/layout)
  PHP_ADD_BUILD_DIR($ext_builddir/src/render)
  PHP_ADD_BUILD_DIR($ext_builddir/src/terminal)
  PHP_ADD_BUILD_DIR($ext_builddir/src/event_loop)
  PHP_ADD_BUILD_DIR($ext_builddir/src/util)
  PHP_ADD_BUILD_DIR($ext_builddir/yoga/yoga)

  PHP_ADD_INCLUDE($ext_srcdir/yoga)
fi
```

### php_tui.h

```c
#ifndef PHP_TUI_H
#define PHP_TUI_H

extern zend_module_entry ink_module_entry;
#define phpext_ink_ptr &ink_module_entry

#define PHP_TUI_VERSION "0.1.0"

#ifdef PHP_WIN32
# define PHP_TUI_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
# define PHP_TUI_API __attribute__ ((visibility("default")))
#else
# define PHP_TUI_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#include "Zend/zend_types.h"

// Module functions
PHP_MINIT_FUNCTION(ink);
PHP_MSHUTDOWN_FUNCTION(ink);
PHP_RINIT_FUNCTION(ink);
PHP_RSHUTDOWN_FUNCTION(ink);
PHP_MINFO_FUNCTION(ink);

// Class entries
extern zend_class_entry *ink_instance_ce;
extern zend_class_entry *ink_app_ce;
extern zend_class_entry *ink_node_ce;
extern zend_class_entry *ink_box_ce;
extern zend_class_entry *ink_text_ce;
extern zend_class_entry *ink_newline_ce;
extern zend_class_entry *ink_spacer_ce;
extern zend_class_entry *ink_static_ce;
extern zend_class_entry *ink_transform_ce;
extern zend_class_entry *ink_key_ce;
extern zend_class_entry *ink_focus_ce;
extern zend_class_entry *ink_focus_manager_ce;
extern zend_class_entry *ink_stdin_context_ce;
extern zend_class_entry *ink_stdout_context_ce;
extern zend_class_entry *ink_stderr_context_ce;

// Top-level function
PHP_FUNCTION(render);

#endif /* PHP_TUI_H */
```

### php_tui.c

```c
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_tui.h"

#include "src/app.h"
#include "src/components/box.h"
#include "src/components/text.h"
// ... other includes

zend_class_entry *ink_instance_ce;
zend_class_entry *ink_app_ce;
zend_class_entry *ink_node_ce;
zend_class_entry *ink_box_ce;
zend_class_entry *ink_text_ce;
zend_class_entry *ink_key_ce;
zend_class_entry *ink_focus_ce;
zend_class_entry *ink_focus_manager_ce;
zend_class_entry *ink_stdin_context_ce;
zend_class_entry *ink_stdout_context_ce;
zend_class_entry *ink_stderr_context_ce;
// ... other class entries

// Top-level functions
static const zend_function_entry ink_functions[] = {
    PHP_FE(render, arginfo_render)
    PHP_FE_END
};

PHP_MINIT_FUNCTION(ink) {
    // Register classes
    ink_register_instance_class();
    ink_register_app_class();
    ink_register_node_class();
    ink_register_box_class();
    ink_register_text_class();
    ink_register_newline_class();
    ink_register_spacer_class();
    ink_register_static_class();
    ink_register_transform_class();
    ink_register_key_class();
    ink_register_focus_class();
    ink_register_focus_manager_class();
    ink_register_stdin_context_class();
    ink_register_stdout_context_class();
    ink_register_stderr_context_class();

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ink) {
    return SUCCESS;
}

PHP_RINIT_FUNCTION(ink) {
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ink) {
    return SUCCESS;
}

PHP_MINFO_FUNCTION(ink) {
    php_info_print_table_start();
    php_info_print_table_header(2, "ink support", "enabled");
    php_info_print_table_row(2, "Version", PHP_TUI_VERSION);
    php_info_print_table_row(2, "Yoga version", "3.1.0");
    php_info_print_table_end();
}

zend_module_entry ink_module_entry = {
    STANDARD_MODULE_HEADER,
    "ink",
    ink_functions,              /* Top-level functions (render) */
    PHP_MINIT(ink),
    PHP_MSHUTDOWN(ink),
    PHP_RINIT(ink),
    PHP_RSHUTDOWN(ink),
    PHP_MINFO(ink),
    PHP_TUI_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_INK
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(ink)
#endif
```

---

## Testing

### PHPUnit Tests

```php
// tests/BoxTest.php
use PHPUnit\Framework\TestCase;
use Ink\{Box, Text};

class BoxTest extends TestCase {
    public function testCreateEmptyBox(): void {
        $box = Box::create();
        $this->assertInstanceOf(Box::class, $box);
    }

    public function testBoxWithStyles(): void {
        $box = Box::create([
            'padding' => 1,
            'flexDirection' => 'column',
        ]);

        $this->assertEquals(1, $box->getStyle('padding'));
        $this->assertEquals('column', $box->getStyle('flexDirection'));
    }

    public function testBoxWithChildren(): void {
        $box = Box::create()
            ->child(Text::create('Hello'))
            ->child(Text::create('World'));

        $children = $box->getChildren();
        $this->assertCount(2, $children);
    }
}
```

### PHPT Tests

```php
--TEST--
Basic Box creation
--EXTENSIONS--
ink
--FILE--
<?php
$box = Ink\Box::create(['padding' => 1]);
var_dump($box instanceof Ink\Box);
var_dump($box instanceof Ink\Node);
?>
--EXPECT--
bool(true)
bool(true)
```

```php
--TEST--
useState hook maintains state across renders
--EXTENSIONS--
ink
--FILE--
<?php
$app = new Ink\App();
$renderCount = 0;

$app->render(function() use ($app, &$renderCount) {
    $renderCount++;
    [$count, $setCount] = $app->useState(0);

    if ($renderCount === 1) {
        echo "Initial: $count\n";
        $setCount(5);
    } else {
        echo "After: $count\n";
        $app->exit();
    }

    return Ink\Box::create();
});

// Simulate run (in tests we use a mock)
$app->runOnce();
$app->runOnce();
?>
--EXPECT--
Initial: 0
After: 5
```

---

## Roadmap

### Phase 1: Core (MVP)
- [ ] Extension skeleton and build system
- [ ] Yoga integration
- [ ] Box and Text components
- [ ] Basic styling (colors, dimensions)
- [ ] Terminal control (raw mode)
- [ ] Simple event loop
- [ ] useState hook

### Phase 2: Components
- [ ] Newline, Spacer, Static, Transform
- [ ] Border rendering
- [ ] Text wrapping
- [ ] useInput hook
- [ ] Key parsing

### Phase 3: Advanced
- [ ] Reconciler with diffing
- [ ] Output diffing
- [ ] Focus management
- [ ] useFocus, useFocusManager
- [ ] Fullscreen mode

### Phase 4: Polish
- [ ] Performance optimization
- [ ] Memory pooling
- [ ] Unicode width handling
- [ ] Mouse support (optional)
- [ ] Documentation
- [ ] Examples

### Phase 5: Ecosystem
- [ ] TextInput component
- [ ] Select component
- [ ] Spinner component
- [ ] Progress bar component
- [ ] PECL release

---

## References

- [Ink (JavaScript)](https://github.com/vadimdemedes/ink)
- [Yoga Layout](https://yogalayout.dev/)
- [React Reconciler](https://github.com/facebook/react/tree/main/packages/react-reconciler)
- [ANSI Escape Codes](https://en.wikipedia.org/wiki/ANSI_escape_code)
- [PHP Internals Book](https://www.phpinternalsbook.com/)
- [termios Manual](https://man7.org/linux/man-pages/man3/termios.3.html)
