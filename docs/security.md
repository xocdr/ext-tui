# Security Considerations

This document describes security considerations for ext-tui, including input validation, buffer limits, thread safety, and signal handling.

## Input Validation

### API Boundaries

All PHP-facing functions validate their inputs before processing:

1. **String Inputs**: String lengths are checked and bounded. Functions like `tui_string_width_n()` take explicit length parameters rather than relying on null termination.

2. **Numeric Inputs**: Dimensions and sizes are validated against configurable limits:
   - Canvas dimensions: 1-1000 (configurable)
   - Buffer dimensions: Limited by `tui.max_buffer_width` and `tui.max_buffer_height` INI settings
   - Tree depth: Limited by `tui.max_tree_depth` (default: 100)

3. **Callbacks**: PHP callbacks are validated using Zend's `zend_call_function()` API which handles type checking.

### UTF-8 Handling

The extension handles UTF-8 input safely:

- **Bounded decoding**: `tui_utf8_decode_n()` takes explicit length and won't read past buffer boundaries
- **Invalid sequences**: Malformed UTF-8 (truncated sequences, invalid continuation bytes, overlong encodings, surrogates) is handled gracefully - invalid bytes are treated as width-1 characters
- **No assumptions**: The code doesn't assume input is valid UTF-8; it validates during processing

### ANSI Escape Sequences

When parsing terminal escape sequences:
- Base64 decoding (for clipboard) has explicit output buffer bounds checking
- Escape sequence parsing validates sequence structure before processing
- Mouse coordinates are validated against maximum terminal dimensions

## Buffer Size Limits

### Configurable via php.ini

```ini
tui.max_buffer_width = 500     ; Maximum render buffer width
tui.max_buffer_height = 500    ; Maximum render buffer height
tui.max_tree_depth = 100       ; Maximum node tree depth
tui.max_states = 64            ; Maximum useState hooks per component
tui.max_timers = 32            ; Maximum active timers
```

### Overflow Protection

All buffer allocations include overflow checks:

```c
// Example from buffer.c
size_t cell_count = (size_t)width * (size_t)height;
if (cell_count > SIZE_MAX / sizeof(tui_cell)) {
    return NULL;  // Would overflow
}
```

Integer overflow is checked before:
- Array capacity growth
- Buffer allocations
- String concatenations

### Recursion Limits

Tree traversal operations enforce depth limits to prevent stack overflow:

```c
#define MAX_TREE_DEPTH 256

static void traverse(tui_node *node, int depth) {
    if (!node || depth >= MAX_TREE_DEPTH) return;
    // ... process node
    for (int i = 0; i < node->child_count; i++) {
        traverse(node->children[i], depth + 1);
    }
}
```

## Thread Safety

### Single-Threaded Terminal Access

**Terminal operations are NOT thread-safe.** The TUI application model assumes single-threaded terminal access:

- `termios` state is process-global
- Signal handlers (SIGWINCH) affect the entire process
- Raw mode settings apply to the process's controlling terminal

In ZTS (Zend Thread Safety) builds:
- Module globals are thread-local via `TUI_G()` macro
- Each thread can have its own TUI state
- But actual terminal I/O should only occur from one thread

### Recommendations

1. Only interact with the terminal from the main thread
2. Don't share `TuiInstance` objects between threads
3. Use locks if you need to coordinate terminal access across threads

## Signal Handling

### SIGWINCH (Terminal Resize)

The extension installs a handler for `SIGWINCH` to detect terminal resize:

```c
static volatile sig_atomic_t resize_pending = 0;

static void sigwinch_handler(int sig) {
    resize_pending = 1;  // Atomic, async-signal-safe
}
```

Key safety measures:
- Uses `sig_atomic_t` for signal-handler/main-thread communication
- Handler only sets a flag; actual resize handling happens in main loop
- Terminal size queries happen outside the signal handler

### Terminal State Restoration

The extension saves and restores terminal state:
- Original `termios` settings are preserved
- Raw mode is disabled on cleanup (MSHUTDOWN)
- Alternate screen buffer is exited

## Memory Management

### Allocation Patterns

- Extension uses `emalloc`/`efree` for request-scoped memory (freed automatically on request end)
- Uses `malloc`/`free` for persistent allocations (pools, Yoga config)
- All allocations are checked for NULL before use

### Object Pools

The extension uses object pools to reduce allocation churn:
- Children arrays are pooled and reused
- Pools have fixed maximum sizes to bound memory usage
- Pool statistics are tracked via metrics

### Reference Counting

PHP objects and callbacks are properly reference-counted:
- `Z_TRY_ADDREF()` when storing references
- `zval_ptr_dtor()` when releasing
- Object references cleaned up in destructors

## Error Handling

### Return Value Conventions

```c
/* Error Return Conventions (from php_tui.h):
 * - Functions returning int: return -1 on error, 0 or positive on success
 * - Functions returning pointers: return NULL on error
 * - Append/add functions: return -1 on error, 0 on success
 * - Create functions: return NULL on error, valid pointer on success
 * - Width/size functions: return 0 for empty input, -1 never
 */
```

### Exception Handling

PHP exceptions are thrown for:
- Invalid component return types
- Invalid canvas/sprite dimensions
- Resource allocation failures

## Testing for Security

### Available Tools

1. **Fuzz Testing**: `./scripts/fuzz-test.sh` - Tests input parsing with random data
2. **AddressSanitizer**: `./scripts/test-asan.sh` - Detects memory errors
3. **Valgrind**: `./scripts/test-valgrind.sh` - Memory leak detection
4. **Static Analysis**: `./scripts/static-analysis.sh` - Code analysis with cppcheck

### Compiler Flags

The build enables strict warnings:
- `-Wall -Wextra` - Comprehensive warnings
- `-Wformat=2` - Format string checking
- `-Wconversion` - Implicit conversion warnings
- `-Wshadow` - Variable shadowing detection

## Known Limitations

1. **Platform**: macOS and Linux only (uses POSIX APIs)
2. **Terminal**: Single terminal per process
3. **Input**: Keyboard only (no mouse support yet)
4. **Encoding**: UTF-8 assumed for input/output

## Reporting Security Issues

If you discover a security vulnerability, please report it privately by emailing the maintainers rather than opening a public issue.
