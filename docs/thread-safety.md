# Thread Safety Guide

This document describes the thread safety characteristics of ext-tui functions and data structures.

## Overview

**ext-tui is designed for single-threaded use.** Terminal I/O is inherently process-global, and the extension does not provide internal synchronization.

### Thread Safety Levels

| Level | Description |
|-------|-------------|
| **Thread-Safe** | Can be called from any thread without synchronization |
| **Reentrant** | Safe to call from signal handlers; no global state |
| **Main-Thread-Only** | Must be called from the main thread (or thread owning the terminal) |
| **Not Thread-Safe** | Requires external synchronization for concurrent access |

---

## Function Classification

### Pure Functions (Thread-Safe, Reentrant)

These functions have no side effects and operate only on their input parameters:

```c
// Text measurement - no global state
int tui_string_width(const char *str);
int tui_string_width_n(const char *str, int len);
int tui_char_width(uint32_t codepoint);

// UTF-8 encoding/decoding - pure functions
int tui_utf8_decode(const char *str, uint32_t *codepoint);
int tui_utf8_decode_n(const char *str, int len, uint32_t *codepoint);
int tui_utf8_encode(uint32_t codepoint, char *buf);

// Color utilities - pure functions
float tui_ease(float t, int easing);
float tui_lerp(float a, float b, float t);
void tui_lerp_color(tui_color *a, tui_color *b, float t, tui_color *out);
void tui_gradient(tui_color *colors, int count, tui_color *out, int steps);
void tui_color_to_hex(tui_color color, char *buf, size_t size);
int tui_color_from_hex(const char *hex, tui_color *out);

// ANSI escape generation - returns static or generates to buffer
const char* tui_ansi_reset(void);
const char* tui_ansi_bold(void);
const char* tui_ansi_dim(void);
// ... other style functions
```

### Object-Local Functions (Thread-Safe with Separate Instances)

These functions operate on a specific object instance. They are thread-safe when each thread uses its own instance:

```c
// Buffer operations - safe with separate buffers per thread
tui_buffer* tui_buffer_create(int width, int height);
void tui_buffer_destroy(tui_buffer *buf);
void tui_buffer_clear(tui_buffer *buf);
void tui_buffer_set_cell(tui_buffer *buf, int x, int y, ...);
void tui_draw_line(tui_buffer *buf, ...);
void tui_draw_rect(tui_buffer *buf, ...);
// ... other drawing functions

// Canvas operations - safe with separate canvases per thread
tui_canvas* tui_canvas_create(int width, int height, int mode);
void tui_canvas_free(tui_canvas *canvas);
void tui_canvas_set(tui_canvas *canvas, int x, int y);
void tui_canvas_line(tui_canvas *canvas, ...);
// ... other canvas functions

// Table operations - safe with separate tables per thread
tui_table* tui_table_create(int columns);
void tui_table_free(tui_table *table);
void tui_table_add_row(tui_table *table, ...);

// Sprite operations - safe with separate sprites per thread
tui_sprite* tui_sprite_create(...);
void tui_sprite_free(tui_sprite *sprite);
void tui_sprite_update(tui_sprite *sprite);

// History operations - safe with separate histories per thread
tui_input_history* tui_history_create(int max_entries);
void tui_history_destroy(tui_input_history *history);
void tui_history_add(tui_input_history *history, const char *entry);
const char* tui_history_prev(tui_input_history *history);
const char* tui_history_next(tui_input_history *history);

// Node operations - safe with separate node trees per thread
tui_node* tui_node_create(tui_node_type type);
void tui_node_destroy(tui_node *node);
void tui_node_add_child(tui_node *parent, tui_node *child);
```

### Main-Thread-Only Functions

These functions interact with the terminal and must be called from the main thread:

