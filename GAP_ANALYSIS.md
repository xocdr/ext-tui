# ext-tui Gap Analysis: SPEC.md vs Implementation

This document identifies features specified in `exocoder/docs/tui/SPEC.md` that are **NOT** implemented in ext-tui.

## Summary

| Category | Specified | Implemented | Missing |
|----------|-----------|-------------|---------|
| Core Functions | 17 | 17 | 0 |
| Classes | 4 | 4 | 0 |
| Drawing Primitives | 14 | 0 | **14** |
| Sprite API | 8 | 0 | **8** |
| Canvas API | 10 | 0 | **10** |
| Animation Utils | 4 | 0 | **4** |
| Table Rendering | 2 | 0 | **2** |
| Progress Rendering | 3 | 0 | **3** |
| Text Utils | 4 | 3 | **1** |
| TuiBox Properties | 26 | 22 | **4** |
| **TOTAL** | 92 | 46 | **46** |

---

## Implemented Features

### Core Functions (All Implemented)
- `tui_get_terminal_size()` - Get terminal dimensions
- `tui_is_interactive()` - Check if running in TTY
- `tui_is_ci()` - Detect CI environment
- `tui_string_width()` - Get display width of string
- `tui_wrap_text()` - Wrap text to width
- `tui_truncate()` - Truncate text with ellipsis
- `tui_render()` - Mount component and start render loop
- `tui_rerender()` - Force re-render
- `tui_unmount()` - Unmount and restore terminal
- `tui_wait_until_exit()` - Block until app exits
- `tui_set_input_handler()` - Register input callback
- `tui_set_focus_handler()` - Register focus callback
- `tui_focus_next()` - Move focus to next element
- `tui_focus_prev()` - Move focus to previous element
- `tui_set_resize_handler()` - Register resize callback
- `tui_get_size()` - Get instance size
- `tui_get_focused_node()` - Get currently focused node

### Classes (All Implemented)
- `TuiBox` - Container with flexbox layout
- `TuiText` - Text content with styling
- `TuiInstance` - Control object for rendered app
- `TuiKey` - Key event data
- `TuiFocusEvent` - Focus change event

---

## Missing Features

### 1. Drawing Primitives (Priority: HIGH)

These low-level functions enable game-like graphics and custom visualizations.

```c
// Specified in SPEC.md but NOT implemented
void tui_draw_point(tui_buffer *buf, int x, int y, tui_style *style);
void tui_draw_line(tui_buffer *buf, int x1, int y1, int x2, int y2, ...);
void tui_draw_rect(tui_buffer *buf, int x, int y, int width, int height, ...);
void tui_fill_rect(tui_buffer *buf, int x, int y, int width, int height, ...);
void tui_draw_circle(tui_buffer *buf, int cx, int cy, int radius, ...);
void tui_fill_circle(tui_buffer *buf, int cx, int cy, int radius, ...);
void tui_draw_ellipse(tui_buffer *buf, int cx, int cy, int rx, int ry, ...);
void tui_draw_triangle(tui_buffer *buf, int x1, int y1, ...);
void tui_fill_triangle(tui_buffer *buf, int x1, int y1, ...);
```

**PHP API (missing):**
```php
function tui_draw_line(mixed $buffer, int $x1, int $y1, int $x2, int $y2, ...): void;
function tui_draw_rect(mixed $buffer, int $x, int $y, int $w, int $h, ...): void;
function tui_fill_rect(mixed $buffer, int $x, int $y, int $w, int $h, ...): void;
function tui_draw_circle(mixed $buffer, int $cx, int $cy, int $r, ...): void;
function tui_fill_circle(mixed $buffer, int $cx, int $cy, int $r, ...): void;
```

**Implementation Notes:**
- Requires Bresenham's line algorithm
- Requires midpoint circle algorithm
- Requires scanline fill for triangles
- Need to expose tui_buffer as PHP resource

---

### 2. Sprite Rendering (Priority: HIGH)

Animated ASCII art sprites with frame management and collision detection.

```c
// C structs (missing)
typedef struct tui_sprite_frame { ... };
typedef struct tui_sprite_animation { ... };
typedef struct tui_sprite { ... };

// C functions (missing)
void tui_blit_sprite(tui_buffer *buf, tui_sprite *sprite);
void tui_update_sprite(tui_sprite *sprite, int delta_ms);
void tui_flip_sprite(tui_sprite *sprite, bool flipped);
```

