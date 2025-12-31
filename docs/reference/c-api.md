# C API Reference

This document describes the internal C API of ext-tui for extension developers and contributors.

## Overview

The C API is organized into several modules:

| Module | Header | Purpose |
|--------|--------|---------|
| App | `src/app/app.h` | Application lifecycle and state management |
| Node | `src/node/node.h` | Virtual DOM node creation and manipulation |
| Buffer | `src/render/buffer.h` | Character buffer for terminal rendering |
| Output | `src/render/output.h` | Terminal output and cursor control |
| Event | `src/event/loop.h` | Event loop for input handling |
| Pool | `src/pool/pool.h` | Object pools for performance |

---

## App Module (`src/app/app.h`)

Manages the TUI application lifecycle, state hooks, focus system, and timers.

### Types

```c
typedef struct {
    zval value;           /* Current state value */
    zval setter;          /* Setter closure */
    int index;            /* State slot index */
} tui_state_slot;

typedef struct tui_app tui_app;
```

### Lifecycle Functions

#### `tui_app_create`
```c
tui_app* tui_app_create(void);
```
Create a new app instance. Initializes output, buffer, and event loop.

**Returns:** New app instance, or NULL on allocation failure.

#### `tui_app_destroy`
```c
void tui_app_destroy(tui_app *app);
```
Destroy app and free all resources. NULL-safe.

### Configuration Functions

#### `tui_app_set_fullscreen`
```c
void tui_app_set_fullscreen(tui_app *app, int fullscreen);
```
Enable/disable alternate screen buffer mode.

#### `tui_app_set_exit_on_ctrl_c`
```c
void tui_app_set_exit_on_ctrl_c(tui_app *app, int enabled);
```
Enable/disable automatic exit on Ctrl+C.

### Handler Functions

#### `tui_app_set_component`
```c
void tui_app_set_component(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc);
```
Set the main component callback (called each rerender).

#### `tui_app_set_input_handler`
```c
void tui_app_set_input_handler(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc);
```
Set keyboard input handler.

#### `tui_app_set_focus_handler`
```c
void tui_app_set_focus_handler(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc);
```
Set focus change handler.

#### `tui_app_set_resize_handler`
```c
void tui_app_set_resize_handler(tui_app *app, zend_fcall_info *fci, zend_fcall_info_cache *fcc);
```
Set terminal resize handler.

### Execution Functions

#### `tui_app_run`
```c
int tui_app_run(tui_app *app);
```
Enter the event loop. Blocks until exit is requested.

**Returns:** Exit code (0 = success).

#### `tui_app_exit`
```c
void tui_app_exit(tui_app *app, int code);
```
Request graceful exit from event loop.

### State Functions

#### `tui_app_get_or_create_state_slot`
```c
int tui_app_get_or_create_state_slot(tui_app *app, zval *initial, int *is_new);
```
Get or create a state slot for useState hook. Array grows dynamically as needed.

**Parameters:**
- `app` - App instance
- `initial` - Initial value (used only when creating new slot)
- `is_new` - Output: set to 1 if new slot created, 0 if reused

**Returns:** State index (>= 0), or -1 on allocation failure.

#### `tui_app_reset_state_index`
```c
void tui_app_reset_state_index(tui_app *app);
```
Reset state index for new render cycle. Called before each component render.

#### `tui_app_cleanup_states`
```c
void tui_app_cleanup_states(tui_app *app);
```
Clean up all state slots and free PHP references. Called during app destruction.

### Timer Functions

#### `tui_app_add_timer`
```c
int tui_app_add_timer(tui_app *app, int interval_ms, zend_fcall_info *fci, zend_fcall_info_cache *fcc);
```
Add a timer callback.

**Returns:** Timer index, or -1 if limit reached.

#### `tui_app_remove_timer`
```c
int tui_app_remove_timer(tui_app *app, int timer_index);
```
Remove a timer by index.

**Returns:** 0 on success, -1 on error.

---

## Node Module (`src/node/node.h`)

Virtual DOM nodes representing the UI tree.

### Types

```c
typedef enum {
    TUI_NODE_BOX,
    TUI_NODE_TEXT,
    TUI_NODE_STATIC,
    TUI_NODE_NEWLINE
} tui_node_type;

typedef struct tui_node tui_node;
```

