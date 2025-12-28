# Event Handling

[← Sprites](sprites.md) | [Back to Documentation](../README.md)

---

Event handling provides keyboard input and terminal resize detection for interactive applications.

## Overview

The event system supports:
- **Keyboard input**: Regular keys, special keys, and key combinations
- **Terminal resize**: Detect when terminal dimensions change
- **Event loop**: Main loop for interactive applications

## Functions

### tui_on_key()

Register a callback for keyboard input.

```php
void tui_on_key(callable $callback)
```

**Parameters**:
- `$callback`: Function receiving key string

**Callback signature**:
```php
function(string $key): void
```

**Key values**:
- Regular characters: `"a"`, `"A"`, `"1"`, `"@"`, etc.
- Special keys: `"Enter"`, `"Tab"`, `"Escape"`, `"Backspace"`, `"Delete"`
- Arrow keys: `"Up"`, `"Down"`, `"Left"`, `"Right"`
- Function keys: `"F1"` through `"F12"`
- Modifiers: `"Ctrl+c"`, `"Ctrl+x"`, `"Alt+a"`, etc.

**Example**:
```php
tui_on_key(function(string $key) {
    switch ($key) {
        case 'q':
        case 'Escape':
            tui_stop();
            break;
        case 'Up':
            move_cursor_up();
            break;
        case 'Down':
            move_cursor_down();
            break;
        case 'Enter':
            select_item();
            break;
        case 'Ctrl+c':
            handle_interrupt();
            break;
    }
});
```

---

### tui_on_resize()

Register a callback for terminal resize events.

```php
void tui_on_resize(callable $callback)
```

**Parameters**:
- `$callback`: Function receiving new dimensions

**Callback signature**:
```php
function(int $width, int $height): void
```

**Example**:
```php
$buf = null;

tui_on_resize(function(int $width, int $height) {
    global $buf;

    // Recreate buffer with new size
    if ($buf) {
        tui_buffer_free($buf);
    }
    $buf = tui_buffer_create($width, $height);

    // Redraw UI
    redraw();
});
```

---

### tui_run()

Start the event loop.

```php
void tui_run()
```

This enters raw mode, starts listening for events, and blocks until `tui_stop()` is called.

**Example**:
```php
// Set up handlers
tui_on_key(function($key) { /* ... */ });
tui_on_resize(function($w, $h) { /* ... */ });

// Initial render
render();

// Start event loop (blocks)
tui_run();

// Clean up after loop exits
cleanup();
```

---

### tui_stop()

Stop the event loop and exit.

```php
void tui_stop()
```

**Example**:
```php
tui_on_key(function(string $key) {
    if ($key === 'q') {
        tui_stop();
    }
});
```

---

## Practical Patterns

### Basic Interactive Application

```php
<?php
// Check environment
if (!tui_is_interactive()) {
    die("Interactive terminal required\n");
}

// Initialize
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);
$cursor_y = 0;
$items = ['Option 1', 'Option 2', 'Option 3', 'Exit'];

function render(): void
{
    global $buf, $width, $height, $items, $cursor_y;

    tui_buffer_clear($buf);

    // Title
    $title_style = ['fg' => [255, 200, 0], 'bold' => true];
    tui_buffer_write($buf, 2, 1, "Select an option:", $title_style);

    // Menu items
    foreach ($items as $i => $item) {
        $is_selected = ($i === $cursor_y);

        $style = $is_selected
            ? ['fg' => [0, 0, 0], 'bg' => [100, 150, 255]]
            : ['fg' => [200, 200, 200]];

        $prefix = $is_selected ? '> ' : '  ';
        tui_buffer_write($buf, 2, 3 + $i, $prefix . $item, $style);
    }

    // Help
    $help_style = ['fg' => [128, 128, 128]];
    tui_buffer_write($buf, 2, $height - 2, "Use arrow keys to navigate, Enter to select, q to quit", $help_style);

    tui_render($buf);
}

// Handle keyboard
tui_on_key(function(string $key) {
    global $cursor_y, $items;

    switch ($key) {
        case 'Up':
            $cursor_y = max(0, $cursor_y - 1);
            break;
        case 'Down':
            $cursor_y = min(count($items) - 1, $cursor_y + 1);
            break;
        case 'Enter':
            if ($items[$cursor_y] === 'Exit') {
                tui_stop();
            } else {
                // Handle selection
            }
            break;
        case 'q':
        case 'Escape':
            tui_stop();
            return;
    }

    render();
});

// Handle resize
tui_on_resize(function(int $w, int $h) {
    global $buf, $width, $height;

    tui_buffer_free($buf);
    $width = $w;
    $height = $h;
    $buf = tui_buffer_create($w, $h);
    render();
});

// Initial render and start
render();
tui_run();

// Cleanup
tui_buffer_free($buf);
echo "Goodbye!\n";
```

