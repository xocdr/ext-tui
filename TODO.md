# ext-tui: Code Review & Gap Analysis Report

## Part 1: Code Review Summary

The code review identified issues across 4 severity levels:

### Critical Issues (4) - ✅ ALL FIXED

1. ~~**Missing NULL checks after `strdup()` calls**~~
   - ~~Files: `tui.c:686`, `src/node/node.c:56,135`, `src/drawing/sprite.c`, `src/drawing/table.c`, `src/text/wrap.c`~~
   - ~~Impact: Potential NULL pointer dereference, crashes~~
   - **FIXED:** Added NULL checks with proper cleanup in all locations

2. ~~**Unsafe `tui_utf8_decode()` without bounds checking**~~
   - ~~File: `src/text/measure.c:149-197`~~
   - ~~Impact: Buffer over-read if string has incomplete UTF-8 sequences~~
   - **FIXED:** Added continuation byte validation (`& 0xC0 == 0x80`)

3. ~~**Incomplete UTF-8 validation**~~
   - ~~Continuation bytes not validated for `10xxxxxx` pattern~~
   - ~~Impact: Acceptance of invalid UTF-8 sequences~~
   - **FIXED:** All continuation bytes now validated in `tui_utf8_decode()`

4. ~~**Potential double-free in reconciler**~~
   - ~~File: `src/node/reconciler.c:377-391` when `strdup()` fails~~
   - ~~Impact: Silent data corruption, potential use-after-free~~
   - **FIXED:** `diff_result_add()` now returns error code, overflow check added

### High Severity Issues (4) - ✅ ALL FIXED

1. ~~**Integer overflow in timer ID**~~
   - ~~File: `src/event/loop.c:99`~~
   - ~~Impact: Wraps after 2^31 timers, causing ID collisions~~
   - **FIXED:** Timer ID wraps to 1 when reaching INT_MAX

2. ~~**Allocation failure not propagated**~~
   - ~~File: `src/node/reconciler.c:107-109`~~
   - ~~Impact: Silent loss of diff operations, incomplete rendering~~
   - **FIXED:** `diff_result_add()` returns -1 on failure

3. ~~**Race condition in atomic flag**~~
   - ~~File: `src/event/loop.c:153-154`~~
   - ~~Impact: Missed window resize events under rapid resize~~
   - **FIXED:** Improved atomic flag handling with proper clear

4. ~~**Canvas size overflow check insufficient**~~
   - ~~File: `src/drawing/canvas.c:81-89`~~
   - ~~Impact: Incorrect allocation size for pixel data~~
   - **FIXED:** Improved overflow checks using size_t

### Medium Severity Issues (7) - ✅ ALL FIXED

- ~~PHP callback reference counting issues (`src/app/app.c:61-100`, `tui.c:1000+`)~~ **VERIFIED:** Already correctly implemented with proper `zval_ptr_dtor`, `ZVAL_UNDEF`, and `OBJ_RELEASE` calls
- ~~Buffer overflow risks in output/ANSI rendering (fixed 65536/128 byte buffers)~~ **FIXED:** Buffer now flushes when full instead of silently dropping data (`src/render/output.c:210-266`), increased ANSI buffer to 256 bytes
- ~~Unsafe `sscanf()` for hex colors (`tui.c:229`)~~ **FIXED:** Added strict hex validation
- ~~Missing state cleanup on exceptions (useState implementation)~~ **VERIFIED:** Already correctly handled via `tui_app_destroy()` → `tui_app_cleanup_states()` on all error paths
- ~~No circular reference detection in focus system (`src/app/app.c:281-299`)~~ **FIXED:** Added MAX_TREE_DEPTH limit
- ~~Missing NULL checks after `malloc()` (`src/text/wrap.c:42-43`)~~ **VERIFIED:** Already had NULL check
- ~~Inconsistent error return values~~ **VERIFIED:** Codebase consistently uses `-1` for error, `0` for success on int-returning functions

### Well-Implemented Areas

- ✅ Overflow prevention in buffer management
- ✅ Solid UTF-8 handling (with caveats noted above)
- ✅ Clean reconciliation algorithm with key-based diffing
- ✅ Proper terminal signal handling
- ✅ Good reference counting in PHP objects
- ✅ Style property diffing (only emit necessary ANSI codes)
- ✅ Input parsing safety with comprehensive bounds checking

---

## Part 2: Yoga Layout Engine Analysis

### Currently Used (29 functions)

- flexDirection, justifyContent, alignItems, alignSelf
- flexGrow, flexShrink, flexBasis (all 3 variants)
- width, height, min/max constraints (including % and auto)
- margin, padding, border setters
- gap setter
- overflow, display, positionType

