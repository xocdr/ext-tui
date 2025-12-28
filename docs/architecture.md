# ext-tui Architecture

This document describes the internal architecture of the ext-tui PHP C extension.

## Overview

ext-tui is a PHP C extension that provides terminal UI capabilities. It follows a component-based architecture with a virtual DOM, flexbox layout engine, and efficient terminal rendering.

```
┌──────────────────────────────────────────────────────────┐
│                      PHP Layer                           │
│  ┌─────────┐ ┌─────────┐ ┌────────────┐ ┌─────────────┐ │
│  │ TuiBox  │ │ TuiText │ │TuiInstance │ │   TuiKey    │ │
│  └────┬────┘ └────┬────┘ └─────┬──────┘ └──────┬──────┘ │
└───────┼──────────┼─────────────┼───────────────┼────────┘
        │          │             │               │
┌───────┼──────────┼─────────────┼───────────────┼────────┐
│       ▼          ▼             ▼               ▼        │
│  ┌─────────────────────────────────────────────────┐    │
│  │                   tui.c                          │    │
│  │         PHP ↔ C Bridge / Object Mapping          │    │
│  └────────────────────┬────────────────────────────┘    │
│                       │                                  │
│  ┌────────────────────▼────────────────────────────┐    │
│  │                 src/app/                         │    │
│  │  App State | Event Dispatch | Render Orchestration│   │
│  └──────┬────────────┬────────────────┬────────────┘    │
│         │            │                │                  │
│    ┌────▼───┐  ┌─────▼─────┐    ┌────▼────┐             │
│    │  node/ │  │  event/   │    │ render/ │             │
│    │        │  │           │    │         │             │
│    │ VirtualDOM│ Event Loop│    │ Buffer  │             │
│    │  Yoga   │ │  Input    │    │ Output  │             │
│    └────────┘  └───────────┘    └────┬────┘             │
│                                      │                   │
│    ┌─────────────────────────────────▼──────────────┐   │
│    │                 terminal/                       │   │
│    │     Raw Mode | ANSI Codes | Size Detection     │   │
│    └────────────────────────────────────────────────┘   │
│                          C Layer                         │
└──────────────────────────────────────────────────────────┘
```

## Directory Structure

```
ext-tui/
├── tui.c                  # PHP extension entry point
├── php_tui.h              # Extension header
├── config.m4              # Build configuration
├── src/
│   ├── app/
│   │   ├── app.c          # Application state management
│   │   └── app.h
│   ├── event/
│   │   ├── loop.c         # Event loop (poll-based)
│   │   ├── loop.h
│   │   ├── input.c        # Keyboard input parsing
│   │   └── input.h
│   ├── node/
│   │   ├── node.c         # Virtual DOM nodes
│   │   ├── node.h
│   │   ├── reconciler.c   # Tree diff/patch
│   │   └── reconciler.h
│   ├── render/
│   │   ├── buffer.c       # Cell buffer
│   │   ├── buffer.h
│   │   ├── output.c       # Terminal output
│   │   └── output.h
│   ├── terminal/
│   │   ├── terminal.c     # Raw mode, size detection
│   │   ├── terminal.h
│   │   ├── ansi.c         # ANSI escape codes
│   │   └── ansi.h
│   ├── text/
│   │   ├── measure.c      # Text width measurement
│   │   ├── measure.h
│   │   ├── wrap.c         # Text wrapping
│   │   └── wrap.h
│   └── yoga/              # Facebook Yoga layout engine
│       └── ...            # Vendored C++ source
├── tests/                 # PHP test files (.phpt)
└── docs/                  # Documentation
```

## Core Components

### 1. tui.c - PHP Binding Layer

The main extension file that bridges PHP and C:

- **Class Definitions**: TuiBox, TuiText, TuiInstance, TuiKey
- **Function Exports**: tui_render(), tui_set_input_handler(), etc.
- **Object Mapping**: Converts PHP objects to C structs and back
- **Memory Management**: Custom object handlers for TuiInstance

Key patterns:
```c
// Custom object structure for proper cleanup
typedef struct {
    tui_app *app;
    zend_object std;
} tui_instance_object;

// Object destructor - prevents memory leaks
static void tui_instance_free_object(zend_object *obj) {
    tui_instance_object *intern = tui_instance_from_obj(obj);
    if (intern->app) {
        tui_app_stop(intern->app);
        tui_app_destroy(intern->app);
    }
    zend_object_std_dtor(&intern->std);
}
```

### 2. src/app/ - Application State

Manages the lifecycle of a TUI application:

