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

---

## Mouse

### tui_mouse_enable

```php
tui_mouse_enable(int $mode = TUI_MOUSE_MODE_BUTTON): bool
```

Enables mouse tracking. Returns `true` on success.

Modes: `TUI_MOUSE_MODE_CLICK`, `TUI_MOUSE_MODE_BUTTON` (default), `TUI_MOUSE_MODE_ALL`.

### tui_mouse_disable

```php
tui_mouse_disable(): bool
```

Disables mouse tracking. Returns `true` on success.

---

## Clipboard (OSC 52)

### tui_clipboard_copy

```php
tui_clipboard_copy(string $text, string $target = "clipboard"): bool
```

Copies text to clipboard. Returns `true` on success.

**Targets:** `"clipboard"` (default), `"primary"`, `"secondary"`

### tui_clipboard_request

```php
tui_clipboard_request(string $target = "clipboard"): void
```

Requests clipboard contents (async). Response arrives via input handler.

### tui_clipboard_clear

```php
tui_clipboard_clear(string $target = "clipboard"): void
```

Clears clipboard.

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
tui_bracketed_paste_enable(): bool
```

Enables bracketed paste mode. Returns `true` on success.

### tui_bracketed_paste_disable

```php
tui_bracketed_paste_disable(): bool
```

Disables bracketed paste mode. Returns `true` on success.

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
tui_canvas_create(int $width, int $height, string $mode = "braille"): resource
```

Modes: `"braille"` (2×4 per cell), `"block"` (2×2 per cell), `"ascii"` (1×1 per cell)

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
tui_ease(float $t, string $easing = "linear"): float
```

Apply easing to `t` (0.0-1.0).

**Easing types:** `"linear"`, `"in_quad"`, `"out_quad"`, `"in_out_quad"`, `"in_cubic"`, `"out_cubic"`, `"in_out_cubic"`, `"in_quart"`, `"out_quart"`, `"in_out_quart"`, `"in_sine"`, `"out_sine"`, `"in_out_sine"`, `"out_bounce"`, `"out_elastic"`, `"out_back"`

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
tui_table_set_align(resource $table, int $column, bool $right_align): void
```