### ~~Not Currently Used (Significant Features)~~ - ✅ ALL NOW IMPLEMENTED

| Feature | Use Case | Priority | Status |
|---------|----------|----------|--------|
| ~~**Custom Measure Functions**~~ | Text measurement, dynamic sizing | HIGH | **ALREADY IMPLEMENTED** (text nodes use `text_measure_func`) |
| ~~**Dirtied Callbacks**~~ | Incremental updates, avoid full recalc | HIGH | **FIXED:** Added `node_dirtied_func` callback |
| ~~**Aspect Ratio**~~ | Maintain width/height proportions | MEDIUM | **FIXED:** Added `aspectRatio` property to Box |
| ~~**Baseline Functions**~~ | Text baseline alignment | MEDIUM | **FIXED:** Added `text_baseline_func`, `alignItems: baseline` |
| ~~**Node Context**~~ | Store metadata on layout nodes | LOW | **ALREADY IMPLEMENTED** (`YGNodeSetContext` used) |
| ~~**RTL Support**~~ | Right-to-left layouts | LOW | **FIXED:** Added `direction` property (ltr/rtl) |
| ~~**Configuration Sharing**~~ | Global config, errata modes | LOW | **FIXED:** Added shared `YGConfig` via `tui_get_yoga_config()` |

---

## Part 3: React Reconciler Comparison

### What ext-tui Does Well

- ✅ Key-based reconciliation
- ✅ Map-based child matching (O(n) lookup)
- ✅ Multi-pass apply (DELETE → UPDATE → REORDER → CREATE)
- ✅ Preserves Yoga node identity for layout caching

### Missing Optimizations from React

| Feature | React | ext-tui | Impact |
|---------|-------|---------|--------|
| **Smart Reorder Detection** | `placeChild()` with `lastPlacedIndex` | All position changes marked | React generates fewer DOM ops |
| **Fast Path Separation** | Linear scan → Map fallback | Always keyed | React faster for simple cases |
| **Effect Flags** | Bit flags for composable effects | Array of operations | React more memory efficient |
| **Lazy Deletion** | Collected on parent | Immediate in diff | React batches deletions |

### Recommended Optimization

Implement `lastPlacedIndex` algorithm to reduce unnecessary reorder operations:

```c
static int last_placed_index = 0;

// Instead of marking all position changes:
if (matched_old_idx != new_idx) {
    diff_result_add(result, TUI_DIFF_REORDER, ...);
}

// Do smart detection - only mark nodes that truly need movement:
if (matched_old_idx < last_placed_index) {
    diff_result_add(result, TUI_DIFF_REORDER, ...);
    // Don't update last_placed_index
} else {
    last_placed_index = matched_old_idx;
    // Item is already in correct relative position, no move needed
}
```

---

## Part 4: Ink Feature Gap Analysis

### Critical Missing Features in ext-tui

| Feature | Description | Difficulty |
|---------|-------------|------------|
| **Tab-based focus navigation** | `focusNext()`/`focusPrevious()` with Tab key | LOW |
| **Focus-by-ID** | `focus(id)` programmatic control | LOW |
| **useFocusManager hook** | Global focus control API | MEDIUM |
| **Error boundary component** | Crash display with stack trace | MEDIUM |
| **Text wrap modes** | `truncate-start`, `truncate-middle` | LOW |
| **Named color support** | CSS color names, not just RGB | LOW |
| **Console interception** | Patch echo/print during render | MEDIUM |
| **measureElement API** | Get runtime dimensions of components | MEDIUM |
| **Line-by-line transform** | Transform callback per line | LOW |
| **React DevTools equivalent** | Debug component tree | HIGH |

### Advantages ext-tui Has Over Ink

- ✅ Built-in drawing primitives (table, progress, sprite, animation)
- ✅ Canvas-based rendering architecture
- ✅ Direct C API (lower level control)
- ✅ No JS runtime dependency

---

## Part 5: CLI Utility Features Worth Implementing

### High Priority

#### 1. ANSI Code Handling (from `ansi-regex`, `slice-ansi`)

```c
// Strip all ANSI escape codes from string
char* tui_strip_ansi(const char *str);

// Calculate display width excluding ANSI codes
int tui_string_width_ignore_ansi(const char *str);

// Extract substring while preserving ANSI codes
char* tui_slice_ansi(const char *str, int start, int end);
```

#### 2. Advanced Truncation (from `cli-truncate`)

```c
typedef enum {
    TUI_TRUNCATE_END,      // "Hello Wo..." (current)
    TUI_TRUNCATE_START,    // "...lo World"
    TUI_TRUNCATE_MIDDLE    // "Hello...orld"
} tui_truncate_position;

char* tui_truncate_ex(const char *str, int width,
                      tui_truncate_position pos,
                      const char *ellipsis,
                      int preserve_ansi);
```