```c
// Terminal control - modifies global terminal state
int tui_terminal_init(void);
void tui_terminal_cleanup(void);
int tui_terminal_enable_raw_mode(void);
int tui_terminal_disable_raw_mode(void);
int tui_terminal_get_size(int *width, int *height);
void tui_terminal_enable_mouse(int mode);
void tui_terminal_disable_mouse(void);

// Output to terminal - writes to stdout
void tui_output_render(tui_buffer *buf);
void tui_output_cursor_show(void);
void tui_output_cursor_hide(void);
void tui_output_clear_screen(void);
void tui_output_move_cursor(int x, int y);

// Clipboard (writes escape sequences to stdout)
void tui_clipboard_copy(const char *text, int target);
void tui_clipboard_request(int target);
void tui_clipboard_clear(int target);

// App lifecycle - manages event loop and terminal state
tui_app* tui_app_create(void);
void tui_app_destroy(tui_app *app);
int tui_app_start(tui_app *app);
void tui_app_stop(tui_app *app);
void tui_app_wait_until_exit(tui_app *app);
void tui_app_render(tui_app *app);
```

### Signal Handlers (Async-Signal-Safe)

The SIGWINCH handler uses only async-signal-safe operations:

```c
static volatile sig_atomic_t resize_pending = 0;

static void sigwinch_handler(int sig) {
    resize_pending = 1;  // Atomic write only - async-signal-safe
}
```

The actual resize handling happens in the main event loop, not in the signal handler.

---

## Data Structure Thread Safety

### Global State

| State | Thread Safety | Notes |
|-------|---------------|-------|
| Terminal settings (termios) | Main-thread-only | Process-global, modified by `enable_raw_mode` |
| Signal handlers | Main-thread-only | Process-global |
| `resize_pending` flag | Async-signal-safe | `sig_atomic_t`, set by signal handler |
| Module globals (`TUI_G()`) | Thread-local in ZTS | Separate per-thread in ZTS builds |
| Yoga config | Thread-local in ZTS | Created per-thread |

### Object Instances

| Object | Thread Safety | Notes |
|--------|---------------|-------|
| `tui_app` | Not thread-safe | Single owner, interacts with terminal |
| `tui_buffer` | Object-local | Safe with separate instances |
| `tui_canvas` | Object-local | Safe with separate instances |
| `tui_node` | Object-local | Safe with separate node trees |
| `tui_table` | Object-local | Safe with separate instances |
| `tui_sprite` | Object-local | Safe with separate instances |
| `tui_input_history` | Object-local | Safe with separate instances |

---

## PHP Usage Patterns

### Single-Threaded (Recommended)

```php
<?php
// Standard single-threaded usage - always safe
$instance = tui_render($component);
tui_wait_until_exit($instance);
```

### With pthreads/parallel (Not Recommended)

If using PHP threading extensions, ensure:

1. Only one thread interacts with `TuiInstance`
2. Use message passing for cross-thread communication
3. Do not share TUI objects between threads

```php
<?php
// DON'T DO THIS - sharing TuiInstance between threads is unsafe
$instance = tui_render($component);
$thread = new Thread(function() use ($instance) {
    $instance->rerender();  // UNSAFE - terminal access from wrong thread
});

// DO THIS INSTEAD - use message passing
$channel = new Channel();
$thread = new Thread(function() use ($channel) {
    // Worker does computation, sends result via channel
    $channel->send($result);
});

// Main thread handles all TUI operations
while ($msg = $channel->receive()) {
    updateState($msg);
    $instance->rerender();
}
```

---

## ZTS (Zend Thread Safety) Considerations

In ZTS builds, module globals are thread-local:

```c
// Each thread gets its own globals
TUI_G(yoga_config)     // Thread-local Yoga config
TUI_G(metrics_enabled) // Thread-local metrics flag
```

However, terminal I/O is still process-global. Multiple threads should not attempt to control the terminal simultaneously.

---

## Best Practices

1. **Keep TUI operations on main thread**: All `tui_render`, `tui_rerender`, input handling, etc.

2. **Use pure functions freely**: Text measurement, color utilities, and easing functions are safe anywhere.

3. **Separate instances for concurrent work**: If processing data in parallel, create separate buffers/canvases per thread.

4. **Avoid signal handler complexity**: The SIGWINCH handler only sets a flag; actual resize handling is in the event loop.

5. **Don't mix terminal control**: If multiple components need terminal access, coordinate through the main TuiInstance.

---

## See Also

- [Security Considerations](security.md) - Signal handling, resource limits
- [Architecture](architecture.md) - Internal structure and data flow
