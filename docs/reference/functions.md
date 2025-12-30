# Function Reference

Complete list of ext-tui functions.

## Application Lifecycle

### tui_render

```php
tui_render(callable $component, array $options = []): Xocdr\Tui\Ext\Instance
```

Renders a TUI component tree.

**Options:**
- `fullscreen` (bool): Use alternate screen buffer. Default: `true`
- `exitOnCtrlC` (bool): Exit on Ctrl+C. Default: `true`

### tui_rerender

```php
tui_rerender(Xocdr\Tui\Ext\Instance $instance): void
```

Forces re-render of component tree.

### tui_unmount

```php
tui_unmount(Xocdr\Tui\Ext\Instance $instance): void
```

Stops TUI and restores terminal.

### tui_wait_until_exit

```php
tui_wait_until_exit(Xocdr\Tui\Ext\Instance $instance): void
```

Blocks until TUI exits.

---

## Event Handlers

### tui_set_input_handler

```php
tui_set_input_handler(Xocdr\Tui\Ext\Instance $instance, callable $handler): void
```

Handler receives: `function(Xocdr\Tui\Ext\Key $key): void`

### tui_set_focus_handler

```php
tui_set_focus_handler(Xocdr\Tui\Ext\Instance $instance, callable $handler): void
```

Handler receives: `function(Xocdr\Tui\Ext\FocusEvent $event): void`

### tui_set_resize_handler

```php
tui_set_resize_handler(Xocdr\Tui\Ext\Instance $instance, callable $handler): void
```

Handler receives: `function(int $width, int $height): void`

### tui_set_tick_handler

```php
tui_set_tick_handler(Xocdr\Tui\Ext\Instance $instance, callable $handler): void
```

Handler called each frame.

---

## Focus Management

### tui_focus_next

```php
tui_focus_next(Xocdr\Tui\Ext\Instance $instance): void
```

Moves focus to next focusable element.

### tui_focus_prev

```php
tui_focus_prev(Xocdr\Tui\Ext\Instance $instance): void
```

Moves focus to previous focusable element.

### tui_get_focused_node

```php
tui_get_focused_node(Xocdr\Tui\Ext\Instance $instance): ?array
```

Returns info about focused node or null.

### tui_focus_next_in_group

```php
tui_focus_next_in_group(Xocdr\Tui\Ext\Instance $instance, string $group): void
```

Moves focus to next element within specified focus group.

### tui_focus_by_id

```php
tui_focus_by_id(Xocdr\Tui\Ext\Instance $instance, string $id): void
```

Focuses element by ID.

---

## Mouse

### tui_mouse_enable

```php
tui_mouse_enable(Xocdr\Tui\Ext\Instance $instance, int $mode): void
```

Enables mouse tracking. Modes: `TUI_MOUSE_MODE_CLICK`, `TUI_MOUSE_MODE_BUTTON`, `TUI_MOUSE_MODE_ALL`.

### tui_mouse_disable

```php
tui_mouse_disable(Xocdr\Tui\Ext\Instance $instance): void
```

Disables mouse tracking.

### tui_set_mouse_handler

```php
tui_set_mouse_handler(Xocdr\Tui\Ext\Instance $instance, callable $handler): void
```

Handler receives: `function(Xocdr\Tui\Ext\MouseEvent $event): void`

### tui_hit_test

```php
tui_hit_test(Xocdr\Tui\Ext\Instance $instance, int $x, int $y): ?array
```

Returns deepest node at coordinates, or null.

### tui_hit_test_all

```php
tui_hit_test_all(Xocdr\Tui\Ext\Instance $instance, int $x, int $y): array
```

Returns all nodes at coordinates (root to leaf order).

---

## Clipboard (OSC 52)

### tui_clipboard_write

```php
tui_clipboard_write(string $text, int $target = TUI_CLIPBOARD_CLIPBOARD): void
```

Writes text to clipboard.

### tui_clipboard_request