- **tui_app struct**: Holds all application state
- **Callbacks**: Component, input, focus, resize handlers
- **Render loop**: Coordinates layout and output
- **Focus management**: Tracks and navigates focusable elements

```c
typedef struct {
    /* Terminal state */
    int fullscreen;
    int running;
    int width, height;

    /* Callbacks */
    zend_fcall_info component_fci;
    zend_fcall_info input_fci;
    zend_fcall_info focus_fci;
    zend_fcall_info resize_fci;

    /* Virtual DOM */
    tui_node *root_node;
    tui_node *focused_node;

    /* Render state */
    tui_buffer *buffer;
    tui_output *output;
    tui_loop *loop;
} tui_app;
```

### 3. src/event/ - Event System

#### Event Loop (loop.c)

Poll-based event loop using `poll()`:

```c
// Simplified event loop
while (running) {
    struct pollfd fds[1] = {{STDIN_FILENO, POLLIN, 0}};
    int ret = poll(fds, 1, timeout_ms);

    if (ret > 0 && (fds[0].revents & POLLIN)) {
        char buf[64];
        int n = read(STDIN_FILENO, buf, sizeof(buf));
        if (n > 0) {
            input_callback(buf, n, userdata);
        }
    }

    // Handle SIGWINCH for resize
    if (resize_pending) {
        resize_callback(new_width, new_height, userdata);
    }
}
```

#### Input Parser (input.c)

Parses terminal escape sequences into structured events:

- Single characters (a-z, 0-9)
- Control characters (Ctrl+A through Ctrl+Z)
- Escape sequences (arrows, function keys)
- UTF-8 multi-byte characters
- Modifier detection (Shift, Ctrl, Alt)

```c
// Key event structure
typedef struct {
    char key[8];        // UTF-8 character
    bool upArrow, downArrow, leftArrow, rightArrow;
    bool enter, escape, backspace, delete, tab;
    bool ctrl, meta, shift;
} tui_key_event;
```

### 4. src/node/ - Virtual DOM

#### Node Structure

```c
typedef struct tui_node {
    tui_node_type type;     // BOX or TEXT
    char *text;             // For TEXT nodes
    char *key;              // Identity for reconciler

    tui_style style;        // Colors, bold, etc.
    tui_border_style border_style;
    tui_wrap_mode wrap_mode;

    int focusable, focused;

    YGNodeRef yoga_node;    // Yoga layout node

    struct tui_node *parent;
    struct tui_node **children;
    int child_count;

    float x, y, width, height;  // Computed layout
} tui_node;
```

#### Layout Engine (Yoga)

Uses Facebook's Yoga library for flexbox layout:

```c
// Create node with Yoga
tui_node *node = tui_node_create_box();
YGNodeStyleSetFlexDirection(node->yoga_node, YGFlexDirectionColumn);
YGNodeStyleSetPadding(node->yoga_node, YGEdgeAll, 1);

// Calculate layout
YGNodeCalculateLayout(root->yoga_node, width, height, YGDirectionLTR);

// Copy results
node->x = YGNodeLayoutGetLeft(node->yoga_node);
node->y = YGNodeLayoutGetTop(node->yoga_node);
node->width = YGNodeLayoutGetWidth(node->yoga_node);
node->height = YGNodeLayoutGetHeight(node->yoga_node);
```

### 5. src/render/ - Rendering

#### Buffer (buffer.c)

Character cell buffer for double-buffering:

```c
typedef struct {
    uint32_t codepoint;     // Unicode character
    tui_style style;        // Colors and attributes
    uint8_t dirty;          // Needs redraw
} tui_cell;

typedef struct {
    tui_cell *cells;
    int width, height;
} tui_buffer;
```

#### Output (output.c)

Generates terminal output:

- Cursor positioning
- ANSI color codes
- Style attributes
- Alternate screen buffer

```c
// Render buffer to terminal
void tui_output_render(tui_output *out, tui_buffer *buf) {
    for (int y = 0; y < buf->height; y++) {
        for (int x = 0; x < buf->width; x++) {
            tui_cell *cell = &buf->cells[y * buf->width + x];
            if (cell->dirty) {
                move_cursor(x, y);
                apply_style(&cell->style);
                write_char(cell->codepoint);
                cell->dirty = 0;
            }
        }
    }
    flush_output();
}
```

### 6. src/terminal/ - Terminal Control

#### Raw Mode (terminal.c)

Configures terminal for character-by-character input:

```c
void tui_terminal_enable_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);

    // Disable canonical mode, echo, signals
    raw.c_lflag &= ~(ICANON | ECHO | ISIG);
    raw.c_iflag &= ~(IXON);  // Disable Ctrl+S/Q
    raw.c_cc[VMIN] = 0;      // Non-blocking
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
```