**PHP API (missing):**
```php
function tui_sprite_create(array $animations): mixed;
function tui_sprite_update(mixed $sprite, int $deltaMs): void;
function tui_sprite_set_animation(mixed $sprite, string $name): void;
function tui_sprite_set_position(mixed $sprite, int $x, int $y): void;
function tui_sprite_flip(mixed $sprite, bool $flipped): void;
function tui_sprite_render(mixed $sprite): void;
function tui_sprite_get_bounds(mixed $sprite): array;
function tui_sprite_collides(mixed $sprite1, mixed $sprite2): bool;
```

**Implementation Notes:**
- Need resource type for sprite handles
- Need frame timing in event loop
- AABB collision detection

---

### 3. Canvas API (Priority: HIGH)

High-resolution drawing using braille characters (2x4 dots per cell).

```c
// C structs (missing)
typedef enum { TUI_CANVAS_BRAILLE, TUI_CANVAS_BLOCK, TUI_CANVAS_ASCII } tui_canvas_mode;
typedef struct tui_canvas { ... };

// C functions (missing)
tui_canvas* tui_canvas_create(int width, int height, tui_canvas_mode mode);
void tui_canvas_set(tui_canvas *canvas, int x, int y);
void tui_canvas_clear(tui_canvas *canvas, int x, int y);
void tui_canvas_line(tui_canvas *canvas, int x1, int y1, int x2, int y2);
void tui_canvas_circle(tui_canvas *canvas, int cx, int cy, int radius, bool filled);
void tui_canvas_render(tui_canvas *canvas, tui_buffer *buf, int x, int y);
void tui_canvas_free(tui_canvas *canvas);
```

**PHP API (missing):**
```php
function tui_canvas_create(int $width, int $height, string $mode = 'braille'): mixed;
function tui_canvas_set(mixed $canvas, int $x, int $y): void;
function tui_canvas_clear(mixed $canvas, int $x, int $y): void;
function tui_canvas_line(mixed $canvas, int $x1, int $y1, int $x2, int $y2): void;
function tui_canvas_rect(mixed $canvas, int $x, int $y, int $w, int $h, bool $filled = false): void;
function tui_canvas_circle(mixed $canvas, int $cx, int $cy, int $r, bool $filled = false): void;
function tui_canvas_plot(mixed $canvas, callable $fn): void;
function tui_canvas_get_resolution(mixed $canvas): array;
function tui_canvas_render(mixed $canvas): array;
```

**Implementation Notes:**
- Braille Unicode block: U+2800 to U+28FF
- Each braille character represents 2x4 pixel matrix
- Need bit array for pixel storage
- Braille dot mapping table required

---

### 4. Animation Utilities (Priority: MEDIUM)

Easing functions and interpolation for smooth animations.

```c
// C functions (missing)
double tui_ease(double t, tui_easing easing);
double tui_lerp(double a, double b, double t);
tui_color tui_lerp_color(tui_color a, tui_color b, double t);
```

**PHP API (missing):**
```php
function tui_ease(float $t, string $easing = 'linear'): float;
function tui_lerp(float $a, float $b, float $t): float;
function tui_lerp_color(string $colorA, string $colorB, float $t): string;
function tui_gradient(array $colors, int $steps): array;
```

**Easing Functions to Implement:**
- Linear
- In/Out/InOut: Quad, Cubic, Elastic, Bounce

---

### 5. Table Rendering (Priority: MEDIUM)

Optimized table layout and rendering.

```c
// C functions (missing)
void tui_table_layout(tui_table *table, int max_width);
void tui_render_table(tui_buffer *buf, tui_table *table, ...);
```

**PHP API (missing):**
```php
function tui_table_layout(array $headers, array $rows, int $maxWidth): array;
function tui_render_table(array $headers, array $rows, ...): void;
```

---

### 6. Progress/Spinner Rendering (Priority: MEDIUM)

Native progress bar and spinner rendering.

```c
// C functions (missing)
void tui_render_progress(tui_buffer *buf, ...);
void tui_render_busy(tui_buffer *buf, ...);
void tui_render_spinner(tui_buffer *buf, ...);
```