```php
tui_clipboard_request(int $target = TUI_CLIPBOARD_CLIPBOARD): void
```

Requests clipboard contents (async).

### tui_clipboard_clear

```php
tui_clipboard_clear(int $target = TUI_CLIPBOARD_CLIPBOARD): void
```

Clears clipboard.

### tui_set_clipboard_handler

```php
tui_set_clipboard_handler(Xocdr\Tui\Ext\Instance $instance, callable $handler): void
```

Handler receives: `function(string $content): void`

---

## Input History

### tui_history_create

```php
tui_history_create(int $maxEntries = 1000): resource
```

Creates input history with max capacity.

### tui_history_destroy

```php
tui_history_destroy(resource $history): void
```

Frees history resources.

### tui_history_add

```php
tui_history_add(resource $history, string $entry): void
```

Adds entry to history (deduplicates consecutive entries).

### tui_history_prev

```php
tui_history_prev(resource $history): ?string
```

Returns previous history entry, or null.

### tui_history_next

```php
tui_history_next(resource $history): ?string
```

Returns next history entry, or null.

### tui_history_save_temp

```php
tui_history_save_temp(resource $history, string $input): void
```

Saves current input before navigating history.

### tui_history_get_temp

```php
tui_history_get_temp(resource $history): ?string
```

Returns saved temporary input.

### tui_history_reset

```php
tui_history_reset(resource $history): void
```

Resets navigation position.

---

## Bracketed Paste

### tui_bracketed_paste_enable

```php
tui_bracketed_paste_enable(Xocdr\Tui\Ext\Instance $instance): void
```

Enables bracketed paste mode.

### tui_bracketed_paste_disable

```php
tui_bracketed_paste_disable(Xocdr\Tui\Ext\Instance $instance): void
```

Disables bracketed paste mode.

### tui_set_paste_handler

```php
tui_set_paste_handler(Xocdr\Tui\Ext\Instance $instance, callable $handler): void
```

Handler receives: `function(string $text): void`

---

## Timers

### tui_add_timer

```php
tui_add_timer(Xocdr\Tui\Ext\Instance $instance, callable $callback, int $intervalMs): int
```

Adds repeating timer. Returns timer ID.

### tui_remove_timer

```php
tui_remove_timer(Xocdr\Tui\Ext\Instance $instance, int $timerId): void
```

Removes timer by ID.

---

## Terminal Info

### tui_get_terminal_size

```php
tui_get_terminal_size(): array
```

Returns `[int $width, int $height]`.

### tui_get_size

```php
tui_get_size(Xocdr\Tui\Ext\Instance $instance): ?array
```

Returns `['width' => int, 'height' => int, 'columns' => int, 'rows' => int]`.

### tui_is_interactive

```php
tui_is_interactive(): bool
```

Returns true if stdin/stdout are TTY.

### tui_is_ci

```php
tui_is_ci(): bool
```

Returns true if running in CI environment.

---

## Cursor Control

### tui_cursor_shape

```php
tui_cursor_shape(string $shape): void
```

Sets cursor shape. Shapes: `default`, `block`, `block_blink`, `underline`, `underline_blink`, `bar`, `bar_blink`.

### tui_cursor_show

```php
tui_cursor_show(): void
```

Shows cursor.

### tui_cursor_hide

```php
tui_cursor_hide(): void
```

Hides cursor.

---

## Window Title

### tui_set_title

```php
tui_set_title(string $title): void
```

Sets terminal window/tab title via OSC 2.

### tui_reset_title

```php
tui_reset_title(): void
```

Resets window title to empty.

---

## Capability Detection

### tui_get_capabilities

```php
tui_get_capabilities(): array
```

Returns terminal capabilities:
- `terminal` (string): Terminal type (`kitty`, `iterm2`, `vte`, etc.)
- `name` (string): Terminal name
- `version` (string): Version if available
- `color_depth` (int): 8, 256, or 16777216 (24-bit)
- `capabilities` (array): Boolean flags for supported features