Sets column alignment. Pass `true` for right-align, `false` for left-align (default).

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
tui_render_busy_bar(resource $buffer, int $x, int $y, int $width, int $frame, string $style_name = "pulse", ?array $style = null): void
```

Renders an animated busy/indeterminate bar to a buffer.

### tui_spinner_frame

```php
tui_spinner_frame(string $type, int $frame): string
```

Returns the character(s) for a spinner animation frame.

**Types:** `"dots"`, `"line"`, `"bounce"`, `"circle"`

### tui_spinner_frame_count

```php
tui_spinner_frame_count(string $type): int
```

Returns the number of frames in a spinner animation.

### tui_render_spinner

```php
tui_render_spinner(resource $buffer, int $x, int $y, string $type, int $frame, ?array $style = null): void
```

Renders a spinner animation frame to a buffer.

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

---

## Virtual List / Windowing

Efficient rendering of large lists by only rendering visible items.

### tui_virtual_create

```php
tui_virtual_create(int $itemCount, int $itemHeight, int $viewportHeight, int $overscan = 5): resource
```

Creates a virtual list for windowed rendering.

**Parameters:**
- `itemCount`: Total number of items in the list
- `itemHeight`: Height of each item in rows (fixed)
- `viewportHeight`: Height of visible area in rows
- `overscan`: Extra items to render above/below visible area (default: 5)

### tui_virtual_get_range

```php
tui_virtual_get_range(resource $vlist): array
```

Returns current visible range:
- `start` (int): First visible item index (including overscan)
- `end` (int): Last visible item index (exclusive, including overscan)
- `offset` (float): Current scroll offset in rows
- `progress` (float): Scroll progress 0.0 to 1.0
- `item_count` (int): Total items
- `viewport_height` (int): Viewport height
- `max_scroll` (float): Maximum scroll offset

### tui_virtual_scroll_to

```php
tui_virtual_scroll_to(resource $vlist, int $index): void
```

Scrolls to show item at top of viewport.

### tui_virtual_scroll_by

```php
tui_virtual_scroll_by(resource $vlist, int $delta): void
```

Scrolls by a number of rows (positive = down, negative = up).

### tui_virtual_scroll_items

```php
tui_virtual_scroll_items(resource $vlist, int $items): void
```

Scrolls by a number of items.

### tui_virtual_ensure_visible

```php
tui_virtual_ensure_visible(resource $vlist, int $index): void
```

Ensures item is visible, scrolling minimally if needed.

### tui_virtual_page_up / page_down

```php
tui_virtual_page_up(resource $vlist): void
tui_virtual_page_down(resource $vlist): void
```

Scrolls by one page (viewport height - 1).

### tui_virtual_scroll_top / scroll_bottom

```php
tui_virtual_scroll_top(resource $vlist): void
tui_virtual_scroll_bottom(resource $vlist): void
```

Scrolls to top or bottom of list.

### tui_virtual_set_count

```php
tui_virtual_set_count(resource $vlist, int $itemCount): void
```

Updates total item count (e.g., after filtering).

### tui_virtual_set_viewport

```php
tui_virtual_set_viewport(resource $vlist, int $viewportHeight): void
```

Updates viewport height (e.g., after resize).

### tui_virtual_item_offset

```php
tui_virtual_item_offset(resource $vlist, int $index): int
```

Returns Y offset of item relative to viewport (may be negative).

### tui_virtual_is_visible

```php
tui_virtual_is_visible(resource $vlist, int $index): bool
```

Returns true if item is in visible range (including overscan).

### tui_virtual_destroy

```php
tui_virtual_destroy(resource $vlist): void
```

Destroys virtual list resource.

---

## Smooth Scrolling

Spring-physics based smooth scrolling animation.

### tui_scroll_create

```php
tui_scroll_create(): resource
```

Creates a smooth scroll animation controller.

### tui_scroll_set_spring

```php
tui_scroll_set_spring(resource $anim, float $stiffness, float $damping): void
```

Sets spring physics parameters.

**Parameters:**
- `stiffness`: Spring constant (default: 200.0, range: 50-500). Higher = faster.
- `damping`: Damping ratio (default: 1.0 = critically damped).
  - `< 1.0`: Underdamped (oscillates)
  - `= 1.0`: Critically damped (fastest without oscillation)
  - `> 1.0`: Overdamped (slower convergence)

### tui_scroll_set_target

```php
tui_scroll_set_target(resource $anim, float $x, float $y): void
```

Sets the target scroll position. Animation begins automatically.

### tui_scroll_by

```php
tui_scroll_by(resource $anim, float $dx, float $dy): void
```

Adds to current target (incremental scrolling).

### tui_scroll_update

```php
tui_scroll_update(resource $anim, float $dt): bool
```

Updates animation for one frame. Returns `true` if still animating.

**Parameters:**
- `dt`: Time delta in seconds (e.g., `1.0/60.0` for 60fps)

### tui_scroll_snap

```php
tui_scroll_snap(resource $anim): void
```

Immediately jumps to target position (cancels animation).

### tui_scroll_get_position

```php
tui_scroll_get_position(resource $anim): array
```

Returns current scroll position as `['x' => float, 'y' => float]`.

### tui_scroll_is_animating

```php
tui_scroll_is_animating(resource $anim): bool
```

Returns `true` if animation is in progress.

### tui_scroll_progress

```php
tui_scroll_progress(resource $anim): float
```

Returns animation progress (0.0 to 1.0).

### tui_scroll_destroy

```php
tui_scroll_destroy(resource $anim): void
```

Destroys scroll animation resource.

---

## Graphics (Kitty Protocol)

Display images in terminals that support the Kitty graphics protocol (Kitty, WezTerm).

### tui_graphics_supported

```php
tui_graphics_supported(): bool
```

Check if the terminal supports Kitty graphics protocol.

**Returns:** `true` if Kitty graphics is supported, `false` otherwise.

### tui_image_load

```php
tui_image_load(string $path): resource
```

Load an image from a file path. Automatically detects PNG format.

**Parameters:**
- `$path`: Path to image file (PNG supported)

**Returns:** Image resource

**Throws:** `Exception` if file cannot be loaded

### tui_image_create

```php
tui_image_create(string $data, int $width, int $height, string $format = 'rgba'): resource
```

Create an image from raw pixel data.

**Parameters:**
- `$data`: Raw pixel data
- `$width`: Image width in pixels
- `$height`: Image height in pixels
- `$format`: `'rgba'` (4 bytes/pixel), `'rgb'` (3 bytes/pixel), or `'png'`

**Returns:** Image resource

**Throws:** `ValueError` if dimensions or format are invalid, or data length doesn't match

### tui_image_transmit

```php
tui_image_transmit(resource $image): bool
```

Transmit image to terminal memory. Uses chunked transmission for large images.

**Parameters:**
- `$image`: Image resource

**Returns:** `true` on success, `false` on failure

### tui_image_display

```php
tui_image_display(resource $image, int $x, int $y, int $cols = 0, int $rows = 0): bool
```

Display an image at the specified position. Automatically transmits if not already transmitted.

**Parameters:**
- `$image`: Image resource
- `$x`: Column position (0-based)
- `$y`: Row position (0-based)
- `$cols`: Display width in cells (0 = auto)
- `$rows`: Display height in cells (0 = auto)

**Returns:** `true` on success, `false` on failure

### tui_image_delete

```php
tui_image_delete(resource $image): bool
```

Delete image from terminal memory.

**Parameters:**
- `$image`: Image resource

**Returns:** `true` on success, `false` on failure

### tui_image_clear

```php
tui_image_clear(): void
```

Clear all images from terminal.

### tui_image_destroy

```php
tui_image_destroy(resource $image): void
```

Destroy image resource and free memory. If the image was transmitted, it's also deleted from the terminal.

### tui_image_get_info

```php
tui_image_get_info(resource $image): array
```

Get image metadata.

**Returns:** Array with:
- `width`: Image width in pixels
- `height`: Image height in pixels
- `format`: `'png'`, `'rgb'`, or `'rgba'`
- `state`: `'empty'`, `'loaded'`, `'transmitted'`, or `'displayed'`
- `data_size`: Size of image data in bytes
- `image_id`: Terminal-assigned image ID (0 if not transmitted)

### Example Usage

```php
<?php
// Check if graphics is supported
if (!tui_graphics_supported()) {
    die("Kitty graphics not supported in this terminal\n");
}

