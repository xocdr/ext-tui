# Security Audit Checklist

This document provides a checklist for auditing ext-tui for security vulnerabilities.

## 1. Input Validation

### 1.1 External Input Sources

| Source | Handler | Validation | Status |
|--------|---------|------------|--------|
| Terminal input (stdin) | `src/event/input.c` | Length-bounded parsing | ✅ |
| PHP string parameters | `tui.c` | Zend API validates types | ✅ |
| Callback return values | `tui.c` | Type checked before use | ✅ |
| Terminal escape sequences | `src/event/input.c` | Bounded sequence parsing | ✅ |
| OSC clipboard data | `src/terminal/ansi.c` | Base64 decode with bounds | ✅ |

### 1.2 UTF-8 Handling

| Check | Location | Status |
|-------|----------|--------|
| Overlong encodings rejected | `src/text/measure.c:165,185,212` | ✅ |
| Surrogate pairs rejected | `src/text/measure.c:189-193` | ✅ |
| Invalid continuation bytes handled | `src/text/measure.c:159,177,203` | ✅ |
| Truncated sequences handled | `src/text/measure.c` | ✅ |
| Out-of-range codepoints rejected | `src/text/measure.c:212` | ✅ |

### 1.3 Numeric Input

| Parameter | Validation | Location |
|-----------|------------|----------|
| Width/height dimensions | Range checked against INI limits | `php_tui.h` |
| Array indices | Bounds checked | Throughout |
| Tree depth | MAX_TREE_DEPTH constant | `src/node/node.c` |
| Timer counts | max_timers INI setting | `php_tui.h` |

## 2. Buffer Operations

### 2.1 Overflow Checks

| Operation | Check | Location |
|-----------|-------|----------|
| Buffer allocation | size_t overflow check | `src/render/buffer.c` |
| Array growth | Capacity overflow check | `src/node/node.c` |
| String concatenation | Length validation | Throughout |
| Canvas creation | Dimension limits | `tui.c` |

### 2.2 Bounds Checking

| Array Type | Checked | Location |
|------------|---------|----------|
| Character buffer | ✅ | `src/render/buffer.c` |
| Children array | ✅ | `src/node/node.c` |
| History entries | ✅ | `src/event/input.c` |
| Output strings | ✅ | `src/text/measure.c`, `src/text/wrap.c` |

### 2.3 Integer Overflow Prevention

Pattern used throughout the codebase:

```c
// Check before multiplication
if (width > 0 && height > SIZE_MAX / (size_t)width) {
    return NULL;  // Would overflow
}
size_t size = (size_t)width * (size_t)height;
```

## 3. Memory Management

### 3.1 Allocation Patterns

| Pattern | Usage | Safety |
|---------|-------|--------|
| `emalloc`/`efree` | Request-scoped memory | Auto-freed on request end |
| `malloc`/`free` | Persistent allocations | Manual management required |
| Object pools | Children arrays | Fixed max size |

### 3.2 Reference Counting

| Object Type | Add Ref | Release | Cleanup |
|-------------|---------|---------|---------|
| PHP callbacks | `Z_TRY_ADDREF()` | `zval_ptr_dtor()` | Destructor |
| TuiInstance | `zend_object` | `zend_objects_destroy_object` | MSHUTDOWN |

### 3.3 Resource Leaks

| Resource | Cleanup Location |
|----------|-----------------|
| Terminal state | `tui_cleanup()` in MSHUTDOWN |
| Yoga nodes | `YGNodeFreeRecursive()` |
| History objects | `tui_history_dtor()` |
| Render buffers | Request end (emalloc) |

## 4. Signal Handling

### 4.1 SIGWINCH Handler

```c
static volatile sig_atomic_t resize_pending = 0;

static void sigwinch_handler(int sig) {
    resize_pending = 1;  // Atomic write only
}
```

Safety measures:
- Uses `sig_atomic_t` for signal-safe communication
- Handler performs no allocations
- Handler performs no system calls
- Actual resize handling in main loop

### 4.2 Terminal State Restoration

| State | Saved | Restored |
|-------|-------|----------|
| termios settings | MINIT | MSHUTDOWN |
| Alternate screen | On render | On unmount |
| Cursor visibility | On render | On unmount |
| Mouse mode | On enable | On disable/cleanup |

## 5. Thread Safety

### 5.1 Not Thread-Safe

Terminal operations are inherently single-threaded:

- `termios` is process-global
- Signal handlers affect entire process
- Only one terminal per process

### 5.2 ZTS Considerations

In Zend Thread Safety builds:
- Module globals via `TUI_G()` macro
- Each thread has isolated state
- Terminal I/O should be main thread only

## 6. Denial of Service Protection

### 6.1 Resource Limits

| Resource | Limit | INI Setting |
|----------|-------|-------------|
| Buffer width | 500 default | `tui.max_buffer_width` |
| Buffer height | 500 default | `tui.max_buffer_height` |
| Tree depth | 100 default | `tui.max_tree_depth` |
| Active timers | 32 default | `tui.max_timers` |
| useState hooks | 64 default | `tui.max_states` |

### 6.2 Recursion Limits

All tree traversals enforce depth limits to prevent stack overflow.

### 6.3 Escape Sequence Limits

```c
#define MAX_ANSI_SEQUENCE_LENGTH 64
```

Prevents parsing pathologically long escape sequences.

## 7. Information Disclosure

### 7.1 Error Messages

- Error messages don't include sensitive data
- Stack traces only in debug mode
- Terminal size is exposed (non-sensitive)

### 7.2 Memory Contents

- Buffers zeroed on allocation where appropriate
- No uninitialized reads (ASAN verified)

## 8. Audit Tools

### 8.1 Compile-Time

```bash
# Strict warnings
./configure CFLAGS="-Wall -Wextra -Wconversion -Wshadow -Wformat=2"

# Static analysis
./scripts/static-analysis.sh
```

### 8.2 Runtime

```bash
# AddressSanitizer
./scripts/test-asan.sh

# Valgrind
./scripts/test-valgrind.sh

# Fuzz testing
./scripts/fuzz-test.sh 10000
cd fuzz && ./fuzz_utf8_standalone corpus/
```

## 9. Audit Findings

### 9.1 Issues Found and Fixed

| Issue | Severity | Fixed In |
|-------|----------|----------|
| N/A | - | - |

### 9.2 Known Limitations

1. **Terminal-specific behavior**: Width calculations may vary between terminals
2. **Unicode version**: Based on Unicode 15.1 data
3. **Platform**: macOS/Linux only (uses POSIX APIs)

## 10. Recommendations

1. **Regular fuzzing**: Run fuzz tests before each release
2. **ASAN CI**: Include AddressSanitizer in CI pipeline
3. **Update Unicode data**: Keep East Asian Width data current
4. **Dependency audit**: Yoga library should be updated periodically

---

Last audit date: 2024-12-30
Auditor: Claude Code Assistant