#### ANSI Codes (ansi.c)

Generates ANSI escape sequences:

```c
// Safe buffer-based API
void tui_ansi_cursor_move(char *buf, size_t *len, int x, int y) {
    *len = snprintf(buf, ANSI_BUF_SIZE, "\x1b[%d;%dH", y + 1, x + 1);
}

void tui_ansi_fg_rgb(char *buf, size_t *len, uint8_t r, uint8_t g, uint8_t b) {
    *len = snprintf(buf, ANSI_BUF_SIZE, "\x1b[38;2;%d;%d;%dm", r, g, b);
}
```

### 7. src/text/ - Text Utilities

#### Width Measurement (measure.c)

Calculates display width of Unicode text:

```c
int tui_string_width(const char *text) {
    int width = 0;
    while (*text) {
        uint32_t cp = decode_utf8(&text);
        width += char_width(cp);  // 0, 1, or 2 cells
    }
    return width;
}
```

#### Text Wrapping (wrap.c)

Wraps text to fit within a width:

```c
tui_wrapped_text* tui_wrap_text(const char *text, int width, tui_wrap_mode mode) {
    // Word wrap, character wrap, or hybrid
    // Returns array of lines
}
```

## Data Flow

### Render Cycle

```
1. PHP calls tui_render($component)
   │
2. tui.c creates tui_app, stores callback
   │
3. Initial component call → PHP returns TuiBox/TuiText tree
   │
4. php_to_tui_node() converts PHP objects to tui_node tree
   │
5. Yoga calculates layout
   │
6. render_node_to_buffer() writes to cell buffer
   │
7. tui_output_render() writes to terminal
   │
8. Event loop starts (tui_wait_until_exit)
```

### Input Flow

```
1. Terminal sends bytes (e.g., "\x1b[A" for up arrow)
   │
2. Event loop reads via poll()
   │
3. tui_input_parse() converts to tui_key_event
   │
4. Built-in handlers (Ctrl+C, Tab navigation)
   │
5. PHP input handler called with TuiKey object
   │
6. render_pending flag set → re-render on next loop
```

## Memory Management

### Reference Counting

PHP callbacks are properly reference-counted:

```c
// When setting callback
Z_TRY_ADDREF(fci.function_name);

// When replacing or destroying
zval_ptr_dtor(&fci.function_name);
```

### Resource Cleanup

TuiInstance has a custom destructor:

```c
static void tui_instance_free_object(zend_object *obj) {
    tui_instance_object *intern = tui_instance_from_obj(obj);
    if (intern->app) {
        tui_app_stop(intern->app);      // Restore terminal
        tui_app_destroy(intern->app);    // Free all resources
    }
    zend_object_std_dtor(&intern->std);
}
```

### Node Tree Cleanup

Recursive destruction:

```c
void tui_node_destroy(tui_node *node) {
    if (!node) return;
    for (int i = 0; i < node->child_count; i++) {
        tui_node_destroy(node->children[i]);
    }
    YGNodeFree(node->yoga_node);
    free(node->text);
    free(node->key);
    free(node->children);
    free(node);
}
```

## Build System

### config.m4

```m4
PHP_ARG_ENABLE(tui, whether to enable tui support)

if test "$PHP_TUI" != "no"; then
    PHP_REQUIRE_CXX()
    PHP_ADD_LIBRARY(stdc++, 1, TUI_SHARED_LIBADD)

    PHP_NEW_EXTENSION(tui, [
        tui.c
        src/app/app.c
        src/event/loop.c
        src/event/input.c
        ...
        src/yoga/*.cpp
    ], $ext_shared,, [-I$ext_srcdir/src -I$ext_srcdir/src/yoga])
fi
```

### Compilation

```bash
phpize
./configure --enable-tui
make
make test
```

## Testing

Tests use PHP's .phpt format:

```phpt
--TEST--
tui_get_terminal_size() returns array
--FILE--
<?php
$size = tui_get_terminal_size();
var_dump(is_array($size));
var_dump(count($size) === 2);
?>
--EXPECT--
bool(true)
bool(true)
```

## Performance Considerations

1. **Double Buffering**: Only dirty cells are redrawn
2. **Layout Caching**: Yoga caches layout calculations
3. **Minimal Redraws**: Render throttling (16ms = 60fps max)
4. **Efficient Input**: Poll-based, non-blocking I/O

## Thread Safety

ext-tui is not thread-safe:
- Terminal state is global (only one raw mode at a time)
- Single event loop per process
- Use separate processes for concurrent TUIs