// Load and display an image
$image = tui_image_load('/path/to/image.png');
tui_image_display($image, 0, 0, 40, 20);

// Wait for input
fgets(STDIN);

// Clean up
tui_image_destroy($image);
```

### Terminal Compatibility

| Terminal | Support |
|----------|---------|
| Kitty | Full |
| WezTerm | Full |
| Konsole | Partial |
| iTerm2 | No (uses different protocol) |
| Apple Terminal | No |

---

## Notifications

Terminal notifications and alerts.

### tui_bell

```php
tui_bell(): void
```

Sends terminal bell (audible beep).

### tui_flash

```php
tui_flash(): void
```

Sends visual bell (screen flash).

### tui_notify

```php
tui_notify(string $title, ?string $body = null, int $priority = 0): bool
```

Sends desktop notification via OSC 9/777.

**Parameters:**
- `title`: Notification title
- `body`: Optional notification body text
- `priority`: 0 = normal, 1 = low, 2 = critical

**Returns:** `true` if notification was sent

---

## Input Parsing

Low-level input sequence parsing.

### tui_parse_mouse

```php
tui_parse_mouse(string $input): ?array
```

Parses SGR mouse escape sequence.

**Returns:** Array with mouse event info or `null` if invalid:
- `x` (int): Column (0-based)
- `y` (int): Row (0-based)
- `button` (string): `'left'`, `'middle'`, `'right'`, `'scroll_up'`, `'scroll_down'`, `'none'`
- `action` (string): `'press'`, `'release'`, `'move'`, `'drag'`
- `ctrl` (bool): Ctrl modifier
- `meta` (bool): Meta/Alt modifier
- `shift` (bool): Shift modifier
- `consumed` (int): Bytes consumed from input

### tui_parse_key

```php
tui_parse_key(string $input): ?array
```

Parses keyboard escape sequence.

**Returns:** Array with key info or `null` if invalid:
- `key` (string): Character or key name
- `ctrl` (bool): Ctrl modifier
- `alt` (bool): Alt modifier
- `shift` (bool): Shift modifier
- `consumed` (int): Bytes consumed from input

---

## ANSI Text Processing

Handle strings containing ANSI escape codes.

### tui_strip_ansi

```php
tui_strip_ansi(string $text): string
```

Removes all ANSI escape codes from text.

### tui_string_width_ansi

```php
tui_string_width_ansi(string $text): int
```

Returns display width of text, ignoring ANSI escape codes.

### tui_slice_ansi

```php
tui_slice_ansi(string $text, int $start, int $end): string
```

Slices text by display width, preserving ANSI codes.

**Parameters:**
- `text`: Text with ANSI codes
- `start`: Start position (display column)
- `end`: End position (display column)

---

## Grapheme Counting

### tui_grapheme_count

```php
tui_grapheme_count(string $text): int
```

Counts grapheme clusters in text. A grapheme cluster is a user-perceived character (e.g., emoji with modifiers count as 1).

---

## Accessibility

Accessibility and user preference detection.

### tui_announce

```php
tui_announce(string $message, string $priority = "polite"): bool
```

Announces message to screen readers.

**Parameters:**
- `message`: Message to announce
- `priority`: `"polite"` (wait for pause) or `"assertive"` (interrupt)

**Returns:** `true` if announcement was sent

### tui_prefers_reduced_motion

```php
tui_prefers_reduced_motion(): bool
```

Returns `true` if user prefers reduced motion (from `REDUCE_MOTION` env or system setting).

### tui_prefers_high_contrast

```php
tui_prefers_high_contrast(): bool
```

Returns `true` if user prefers high contrast.

### tui_get_accessibility_features

```php
tui_get_accessibility_features(): array
```

Returns array of accessibility features:
- `reduced_motion` (bool)
- `high_contrast` (bool)
- `screen_reader` (bool): Screen reader detected

### tui_aria_role_to_string

```php
tui_aria_role_to_string(int $role): string
```

Converts ARIA role constant to string name.

### tui_aria_role_from_string

```php
tui_aria_role_from_string(string $role): int
```

Converts ARIA role string to constant.

---

## Drag & Drop

Drag and drop operations.

### tui_drag_start

```php
tui_drag_start(int $x, int $y, string $type, string $data): bool
```

Starts a drag operation.

**Parameters:**
- `x`, `y`: Starting position
- `type`: MIME type or custom type (e.g., `'text/plain'`, `'application/x-custom'`)
- `data`: Data being dragged

**Returns:** `true` if drag started successfully

### tui_drag_move

```php
tui_drag_move(int $x, int $y): bool
```

Updates drag position.

**Returns:** `true` if drag is active

### tui_drag_end

```php
tui_drag_end(bool $dropped = true): bool
```

Ends drag operation.

**Parameters:**
- `dropped`: Whether the drag resulted in a drop

**Returns:** `true` if drag was active

### tui_drag_cancel

```php
tui_drag_cancel(): bool
```

Cancels active drag operation.

**Returns:** `true` if drag was cancelled

### tui_drag_is_active

```php
tui_drag_is_active(): bool
```

Returns `true` if a drag is in progress.

### tui_drag_get_type

```php
tui_drag_get_type(): ?string
```

Returns current drag type or `null` if no drag active.

### tui_drag_get_data

```php
tui_drag_get_data(): ?string
```

Returns current drag data or `null` if no drag active.

### tui_drag_get_state

```php
tui_drag_get_state(): array
```

Returns drag state:
- `active` (bool)
- `x` (int): Current X position
- `y` (int): Current Y position
- `start_x` (int): Starting X position
- `start_y` (int): Starting Y position
- `type` (?string): Drag type
- `data` (?string): Drag data

---

## Screen Recording

Record terminal output for playback or export.

### tui_record_create

```php
tui_record_create(int $width, int $height, ?string $title = null): resource
```

Creates a screen recorder.

**Parameters:**
- `width`, `height`: Recording dimensions
- `title`: Optional recording title

### tui_record_start

```php
tui_record_start(resource $recording): bool
```

Starts recording.

### tui_record_pause

```php
tui_record_pause(resource $recording): bool
```

Pauses recording.

### tui_record_resume

```php
tui_record_resume(resource $recording): bool
```

Resumes paused recording.

### tui_record_stop

```php
tui_record_stop(resource $recording): bool
```

Stops recording.

### tui_record_capture

```php
tui_record_capture(resource $recording, string $data): bool
```

Captures a frame of terminal output.

### tui_record_duration

```php
tui_record_duration(resource $recording): float
```

Returns recording duration in seconds.

### tui_record_frame_count

```php
tui_record_frame_count(resource $recording): int
```

Returns number of captured frames.

### tui_record_export

```php
tui_record_export(resource $recording): string
```

Exports recording to asciicast format.

### tui_record_save

```php
tui_record_save(resource $recording, string $path): bool
```

Saves recording to file.

### tui_record_destroy

```php
tui_record_destroy(resource $recording): void
```

Destroys recording resource.

---

## Graphics Protocol Detection

Detect terminal graphics capabilities beyond Kitty protocol.

### tui_graphics_protocol

```php
tui_graphics_protocol(): string
```

Returns detected graphics protocol: `'kitty'`, `'iterm2'`, `'sixel'`, or `'none'`.

### tui_iterm2_supported

```php
tui_iterm2_supported(): bool
```

Returns `true` if iTerm2 inline images are supported.

### tui_sixel_supported

```php
tui_sixel_supported(): bool
```

Returns `true` if Sixel graphics are supported.

---

## Testing Framework

Headless testing utilities for TUI applications.

### tui_test_create

```php
tui_test_create(int $width, int $height): resource
```

Creates a test renderer with specified dimensions.

### tui_test_destroy

```php
tui_test_destroy(resource $renderer): void
```

Destroys test renderer.

### tui_test_render

```php
tui_test_render(resource $renderer, ContainerNode $element): void
```

Renders element tree to test renderer.

### tui_test_get_output

```php
tui_test_get_output(resource $renderer): array
```

Returns rendered output as array of lines.

### tui_test_to_string

```php
tui_test_to_string(resource $renderer): string
```

Returns rendered output as single string.

### tui_test_send_input

```php
tui_test_send_input(resource $renderer, string $input): void
```

Simulates raw input string.

### tui_test_send_key

```php
tui_test_send_key(resource $renderer, int $keyCode): void
```

Simulates key press.

### tui_test_advance_frame

```php
tui_test_advance_frame(resource $renderer): void
```

Advances to next frame.

### tui_test_run_timers

```php
tui_test_run_timers(resource $renderer, int $ms): void
```

Runs timers for specified milliseconds.

### tui_test_get_by_id

```php
tui_test_get_by_id(resource $renderer, string $id): mixed
```

Finds element by ID. Returns element info or `null`.

### tui_test_get_by_text

```php
tui_test_get_by_text(resource $renderer, string $text): array
```

Finds elements containing text. Returns array of matches.

---

## Performance Metrics

Performance monitoring and profiling.

### tui_metrics_enable

```php
tui_metrics_enable(): void
```

Enables performance metrics collection.

### tui_metrics_disable

```php
tui_metrics_disable(): void
```

Disables performance metrics collection.

### tui_metrics_enabled

```php
tui_metrics_enabled(): bool
```

Returns `true` if metrics collection is enabled.

### tui_metrics_reset

```php
tui_metrics_reset(): void
```

Resets all collected metrics.

### tui_get_metrics

```php
tui_get_metrics(): array
```

Returns overall metrics summary.

### tui_get_node_metrics

```php
tui_get_node_metrics(): array
```

Returns node-related metrics (creation, destruction counts).

### tui_get_reconciler_metrics

```php
tui_get_reconciler_metrics(): array
```

Returns reconciler metrics (diff operations, updates).

### tui_get_render_metrics

```php
tui_get_render_metrics(): array
```

Returns render metrics (frame times, buffer operations).

### tui_get_loop_metrics

```php
tui_get_loop_metrics(): array
```

Returns event loop metrics (input processing, timer execution).

### tui_get_pool_metrics

```php
tui_get_pool_metrics(): array
```

Returns memory pool metrics (allocations, cache hits).

---

## Additional Functions

### tui_mouse_get_mode

```php
tui_mouse_get_mode(): int
```

Returns current mouse tracking mode: `TUI_MOUSE_MODE_CLICK`, `TUI_MOUSE_MODE_BUTTON`, `TUI_MOUSE_MODE_ALL`, or `0` if disabled.

### tui_bracketed_paste_is_enabled

```php
tui_bracketed_paste_is_enabled(): bool
```

Returns `true` if bracketed paste mode is enabled.
