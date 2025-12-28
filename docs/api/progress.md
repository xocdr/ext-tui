# API Reference: Progress Functions

[Back to Documentation](../README.md) | [Feature Guide](../features/progress.md)

---

## Constants

### Spinner Styles

| Constant | Characters | Description |
|----------|------------|-------------|
| `TUI_SPINNER_DOTS` | `⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏` | Braille dots |
| `TUI_SPINNER_LINE` | `-\|/` | Classic ASCII |
| `TUI_SPINNER_CIRCLE` | `◐◓◑◒` | Quarter circle |
| `TUI_SPINNER_SQUARE` | `◰◳◲◱` | Quarter square |
| `TUI_SPINNER_ARROW` | `←↖↑↗→↘↓↙` | Rotating arrow |

---

## tui_render_progress_bar

Render a progress bar to a buffer.

```php
void tui_render_progress_bar(resource $buffer, int $x, int $y, float $progress, int $width, array $style = [])
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$buffer` | resource | Target buffer |
| `$x` | int | X position |
| `$y` | int | Y position |
| `$progress` | float | Progress value 0.0 to 1.0 |
| `$width` | int | Total width in characters |
| `$style` | array | Optional style array |

### Return Value

None.

### Example

```php
// 75% complete, 40 chars wide
tui_render_progress_bar($buf, 5, 10, 0.75, 40, $style);
tui_buffer_render($buf);
```

### Notes

- Uses `█` for filled and `░` for empty portions
- Progress is clamped to 0.0-1.0 range
- Renders directly to buffer at specified position

---

## tui_render_busy_bar

Render an animated progress bar frame (indeterminate).

```php
void tui_render_busy_bar(resource $buffer, int $x, int $y, int $frame, int $width, array $style = [])
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$buffer` | resource | Target buffer |
| `$x` | int | X position |
| `$y` | int | Y position |
| `$frame` | int | Current animation frame |
| `$width` | int | Total width in characters |
| `$style` | array | Optional style array |

### Return Value

None.

### Example

```php
$frame = 0;
while ($loading) {
    tui_buffer_clear($buf);
    tui_render_busy_bar($buf, 5, 10, $frame++, 40, $style);
    tui_buffer_render($buf);
    usleep(50000);  // 20 FPS
}
```

### Notes

- Creates a "moving" animation effect
- Use for tasks with unknown duration

---

## tui_spinner_frame

Get a spinner character for the current frame.

```php
string tui_spinner_frame(int $frame, int $style = TUI_SPINNER_DOTS)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$frame` | int | Current animation frame |
| `$style` | int | Spinner style constant |

### Return Value

Returns a single spinner character.

### Example

```php
$frame = 0;
while ($loading) {
    $spinner = tui_spinner_frame($frame++, TUI_SPINNER_DOTS);
    tui_buffer_write($buf, 5, 5, "$spinner Loading...", $style);
    tui_buffer_render($buf);
    usleep(80000);  // ~12 FPS
}
```

### Notes

- Frame number wraps automatically
- Different styles have different frame counts

---

## tui_spinner_frame_count

Get the number of frames for a spinner style.

```php
int tui_spinner_frame_count(int $style = TUI_SPINNER_DOTS)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$style` | int | Spinner style constant |

### Return Value

Returns the number of frames in the spinner animation.

### Example

```php
$count = tui_spinner_frame_count(TUI_SPINNER_DOTS);
// 10 frames

$count = tui_spinner_frame_count(TUI_SPINNER_LINE);
// 4 frames
```

---

## tui_render_spinner

Render a spinner to a buffer.

```php
void tui_render_spinner(resource $buffer, int $x, int $y, int $frame, int $style, array $text_style = [])
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$buffer` | resource | Target buffer |
| `$x` | int | X position |
| `$y` | int | Y position |
| `$frame` | int | Current animation frame |
| `$style` | int | Spinner style constant |
| `$text_style` | array | Optional text style array |

### Return Value

None.

### Example

```php
$frame = 0;
while ($loading) {
    tui_buffer_clear($buf);
    tui_render_spinner($buf, 5, 5, $frame++, TUI_SPINNER_DOTS, $style);
    tui_buffer_render($buf);
    usleep(80000);
}
```

---

## Complete Example

```php
<?php
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);
$style = ['fg' => [100, 200, 255]];

// Determinate progress
for ($i = 0; $i <= 100; $i += 2) {
    tui_buffer_clear($buf);

    $progress = $i / 100;
    tui_render_progress_bar($buf, 5, 5, $progress, 50, $style);

    $label = sprintf(" %3d%% Downloading...", $i);
    tui_buffer_write($buf, 56, 5, $label, $style);

    tui_buffer_render($buf);
    usleep(50000);
}

// Indeterminate (extracting)
$frame = 0;
for ($i = 0; $i < 100; $i++) {
    tui_buffer_clear($buf);

    tui_render_busy_bar($buf, 5, 5, $frame, 50, $style);
    tui_render_spinner($buf, 5, 7, $frame, TUI_SPINNER_DOTS, $style);
    tui_buffer_write($buf, 8, 7, "Extracting files...", $style);

    $frame++;
    tui_buffer_render($buf);
    usleep(50000);
}

// Buffer cleanup handled by PHP
```

---

[Back to Documentation](../README.md) | [Feature Guide](../features/progress.md)