#### 3. Box Drawing (from `cli-boxes`)

Pre-defined character sets for 8 styles:

```c
typedef struct {
    const char *topLeft, *top, *topRight;
    const char *left, *right;
    const char *bottomLeft, *bottom, *bottomRight;
} tui_box_chars;

// Styles: single, double, round, bold, singleDouble, doubleSingle, classic, arrow
const tui_box_chars* tui_get_box_chars(tui_border_style style);
```

#### 4. Emoji Support (from `emoji-regex`)

Fix `tui_string_width()` for complex emoji sequences:
- Handle ZWJ (Zero-Width Joiner) sequences
- Handle skin tone modifiers
- Emoji typically display as width 2

### Medium Priority

#### 5. Color Conversion (from `ansi-styles`)

```c
// Convert RGB to 256-color palette index
int tui_rgb_to_ansi256(int r, int g, int b);

// Parse hex color string
tui_color tui_hex_to_rgb(const char *hex);

// Named color constants
extern const tui_color TUI_COLOR_RED;
extern const tui_color TUI_COLOR_GREEN;
// ... etc
```

#### 6. Additional Cursor/Screen Operations (from `ansi-escapes`)

```c
void tui_erase_start_line(tui_output *out);
void tui_erase_end_line(tui_output *out);
void tui_scroll_up(tui_output *out, int lines);
void tui_scroll_down(tui_output *out, int lines);
void tui_cursor_next_line(tui_output *out);
void tui_cursor_prev_line(tui_output *out);
```

---

## Summary: Prioritized Recommendations

### Immediate Fixes (Critical Bugs) - ✅ ALL COMPLETED

- [x] Add NULL checks after all `strdup()` calls
  - Fixed in `tui.c:686-690` (key property)
  - Fixed in `tui.c:696-700` (id property via `tui_node_set_id()`)
  - Fixed in `src/drawing/sprite.c:28-54` (frame lines)
  - Fixed in `src/drawing/sprite.c:125-185` (animation name and frame lines)
  - Fixed in `src/node/node.c:130-142` (`tui_node_set_id()` now returns error code)
- [x] Fix UTF-8 continuation byte validation in `tui_utf8_decode()`
  - Fixed in `src/text/measure.c:164-202` - all continuation bytes now validated for `10xxxxxx` pattern
- [x] Add error return propagation from `diff_result_add()`
  - Fixed in `src/node/reconciler.c:99-126` - now returns int (0 success, -1 failure)
  - Added overflow check before capacity doubling
- [x] Fix atomic flag race condition in resize handling
  - Fixed in `src/event/loop.c:154-172` - improved atomic flag handling with proper clear on non-set case

### High Severity Fixes - ✅ ALL COMPLETED

- [x] Fix integer overflow in timer ID (`src/event/loop.c:99-102`)
  - Timer ID now wraps to 1 when reaching INT_MAX
- [x] Fix canvas size overflow check (`src/drawing/canvas.c:86-93`)
  - Improved overflow checks using size_t for pixel count calculation
  - Added check for byte_count overflow
- [x] Add depth limit to focus tree traversal (`src/app/app.c:281-320`)
  - Added MAX_TREE_DEPTH (256) limit to prevent stack overflow
  - Added overflow check before capacity doubling

### Medium Severity Fixes - ✅ COMPLETED

- [x] Improved hex color validation (`tui.c:228-247`)
  - Now validates all characters are valid hex digits before sscanf

### Short-Term Enhancements

- [ ] Implement `lastPlacedIndex` optimization in reconciler
- [ ] Add ANSI-aware string functions (`strip`, `width_ignore_ansi`, `slice`)
- [ ] Add truncation position modes (start/middle/end)
- [ ] Implement `useFocusManager` with Tab navigation
- [ ] Add box drawing character utilities

### Medium-Term Features

- [ ] Yoga custom measure functions for text
- [ ] Yoga dirtied callbacks for incremental updates
- [ ] Error boundary component
- [ ] Focus-by-ID support
- [ ] Emoji width handling
- [ ] Named color constants

### Long-Term Improvements

- [ ] React DevTools equivalent for PHP
- [ ] Console interception during render
- [ ] RTL layout support
- [ ] Aspect ratio support in Yoga
- [ ] Component testing framework

---

## References

- React Reconciler: Algorithm analysis from `react-reconciler` v0.29.2
- Yoga Layout: Feature analysis from `yoga-layout` v3.2.1
- Ink: Feature comparison from `ink` v5.2.1
- CLI Utilities: `ansi-escapes`, `ansi-regex`, `ansi-styles`, `cli-boxes`, `cli-cursor`, `cli-truncate`, `emoji-regex`