### Creation Functions

#### `tui_node_create_box`
```c
tui_node* tui_node_create_box(void);
```
Create a box (container) node.

**Returns:** New node, or NULL on allocation failure.

#### `tui_node_create_text`
```c
tui_node* tui_node_create_text(const char *text);
```
Create a text node with content.

**Returns:** New node, or NULL on allocation failure.

#### `tui_node_create_static`
```c
tui_node* tui_node_create_static(void);
```
Create a static output node (renders above dynamic content).

**Returns:** New node, or NULL on allocation failure.

#### `tui_node_create_newline`
```c
tui_node* tui_node_create_newline(int count);
```
Create a newline node (blank lines).

**Returns:** New node, or NULL on allocation failure.

### Destruction

#### `tui_node_destroy`
```c
void tui_node_destroy(tui_node *node);
```
Recursively destroy a node and all children. NULL-safe.

### Tree Manipulation

#### `tui_node_add_child`
```c
int tui_node_add_child(tui_node *parent, tui_node *child);
```
Add a child node to a parent.

**Returns:** 0 on success, -1 on error.

#### `tui_node_remove_child`
```c
int tui_node_remove_child(tui_node *parent, int index);
```
Remove child at index (does not destroy it).

**Returns:** 0 on success, -1 on error.

### Property Functions

#### `tui_node_set_id`
```c
int tui_node_set_id(tui_node *node, const char *id);
```
Set node ID (for focus-by-id and measureElement).

**Returns:** 0 on success, -1 on allocation failure.

#### `tui_node_set_hyperlink`
```c
int tui_node_set_hyperlink(tui_node *node, const char *url, const char *id);
```
Set OSC 8 hyperlink on node.

**Returns:** 0 on success, -1 on allocation failure.

---

## Buffer Module (`src/render/buffer.h`)

Character buffer for terminal rendering with double-buffering.

### Types

```c
typedef struct {
    char ch[4];           /* UTF-8 character */
    tui_style style;      /* Cell style */
} tui_cell;

typedef struct tui_buffer tui_buffer;
```

### Lifecycle

#### `tui_buffer_create`
```c
tui_buffer* tui_buffer_create(int width, int height);
```
Create a new character buffer.

**Returns:** New buffer, or NULL on allocation failure.

#### `tui_buffer_destroy`
```c
void tui_buffer_destroy(tui_buffer *buf);
```
Free buffer and all resources.

### Operations

#### `tui_buffer_clear`
```c
void tui_buffer_clear(tui_buffer *buf);
```
Clear buffer to spaces with default style.

#### `tui_buffer_resize`
```c
int tui_buffer_resize(tui_buffer *buf, int width, int height);
```
Resize buffer dimensions.

**Returns:** 0 on success, -1 on allocation failure.

#### `tui_buffer_set_cell`
```c
void tui_buffer_set_cell(tui_buffer *buf, int x, int y, const char *ch, tui_style *style);
```
Set a cell's character and style.

#### `tui_buffer_write_string`
```c
int tui_buffer_write_string(tui_buffer *buf, int x, int y, const char *str, tui_style *style);
```
Write a string starting at position. Returns characters written.

---

## Output Module (`src/render/output.h`)

Terminal output with ANSI escape code handling.

### Lifecycle

#### `tui_output_create`
```c
tui_output* tui_output_create(int width, int height);
```
Create output handler.

**Returns:** New output, or NULL on allocation failure.

#### `tui_output_destroy`
```c
void tui_output_destroy(tui_output *out);
```
Free output and restore terminal state.

### Terminal Control

#### `tui_output_enter_alternate`
```c
void tui_output_enter_alternate(tui_output *out);
```
Enter alternate screen buffer.

#### `tui_output_leave_alternate`
```c
void tui_output_leave_alternate(tui_output *out);
```
Leave alternate screen buffer.

#### `tui_output_hide_cursor`
```c
void tui_output_hide_cursor(tui_output *out);
```
Hide the cursor.

#### `tui_output_show_cursor`
```c
void tui_output_show_cursor(tui_output *out);
```
Show the cursor.

### Rendering