Capabilities flags:
- `true_color`, `256_color`
- `mouse`, `mouse_sgr`
- `bracketed_paste`, `clipboard_osc52`, `hyperlinks_osc8`
- `sync_output`, `unicode`
- `kitty_keyboard`, `kitty_graphics`, `sixel`
- `cursor_shape`, `title`, `focus_events`, `alternate_screen`

### tui_has_capability

```php
tui_has_capability(string $name): bool
```

Checks if terminal supports a specific capability.

---

## Text Utilities

### tui_string_width

```php
tui_string_width(string $text): int
```

Returns display width (handles Unicode, wide chars).

### tui_wrap_text

```php
tui_wrap_text(string $text, int $width): array
```

Wraps text to width. Returns array of lines.

### tui_truncate

```php
tui_truncate(string $text, int $width, string $ellipsis = '...'): string
```

Truncates text with ellipsis.

### tui_pad

```php
tui_pad(string $text, int $width, string $align = 'left', string $char = ' '): string
```

Pads text to width. Align: `'left'`, `'right'`, `'center'`.

---

## Buffers

### tui_buffer_create

```php
tui_buffer_create(int $width, int $height): resource
```

Creates drawing buffer.

### tui_buffer_clear

```php
tui_buffer_clear(resource $buffer): void
```

Clears buffer contents.

### tui_buffer_render

```php
tui_buffer_render(resource $buffer): void
```

Renders buffer to terminal.

---

## Drawing Primitives

### tui_draw_line

```php
tui_draw_line(resource $buffer, int $x1, int $y1, int $x2, int $y2, array $style): void
```

### tui_draw_rect

```php
tui_draw_rect(resource $buffer, int $x, int $y, int $width, int $height, array $style): void
```

### tui_fill_rect

```php
tui_fill_rect(resource $buffer, int $x, int $y, int $width, int $height, array $style): void
```

### tui_draw_circle

```php
tui_draw_circle(resource $buffer, int $cx, int $cy, int $radius, array $style): void
```

### tui_fill_circle

```php
tui_fill_circle(resource $buffer, int $cx, int $cy, int $radius, array $style): void
```

### tui_draw_ellipse

```php
tui_draw_ellipse(resource $buffer, int $cx, int $cy, int $rx, int $ry, array $style): void
```

### tui_fill_ellipse

```php
tui_fill_ellipse(resource $buffer, int $cx, int $cy, int $rx, int $ry, array $style): void
```

### tui_draw_triangle

```php
tui_draw_triangle(resource $buffer, int $x1, int $y1, int $x2, int $y2, int $x3, int $y3, array $style): void
```

### tui_fill_triangle

```php
tui_fill_triangle(resource $buffer, int $x1, int $y1, int $x2, int $y2, int $x3, int $y3, array $style): void
```

**Style array:**
- `char` (string): Character to use
- `fg` (array): Foreground RGB `[r, g, b]`
- `bg` (array): Background RGB

---

## Canvas

### tui_canvas_create

```php
tui_canvas_create(int $width, int $height, int $mode = TUI_CANVAS_BRAILLE): resource
```

Modes: `TUI_CANVAS_BRAILLE`, `TUI_CANVAS_BLOCK`, `TUI_CANVAS_ASCII`

### tui_canvas_set / unset / toggle / get

```php
tui_canvas_set(resource $canvas, int $x, int $y): void
tui_canvas_unset(resource $canvas, int $x, int $y): void
tui_canvas_toggle(resource $canvas, int $x, int $y): void
tui_canvas_get(resource $canvas, int $x, int $y): bool
```

### tui_canvas_clear

```php
tui_canvas_clear(resource $canvas): void
```

### tui_canvas_line

```php
tui_canvas_line(resource $canvas, int $x1, int $y1, int $x2, int $y2): void
```

### tui_canvas_rect / fill_rect