### Text Editor Input

```php
class TextInput
{
    private string $text = '';
    private int $cursor = 0;

    public function handleKey(string $key): bool
    {
        switch ($key) {
            case 'Backspace':
                if ($this->cursor > 0) {
                    $this->text = substr($this->text, 0, $this->cursor - 1) .
                                  substr($this->text, $this->cursor);
                    $this->cursor--;
                }
                break;

            case 'Delete':
                if ($this->cursor < strlen($this->text)) {
                    $this->text = substr($this->text, 0, $this->cursor) .
                                  substr($this->text, $this->cursor + 1);
                }
                break;

            case 'Left':
                $this->cursor = max(0, $this->cursor - 1);
                break;

            case 'Right':
                $this->cursor = min(strlen($this->text), $this->cursor + 1);
                break;

            case 'Home':
                $this->cursor = 0;
                break;

            case 'End':
                $this->cursor = strlen($this->text);
                break;

            case 'Enter':
                return true;  // Submit

            default:
                // Regular character
                if (strlen($key) === 1 && ord($key) >= 32) {
                    $this->text = substr($this->text, 0, $this->cursor) .
                                  $key .
                                  substr($this->text, $this->cursor);
                    $this->cursor++;
                }
        }

        return false;  // Continue editing
    }

    public function render($buf, int $x, int $y, int $width): void
    {
        $style = ['fg' => [200, 200, 200], 'bg' => [40, 40, 50]];
        $cursor_style = ['fg' => [0, 0, 0], 'bg' => [200, 200, 200]];

        // Background
        tui_fill_rect($buf, $x, $y, $width, 1, ' ', $style);

        // Text before cursor
        $before = substr($this->text, 0, $this->cursor);
        tui_buffer_write($buf, $x, $y, $before, $style);

        // Cursor
        $cursor_char = $this->cursor < strlen($this->text)
            ? $this->text[$this->cursor]
            : ' ';
        tui_buffer_write($buf, $x + strlen($before), $y, $cursor_char, $cursor_style);

        // Text after cursor
        if ($this->cursor < strlen($this->text)) {
            $after = substr($this->text, $this->cursor + 1);
            tui_buffer_write($buf, $x + strlen($before) + 1, $y, $after, $style);
        }
    }

    public function getText(): string
    {
        return $this->text;
    }
}
```

### Modal Dialog