#### `tui_output_render`
```c
void tui_output_render(tui_output *out, tui_buffer *buf);
```
Render buffer to terminal (differential update).

#### `tui_output_flush`
```c
void tui_output_flush(tui_output *out);
```
Flush output buffer to terminal.

---

## Event Loop (`src/event/loop.h`)

Event loop for keyboard input and timers.

### Lifecycle

#### `tui_loop_create`
```c
tui_loop* tui_loop_create(void);
```
Create event loop instance.

**Returns:** New loop, or NULL on allocation failure.

#### `tui_loop_destroy`
```c
void tui_loop_destroy(tui_loop *loop);
```
Destroy event loop.

### Execution

#### `tui_loop_run`
```c
int tui_loop_run(tui_loop *loop);
```
Enter event loop. Blocks until stopped.

**Returns:** 0 on success.

#### `tui_loop_stop`
```c
void tui_loop_stop(tui_loop *loop);
```
Request event loop exit.

### Timers

#### `tui_loop_add_timer`
```c
int tui_loop_add_timer(tui_loop *loop, int interval_ms, tui_timer_callback callback, void *userdata);
```
Add a timer.

**Returns:** Timer ID, or -1 on error.

#### `tui_loop_remove_timer`
```c
int tui_loop_remove_timer(tui_loop *loop, int timer_id);
```
Remove a timer by ID.

**Returns:** 0 on success, -1 if not found.

---

## Pool Module (`src/pool/pool.h`)

Object pools for reducing allocation overhead.

### Initialization

#### `tui_pools_create`
```c
tui_pools* tui_pools_create(void);
```
Create the pool manager.

**Returns:** New pools, or NULL on allocation failure.

#### `tui_pools_destroy`
```c
void tui_pools_destroy(tui_pools *pools);
```
Destroy all pools and free memory.

### Diff Entry Pool

#### `tui_diff_pool_alloc`
```c
tui_diff_entry* tui_diff_pool_alloc(tui_pools *pools);
```
Allocate a diff entry from pool.

**Returns:** Diff entry, or NULL if pool exhausted.

#### `tui_diff_pool_free`
```c
void tui_diff_pool_free(tui_pools *pools, tui_diff_entry *entry);
```
Return diff entry to pool.

### Children Array Pool

#### `tui_children_pool_alloc`
```c
tui_node** tui_children_pool_alloc(tui_pools *pools, int requested, int *actual);
```
Allocate children array from pool.

**Returns:** Array pointer, or NULL if allocation failed.

#### `tui_children_pool_free`
```c
void tui_children_pool_free(tui_pools *pools, tui_node **children, int capacity);
```
Return children array to pool.

---

## Error Handling

All functions follow these conventions:

| Return Type | Success | Failure |
|-------------|---------|---------|
| `int` | 0 or positive | -1 |
| Pointer | Valid pointer | NULL |
| `void` | N/A | N/A |

Functions that can fail check their inputs and handle NULL gracefully.

---

## Memory Management

- **Lifecycle**: Create/Destroy pairs must be matched
- **PHP Memory**: Use `emalloc`/`efree` for PHP-request-scoped memory
- **C Memory**: Use `malloc`/`free` for extension-global memory
- **Strings**: Nodes use `strdup`/`free`; PHP strings use `zend_string_copy`

---

## Thread Safety

The C API is NOT thread-safe. All calls must be made from the main thread.

- Module globals are per-thread in ZTS builds
- Terminal operations (termios) are process-global
- Signal handlers only set volatile flags

See `docs/thread-safety.md` for detailed information.

---

## Limits

| Constant | Value | Purpose |
|----------|-------|---------|
| `INITIAL_STATE_CAPACITY` | 8 | Initial state slots (grows dynamically) |
| `INITIAL_TIMER_CAPACITY` | 4 | Initial timer slots (grows dynamically) |
| `TUI_MAX_KEY_LENGTH` | 256 | Maximum key string length |
| `TUI_MAX_ID_LENGTH` | 256 | Maximum id string length |
| `TUI_MAX_TEXT_LENGTH` | 1MB | Maximum text content length |

**Note**: State and timer arrays grow dynamically as needed, controlled by `tui.max_states` and `tui.max_timers` INI settings (defaults: 256 states, 64 timers).