```php
tui_canvas_rect(resource $canvas, int $x, int $y, int $w, int $h): void
tui_canvas_fill_rect(resource $canvas, int $x, int $y, int $w, int $h): void
```

### tui_canvas_circle / fill_circle

```php
tui_canvas_circle(resource $canvas, int $cx, int $cy, int $r): void
tui_canvas_fill_circle(resource $canvas, int $cx, int $cy, int $r): void
```

### tui_canvas_set_color

```php
tui_canvas_set_color(resource $canvas, array $rgb): void
```

### tui_canvas_get_resolution

```php
tui_canvas_get_resolution(resource $canvas): array
```

Returns `['width' => int, 'height' => int]` in pixels.

### tui_canvas_render

```php
tui_canvas_render(resource $canvas, array $style = []): array
```

Returns array of strings.

---

## Animation

### tui_ease

```php
tui_ease(float $t, int $easing = TUI_EASE_LINEAR): float
```

Apply easing to `t` (0.0-1.0).

### tui_lerp

```php
tui_lerp(float $start, float $end, float $t): float
```

Linear interpolation.

### tui_lerp_color

```php
tui_lerp_color(array $from, array $to, float $t): array
```

Color interpolation. Returns RGB array.

### tui_gradient

```php
tui_gradient(array $from, array $to, int $steps): array
```

Create gradient. Returns array of RGB arrays.

### tui_color_from_hex

```php
tui_color_from_hex(string $hex): array
```

Parse hex color to RGB.

---

## Tables

### tui_table_create

```php
tui_table_create(array $headers): resource
```

### tui_table_add_row

```php
tui_table_add_row(resource $table, array $cells): void
```

### tui_table_set_align

```php
tui_table_set_align(resource $table, int $column, int $align): void
```

Align: `TUI_ALIGN_LEFT`, `TUI_ALIGN_CENTER`, `TUI_ALIGN_RIGHT`

### tui_table_render_to_buffer

```php
tui_table_render_to_buffer(resource $table, resource $buffer, int $x, int $y, array $options = []): void
```

Options: `border`, `header_bold`, `header_color`

---

## Progress Indicators

### tui_render_progress_bar

```php
tui_render_progress_bar(float $progress, int $width, array $style = []): string
```

### tui_render_busy_bar

```php
tui_render_busy_bar(int $frame, int $width, array $options = []): string
```

### tui_spinner_frame

```php
tui_spinner_frame(int $frame, int $type = TUI_SPINNER_DOTS): string
```

Types: `TUI_SPINNER_DOTS`, `TUI_SPINNER_LINE`, `TUI_SPINNER_BOUNCE`, `TUI_SPINNER_CIRCLE`

### tui_spinner_frame_count

```php
tui_spinner_frame_count(int $type): int
```

### tui_render_spinner

```php
tui_render_spinner(int $frame, string $label, int $type, array $style = []): string
```

---

## Sprites

### tui_sprite_create

```php
tui_sprite_create(array $config): resource
```

### tui_sprite_update

```php
tui_sprite_update(resource $sprite): void
```

### tui_sprite_set_animation

```php
tui_sprite_set_animation(resource $sprite, string $name): void
```

### tui_sprite_set_position

```php
tui_sprite_set_position(resource $sprite, int $x, int $y): void
```

### tui_sprite_flip

```php
tui_sprite_flip(resource $sprite, bool $horizontal, bool $vertical): void
```

### tui_sprite_set_visible

```php
tui_sprite_set_visible(resource $sprite, bool $visible): void
```

### tui_sprite_render

```php
tui_sprite_render(resource $sprite, resource $buffer): void
```

### tui_sprite_get_bounds

```php
tui_sprite_get_bounds(resource $sprite): array
```

Returns `['x' => int, 'y' => int, 'width' => int, 'height' => int]`

### tui_sprite_collides

```php
tui_sprite_collides(resource $sprite1, resource $sprite2): bool
```
