# Input Handling

Handle keyboard input, focus navigation, and terminal events.

## Keyboard Input

Register an input handler to respond to key presses:

```php
use Xocdr\Tui\Ext\Key;

$instance = tui_render($app);

tui_set_input_handler($instance, function (Key $key) use ($instance) {
    // Handle keyboard input
    if ($key->escape) {
        tui_unmount($instance);
        return;
    }

    if ($key->key === 'q') {
        tui_unmount($instance);
    }
});

tui_wait_until_exit($instance);
```

## The Key Object

Input handlers receive a `Xocdr\Tui\Ext\Key` object with these properties:

### Character Keys

```php
$key->key   // The pressed character (e.g., 'a', 'Z', '1', '@')
```

### Named Keys

```php
$key->upArrow      // Up arrow
$key->downArrow    // Down arrow
$key->leftArrow    // Left arrow
$key->rightArrow   // Right arrow
$key->return       // Enter/Return
$key->escape       // Escape
$key->backspace    // Backspace
$key->delete       // Delete
$key->tab          // Tab
```

You can also check the `name` property:

```php
$key->name   // 'up', 'down', 'left', 'right', 'return', 'escape', etc.
```

### Modifier Keys

```php
$key->ctrl    // Ctrl modifier held
$key->alt     // Alt modifier held
$key->meta    // Meta modifier (same as alt on most systems)
$key->shift   // Shift modifier held
```

## Common Input Patterns

### Menu Navigation

```php
use Xocdr\Tui\Ext\Key;

$items = ['New', 'Open', 'Save', 'Quit'];
$selected = 0;

tui_set_input_handler($instance, function (Key $key) use ($instance, &$selected, $items) {
    if ($key->escape) {
        tui_unmount($instance);
        return;
    }

    if ($key->upArrow) {
        $selected = max(0, $selected - 1);
        tui_rerender($instance);
    }

    if ($key->downArrow) {
        $selected = min(count($items) - 1, $selected + 1);
        tui_rerender($instance);
    }

    if ($key->return) {
        // Handle selection
        handleSelection($items[$selected]);
    }
});
```

### Keyboard Shortcuts

```php
use Xocdr\Tui\Ext\Key;

tui_set_input_handler($instance, function (Key $key) use ($instance) {
    // Ctrl+S to save
    if ($key->ctrl && $key->key === 's') {
        save();
        return;
    }

    // Ctrl+Q to quit
    if ($key->ctrl && $key->key === 'q') {
        tui_unmount($instance);
        return;
    }

    // Ctrl+Z to undo
    if ($key->ctrl && $key->key === 'z') {
        undo();
        tui_rerender($instance);
        return;
    }
});
```

### Text Input

```php
use Xocdr\Tui\Ext\Key;

$text = '';

tui_set_input_handler($instance, function (Key $key) use ($instance, &$text) {
    if ($key->escape) {
        tui_unmount($instance);
        return;
    }

    // Backspace - delete last character
    if ($key->backspace && strlen($text) > 0) {
        $text = substr($text, 0, -1);
        tui_rerender($instance);
        return;
    }

    // Printable character
    if ($key->key !== '' && !$key->ctrl && !$key->alt) {
        $text .= $key->key;
        tui_rerender($instance);
    }
});
```

## Focus Navigation

Make elements focusable and navigate between them:

```php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;
use Xocdr\Tui\Ext\Key;

$app = function () {
    return new Box([
        'flexDirection' => 'column',
        'gap' => 1,
        'children' => [
            new Box([
                'focusable' => true,
                'borderStyle' => 'single',
                'children' => [new Text(['content' => 'Option 1'])],
            ]),
            new Box([
                'focusable' => true,
                'borderStyle' => 'single',
                'children' => [new Text(['content' => 'Option 2'])],
            ]),
        ],
    ]);
};

$instance = tui_render($app);

tui_set_input_handler($instance, function (Key $key) use ($instance) {
    if ($key->tab) {
        if ($key->shift) {
            tui_focus_prev($instance);
        } else {
            tui_focus_next($instance);
        }
        tui_rerender($instance);
    }
});
```

### Focus Handlers

Respond to focus changes:

```php
use Xocdr\Tui\Ext\FocusEvent;

tui_set_focus_handler($instance, function (FocusEvent $event) {
    if ($event->current) {
        // Something gained focus
        echo "Focused: {$event->direction}\n";
    }
});
```

### Getting Focused Node

```php
$focused = tui_get_focused_node($instance);
if ($focused) {
    echo "Focused at ({$focused['x']}, {$focused['y']})\n";
}
```

## Terminal Resize

Handle terminal resize events:

```php
tui_set_resize_handler($instance, function (int $width, int $height) use ($instance) {
    // Terminal resized
    tui_rerender($instance);
});
```

You can also query the current size:

```php
[$width, $height] = tui_get_terminal_size();

// Or from instance
$size = tui_get_size($instance);
echo "Size: {$size['width']}x{$size['height']}\n";
```

## Timers

Execute callbacks at intervals:

```php
// Add a timer (100ms interval)
$timerId = tui_add_timer($instance, function () use ($instance, &$frame) {
    $frame++;
    tui_rerender($instance);
}, 100);

// Remove timer when done
tui_remove_timer($instance, $timerId);
```

### Tick Handler

For per-frame updates:

```php
tui_set_tick_handler($instance, function () use ($instance, &$time) {
    $time++;
    tui_rerender($instance);
});
```

## Notes

### Key Detection Limitations

- Some key combinations may be intercepted by the terminal or OS
- Ctrl+C typically exits (unless `exitOnCtrlC: false`)
- Some terminals don't send full modifier info

### Terminal Requirements

- Requires interactive TTY (`tui_is_interactive()`)
- Uses raw mode for immediate key reading
- Escape sequences may have slight delay

## See Also

- [Components](components.md) - focusable property
- [Reference: Functions](../reference/functions.md) - Input functions
- [Reference: Classes](../reference/classes.md) - Key, FocusEvent
