# Input Handling

Handle keyboard input, mouse events, focus navigation, and terminal events.

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

---

## Mouse Events

Enable mouse tracking to receive click, scroll, and drag events.

### Enabling Mouse Support

```php
// Enable mouse with different tracking modes
tui_mouse_enable($instance, TUI_MOUSE_MODE_CLICK);   // Basic clicks
tui_mouse_enable($instance, TUI_MOUSE_MODE_BUTTON);  // Press/release events
tui_mouse_enable($instance, TUI_MOUSE_MODE_ALL);     // All events including hover

// Disable mouse tracking
tui_mouse_disable($instance);
```

### Mouse Handler

```php
use Xocdr\Tui\Ext\MouseEvent;

tui_set_mouse_handler($instance, function (MouseEvent $event) use ($instance) {
    // Click detection
    if ($event->action === 'press' && $event->button === 'left') {
        echo "Clicked at ({$event->x}, {$event->y})\n";
    }

    // Scroll handling
    if ($event->button === 'scroll_up') {
        scrollUp();
        tui_rerender($instance);
    }
    if ($event->button === 'scroll_down') {
        scrollDown();
        tui_rerender($instance);
    }

    // Drag detection
    if ($event->action === 'drag') {
        handleDrag($event->x, $event->y);
    }
});
```

### The MouseEvent Object

| Property | Type | Description |
|----------|------|-------------|
| `x` | int | Column (0-based) |
| `y` | int | Row (0-based) |
| `button` | string | `'left'`, `'middle'`, `'right'`, `'scroll_up'`, `'scroll_down'`, `'scroll_left'`, `'scroll_right'`, `'none'` |
| `action` | string | `'press'`, `'release'`, `'move'`, `'drag'` |
| `ctrl` | bool | Ctrl modifier held |
| `meta` | bool | Meta/Alt modifier held |
| `shift` | bool | Shift modifier held |

### Mouse Tracking Modes

| Constant | Description |
|----------|-------------|
| `TUI_MOUSE_MODE_OFF` | Disable mouse tracking |
| `TUI_MOUSE_MODE_CLICK` | Basic click events (mode 1000) |
| `TUI_MOUSE_MODE_BUTTON` | Button press/release events (mode 1002) |
| `TUI_MOUSE_MODE_ALL` | All motion including hover (mode 1003) |

### Hit Testing

Find which node was clicked:

```php
tui_set_mouse_handler($instance, function (MouseEvent $event) use ($instance) {
    if ($event->action === 'press') {
        // Get the deepest node at click position
        $node = tui_hit_test($instance, $event->x, $event->y);
        if ($node && $node['id'] === 'submit-button') {
            handleSubmit();
        }

        // Or get all nodes at position (for event bubbling)
        $nodes = tui_hit_test_all($instance, $event->x, $event->y);
        foreach ($nodes as $node) {
            if (handleClick($node)) break;  // Stop if handled
        }
    }
});
```

---

## Bracketed Paste

Bracketed paste mode allows detecting when text is pasted vs typed, preventing pasted content from being interpreted as commands.

```php
// Enable bracketed paste mode
tui_bracketed_paste_enable($instance);

tui_set_paste_handler($instance, function (string $text) use ($instance, &$buffer) {
    // Handle pasted text - insert directly without interpretation
    $buffer .= $text;
    tui_rerender($instance);
});

// Disable when done
tui_bracketed_paste_disable($instance);
```

---

## Input History

Manage input history for text fields with up/down arrow navigation:

```php
// Create history with max 100 entries
$history = tui_history_create(100);

tui_set_input_handler($instance, function (Key $key) use ($instance, $history, &$input) {
    if ($key->upArrow) {
        // Save current input before navigating
        tui_history_save_temp($history, $input);
        $prev = tui_history_prev($history);
        if ($prev !== null) {
            $input = $prev;
            tui_rerender($instance);
        }
    }

    if ($key->downArrow) {
        $next = tui_history_next($history);
        if ($next !== null) {
            $input = $next;
        } else {
            // Restore original input
            $input = tui_history_get_temp($history) ?? '';
        }
        tui_rerender($instance);
    }

    if ($key->return && $input !== '') {
        // Add to history on submit
        tui_history_add($history, $input);
        processInput($input);
        $input = '';
        tui_rerender($instance);
    }
});

// Clean up
tui_history_destroy($history);
```

---

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

### Focus Groups

Group focusable elements for scoped navigation:

```php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

$app = function () {
    return new Box([
        'flexDirection' => 'column',
        'children' => [
            // Toolbar group
            new Box([
                'flexDirection' => 'row',
                'children' => [
                    new Box(['focusable' => true, 'focusGroup' => 'toolbar', 'tabIndex' => 1]),
                    new Box(['focusable' => true, 'focusGroup' => 'toolbar', 'tabIndex' => 2]),
                ],
            ]),
            // Content group
            new Box([
                'flexDirection' => 'row',
                'children' => [
                    new Box(['focusable' => true, 'focusGroup' => 'content', 'tabIndex' => 1]),
                    new Box(['focusable' => true, 'focusGroup' => 'content', 'tabIndex' => 2]),
                ],
            ]),
        ],
    ]);
};

// Navigate within a specific group
tui_focus_next_in_group($instance, 'toolbar');
```

### Tab Index

Control focus order with `tabIndex`:

```php
new Box([
    'focusable' => true,
    'tabIndex' => 1,     // Focus order (lower = first)
])

new Box([
    'focusable' => true,
    'tabIndex' => -1,    // Skip in tab order (only focusable programmatically)
])
```

### Auto Focus

Automatically focus an element on mount:

```php
new Box([
    'focusable' => true,
    'autoFocus' => true,  // Receives focus when rendered
    'id' => 'main-input',
])
```

### Focus Trap

Trap focus within a container (useful for modals/dialogs):

```php
use Xocdr\Tui\Ext\Box;

// Modal with focus trap
$modal = new Box([
    'focusTrap' => true,  // Tab/Shift+Tab cycles only within this container
    'borderStyle' => 'double',
    'children' => [
        new Box(['focusable' => true, 'children' => [new Text(['content' => 'OK'])]]),
        new Box(['focusable' => true, 'children' => [new Text(['content' => 'Cancel'])]]),
    ],
]);
```

When `focusTrap` is enabled:
- Tab cycles through focusable children only
- Focus cannot escape to elements outside the trap
- Useful for modal dialogs, dropdown menus, etc.

### Focus by ID

Focus a specific element programmatically:

```php
// Element with ID
new Box([
    'focusable' => true,
    'id' => 'username-input',
])

// Focus it
tui_focus_by_id($instance, 'username-input');
```

---

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

---

## Clipboard (OSC 52)

Read from and write to the system clipboard using OSC 52 escape sequences. This works over SSH and in terminals that support OSC 52.

### Writing to Clipboard

```php
// Copy text to clipboard
tui_clipboard_copy('Hello, clipboard!');

// Copy to specific target (X11)
tui_clipboard_copy('Selected text', TUI_CLIPBOARD_PRIMARY);    // Primary selection
tui_clipboard_copy('Copied text', TUI_CLIPBOARD_CLIPBOARD);    // System clipboard (default)
tui_clipboard_copy('Secondary', TUI_CLIPBOARD_SECONDARY);      // Secondary selection
```

### Reading from Clipboard

```php
// Request clipboard contents (async - response comes via handler)
tui_clipboard_request();

tui_set_clipboard_handler($instance, function (string $content) {
    // Handle received clipboard content
    $buffer .= $content;
    tui_rerender($instance);
});
```

### Clearing Clipboard

```php
tui_clipboard_clear();
tui_clipboard_clear(TUI_CLIPBOARD_PRIMARY);
```

### Clipboard Targets

| Constant | Description |
|----------|-------------|
| `TUI_CLIPBOARD_CLIPBOARD` | System clipboard (default, 'c') |
| `TUI_CLIPBOARD_PRIMARY` | Primary selection (X11, 'p') |
| `TUI_CLIPBOARD_SECONDARY` | Secondary selection (X11, 's') |

**Note:** OSC 52 support varies by terminal. Most modern terminals support it, but some may require explicit enabling in settings.

---

## Notes

### Key Detection Limitations

- Some key combinations may be intercepted by the terminal or OS
- Ctrl+C typically exits (unless `exitOnCtrlC: false`)
- Some terminals don't send full modifier info

### Mouse Limitations

- Requires terminal support for SGR mouse protocol (mode 1006)
- Coordinates may be limited in older terminals
- Some terminal emulators intercept certain mouse events

### Terminal Requirements

- Requires interactive TTY (`tui_is_interactive()`)
- Uses raw mode for immediate key reading
- Escape sequences may have slight delay

## See Also

- [Components](components.md) - focusable property
- [Reference: Functions](../reference/functions.md) - Input functions
- [Reference: Classes](../reference/classes.md) - Key, FocusEvent, MouseEvent