```php
class ModalDialog
{
    private string $title;
    private string $message;
    private array $buttons;
    private int $selected = 0;
    private $callback;

    public function __construct(string $title, string $message, array $buttons, callable $callback)
    {
        $this->title = $title;
        $this->message = $message;
        $this->buttons = $buttons;
        $this->callback = $callback;
    }

    public function handleKey(string $key): bool
    {
        switch ($key) {
            case 'Left':
                $this->selected = max(0, $this->selected - 1);
                break;
            case 'Right':
                $this->selected = min(count($this->buttons) - 1, $this->selected + 1);
                break;
            case 'Enter':
                ($this->callback)($this->buttons[$this->selected]);
                return true;  // Close dialog
            case 'Escape':
                ($this->callback)(null);  // Cancelled
                return true;
        }
        return false;
    }

    public function render($buf, int $screen_width, int $screen_height): void
    {
        $width = 40;
        $height = 8;
        $x = (int)(($screen_width - $width) / 2);
        $y = (int)(($screen_height - $height) / 2);

        $box_style = ['fg' => [200, 200, 200], 'bg' => [60, 60, 80]];
        $title_style = ['fg' => [255, 255, 100], 'bold' => true];
        $button_style = ['fg' => [180, 180, 180]];
        $selected_style = ['fg' => [0, 0, 0], 'bg' => [100, 150, 255]];

        // Background
        tui_fill_rect($buf, $x, $y, $width, $height, ' ', $box_style);
        tui_draw_box($buf, $x, $y, $width, $height, $box_style);

        // Title
        tui_buffer_write($buf, $x + 2, $y, " $this->title ", $title_style);

        // Message
        $lines = tui_wrap_text($this->message, $width - 4, TUI_WRAP_WORD);
        foreach ($lines as $i => $line) {
            tui_buffer_write($buf, $x + 2, $y + 2 + $i, $line, $box_style);
        }

        // Buttons
        $button_y = $y + $height - 2;
        $total_width = array_sum(array_map(fn($b) => strlen($b) + 4, $this->buttons));
        $button_x = $x + (int)(($width - $total_width) / 2);

        foreach ($this->buttons as $i => $button) {
            $style = ($i === $this->selected) ? $selected_style : $button_style;
            $text = "[ $button ]";
            tui_buffer_write($buf, $button_x, $button_y, $text, $style);
            $button_x += strlen($text) + 2;
        }
    }
}

// Usage
$dialog = new ModalDialog(
    "Confirm",
    "Are you sure you want to delete this file?",
    ["Yes", "No"],
    function(?string $button) {
        if ($button === "Yes") {
            // Delete file
        }
        // Close dialog
    }
);
```

### Debounced Resize

```php
$resize_timer = null;
$pending_resize = null;

tui_on_resize(function(int $w, int $h) use (&$resize_timer, &$pending_resize) {
    // Store pending resize
    $pending_resize = [$w, $h];

    // Debounce - only handle after resize stops
    if ($resize_timer) {
        // Timer already pending, will use new values
        return;
    }

    // Schedule resize handling
    $resize_timer = true;

    // In a real app, you'd use a proper timer
    // This is simplified for illustration
    register_shutdown_function(function() use (&$pending_resize) {
        if ($pending_resize) {
            [$w, $h] = $pending_resize;
            handle_resize($w, $h);
        }
    });
});

function handle_resize(int $w, int $h): void
{
    global $buf, $width, $height;

    tui_buffer_free($buf);
    $width = $w;
    $height = $h;
    $buf = tui_buffer_create($w, $h);
    redraw();
}
```

## Key Reference

### Printable Characters
```
a-z, A-Z, 0-9
! @ # $ % ^ & * ( ) - = _ + [ ] { } | \ ; : ' " , . < > / ?
Space (as " ")
```

### Special Keys
```
Enter, Tab, Escape, Backspace, Delete
Insert, Home, End, PageUp, PageDown
Up, Down, Left, Right
F1-F12
```

### Modifier Combinations
```
Ctrl+a through Ctrl+z
Alt+a through Alt+z (terminal-dependent)
Shift+Tab (as "Shift+Tab")
```

## Related Functions

- [tui_get_terminal_size()](../api/terminal.md#tui_get_terminal_size) - Get current dimensions
- [tui_is_interactive()](../api/terminal.md#tui_is_interactive) - Check for TTY
- [tui_buffer_create()](../api/buffers.md#tui_buffer_create) - Create render buffer

## See Also

- [Terminal Functions](terminal.md) - Environment detection
- [Buffers](buffers.md) - Screen rendering
- [API Reference: Events](../api/events.md) - Complete API details

---

[← Sprites](sprites.md) | [Back to Documentation](../README.md)