**PHP API (missing):**
```php
function tui_render_progress(int $x, int $y, int $width, float $progress, ...): void;
function tui_render_busy(int $x, int $y, int $width, int $frame, ...): void;
function tui_spinner_frame(array $frames, int $intervalMs = 80): string;
```

---

### 7. Text Utilities (Priority: LOW)

```php
// Missing
function tui_pad(string $text, int $width, string $align = 'left', string $char = ' '): string;
```

---

### 8. Missing TuiBox Properties (Priority: HIGH)

These properties are in the SPEC but not declared in TuiBox class:

| Property | Type | Purpose |
|----------|------|---------|
| `flexWrap` | string | `'nowrap'\|'wrap'\|'wrap-reverse'` |
| `minWidth` | int/string | Minimum width constraint |
| `minHeight` | int/string | Minimum height constraint |
| `maxWidth` | int/string | Maximum width constraint |
| `maxHeight` | int/string | Maximum height constraint |
| `overflow` | string | `'visible'\|'hidden'` |
| `overflowX` | string | Horizontal overflow |
| `overflowY` | string | Vertical overflow |
| `display` | string | `'flex'\|'none'` |
| `position` | string | `'relative'\|'absolute'` |
| `columnGap` | int | Gap between columns |
| `rowGap` | int | Gap between rows |

**Note:** The C code reads these properties but they're not declared, causing PHP 8.4 deprecation warnings.

---

### 9. Missing Component: tui/widgets Package

The SPEC describes a separate `tui/widgets` Composer package with:

| Widget | Description |
|--------|-------------|
| `Canvas` | High-res braille/block drawing |
| `Sprite` | Animated ASCII art |
| `Table` | Data tables |
| `ProgressBar` | Progress indicator |
| `BusyBar` | Indeterminate progress |
| `Shape` | Basic shapes |
| `Line` | Lines |
| `TodoList` | Task lists |
| `Collapsible` | Expandable sections |
| `Select` | Selection widgets |
| `Form` | Form fields |

**Status:** Package does not exist yet.

---

## Implementation Roadmap

### Phase 1: Core Stability (Required)
1. Add missing TuiBox property declarations (PHP 8.4 fix)
2. Implement `tui_pad()` text utility

### Phase 2: Graphics Foundation (High Priority)
1. Create `tui_buffer` resource type
2. Implement drawing primitives (`tui_draw_*`, `tui_fill_*`)
3. Implement canvas API (`tui_canvas_*`)

### Phase 3: Animation System (High Priority)
1. Implement sprite structures and rendering
2. Implement animation utilities (easing, lerp)
3. Add sprite functions to event loop

### Phase 4: Advanced Widgets (Medium Priority)
1. Implement table layout and rendering
2. Implement progress/spinner rendering
3. Create `tui/widgets` Composer package

---

## File Locations for Implementation

```
ext-tui/
├── src/
│   ├── drawing/           # NEW - Drawing primitives
│   │   ├── primitives.c   # Lines, rects, circles
│   │   ├── primitives.h
│   │   ├── sprite.c       # Sprite rendering
│   │   ├── sprite.h
│   │   ├── canvas.c       # Braille/block canvas
│   │   ├── canvas.h
│   │   ├── animation.c    # Easing, lerp
│   │   ├── animation.h
│   │   ├── progress.c     # Progress bars
│   │   ├── progress.h
│   │   ├── table.c        # Table layout
│   │   └── table.h
│   └── ... (existing)
└── tui.c                   # Add new PHP functions
```

---

## Estimated Effort

| Feature Set | Complexity | LOC (C) | LOC (PHP stubs) |
|-------------|------------|---------|-----------------|
| TuiBox props | Low | ~50 | 0 |
| Drawing primitives | Medium | ~400 | ~50 |
| Canvas API | Medium | ~300 | ~50 |
| Sprite system | High | ~500 | ~80 |
| Animation utils | Low | ~200 | ~30 |
| Table rendering | Medium | ~300 | ~40 |
| Progress widgets | Low | ~150 | ~30 |
| **Total** | | **~1900** | **~280** |

---

## References

- SPEC.md: `/Users/jkeppens/Desktop/tools/exocoder/docs/tui/SPEC.md`
- Current implementation: `/Users/jkeppens/Desktop/tools/ext-tui/tui.c`
- Braille Unicode: https://en.wikipedia.org/wiki/Braille_Patterns
