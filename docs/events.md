# ext-tui Event System

This document describes the event system in ext-tui and how to handle user interactions and system events.

## Overview

ext-tui provides three types of events:

1. **Input Events** - Keyboard input from the user
2. **Focus Events** - Focus changes between focusable elements
3. **Resize Events** - Terminal window resize notifications

All events use a callback-based system. You register handlers using the `tui_set_*_handler()` functions.

## Input Events

Input events are triggered when the user presses keys on the keyboard.

### Registering an Input Handler

```php
$instance = tui_render(function() {
    return new TuiBox(['padding' => 1]);
});

tui_set_input_handler($instance, function(TuiKey $key) {
    // Handle input
});

tui_wait_until_exit($instance);
```

### The TuiKey Object

The input handler receives a `TuiKey` object with these properties:

| Property | Type | Description |
|----------|------|-------------|
| `key` | string | The character pressed (for printable keys) |
| `upArrow` | bool | Up arrow key pressed |
| `downArrow` | bool | Down arrow key pressed |
| `leftArrow` | bool | Left arrow key pressed |
| `rightArrow` | bool | Right arrow key pressed |
| `return` | bool | Enter/Return key pressed |
| `escape` | bool | Escape key pressed |
| `backspace` | bool | Backspace key pressed |
| `delete` | bool | Delete key pressed |
| `tab` | bool | Tab key pressed |
| `ctrl` | bool | Ctrl modifier held |
| `meta` | bool | Alt/Option modifier held |
| `shift` | bool | Shift modifier held |

### Input Examples

```php
tui_set_input_handler($instance, function(TuiKey $key) use ($instance) {
    // Check for specific characters
    if ($key->key === 'q') {
        $instance->exit(0);
        return;
    }

    // Check for arrow keys
    if ($key->upArrow) {
        moveUp();
    } elseif ($key->downArrow) {
        moveDown();
    }

    // Check for modifiers
    if ($key->ctrl && $key->key === 's') {
        save();
    }

    // Shift+Tab is detected automatically
    if ($key->tab && $key->shift) {
        // Focus previous (also handled by built-in Tab navigation)
    }

    // Escape to exit
    if ($key->escape) {
        $instance->exit(0);
    }
});
```

### Modifier Keys

The following modifier combinations are detected:

| Sequence | Detected Modifiers |
|----------|-------------------|
| `Ctrl+<key>` | `ctrl = true` |
| `Alt+<key>` (or Option on macOS) | `meta = true` |
| `Shift+<arrow>` | `shift = true` |
| `Shift+Tab` | `tab = true, shift = true` |
| `Ctrl+Shift+<key>` | `ctrl = true, shift = true` |

### Built-in Key Handling

Some keys have built-in behavior:

- **Ctrl+C**: Exits the application (if `exitOnCtrlC` option is true)
- **Tab**: Moves focus to the next focusable element
- **Shift+Tab**: Moves focus to the previous focusable element

You can still receive these events in your input handler; they are processed *after* the built-in behavior.

## Focus Events

Focus events are triggered when focus moves between focusable elements.

### Making Elements Focusable

```php
$button = new TuiBox([
    'focusable' => true,
    'borderStyle' => 'single',
    'padding' => 1,
]);
```

### Registering a Focus Handler

```php
tui_set_focus_handler($instance, function(?array $oldNode, ?array $newNode) {
    if ($oldNode) {
        echo "Lost focus\n";
    }
    if ($newNode) {
        echo "Gained focus\n";
    }
});
```

### Focus Event Data

Both `$oldNode` and `$newNode` are arrays (or null) containing:

```php
[
    'focusable' => bool,  // Always true for focusable nodes
    'focused' => bool,    // Current focus state
]
```

### Programmatic Focus Control

```php
// Move focus to next focusable element
tui_focus_next($instance);

// Move focus to previous focusable element
tui_focus_prev($instance);
```

### Focus Navigation

Focus navigation is automatic:

- **Tab**: Focus next element
- **Shift+Tab**: Focus previous element

Focus cycles: after the last element, focus returns to the first.

### Visual Focus Indication

You can style focused elements differently:

```php
function renderButton($label, $focused) {
    return new TuiBox([
        'focusable' => true,
        'focused' => $focused,
        'borderStyle' => $focused ? 'double' : 'single',
        'borderColor' => $focused ? '#00ff00' : '#ffffff',
        'padding' => 1,
        'children' => [new TuiText($label)],
    ]);
}
```

## Resize Events

Resize events are triggered when the terminal window changes size.

### Registering a Resize Handler

```php
tui_set_resize_handler($instance, function(int $width, int $height) {
    echo "Terminal is now {$width}x{$height}\n";
});
```

### Automatic Handling

When the terminal resizes, ext-tui automatically:

1. Updates internal buffers
2. Recalculates layout
3. Re-renders the component tree
4. Calls your resize handler (if registered)

Your handler is called *after* the automatic resize handling.

### Getting Current Size

```php
[$width, $height] = tui_get_terminal_size();
```

## Event Flow

Events are processed in this order:

1. **Raw input received** from terminal
2. **Input parsed** into TuiKey object
3. **Built-in handlers** run (Ctrl+C, Tab navigation)
4. **Your input handler** is called
5. **Re-render** is triggered if needed

For resize:

1. **SIGWINCH signal** received
2. **Buffers resized**
3. **Layout recalculated**
4. **Your resize handler** is called
5. **Screen re-rendered**

## Best Practices

### 1. Keep Handlers Fast

Event handlers should complete quickly. Long-running operations should be deferred:

```php
// Good: Quick check and flag
tui_set_input_handler($instance, function(TuiKey $key) use (&$shouldSave) {
    if ($key->ctrl && $key->key === 's') {
        $shouldSave = true;
    }
});

// Do heavy work in main loop
while (!$shouldExit) {
    if ($shouldSave) {
        save();
        $shouldSave = false;
    }
}
```

### 2. Use State for Complex UIs

```php
$state = ['selectedIndex' => 0, 'items' => ['A', 'B', 'C']];

tui_set_input_handler($instance, function(TuiKey $key) use (&$state, $instance) {
    if ($key->upArrow && $state['selectedIndex'] > 0) {
        $state['selectedIndex']--;
        tui_rerender($instance);
    }
    if ($key->downArrow && $state['selectedIndex'] < count($state['items']) - 1) {
        $state['selectedIndex']++;
        tui_rerender($instance);
    }
});
```

### 3. Clean Exit Handling

```php
$instance = tui_render($component);

tui_set_input_handler($instance, function(TuiKey $key) use ($instance) {
    if ($key->escape || $key->key === 'q') {
        $instance->exit(0);
    }
});

tui_wait_until_exit($instance);
// Terminal is restored automatically
```

## Complete Example

```php
<?php

$state = [
    'items' => ['Option 1', 'Option 2', 'Option 3'],
    'selected' => 0,
];

$component = function() use (&$state) {
    $box = new TuiBox([
        'flexDirection' => 'column',
        'padding' => 1,
    ]);

    foreach ($state['items'] as $i => $item) {
        $isSelected = $i === $state['selected'];
        $box->addChild(new TuiBox([
            'focusable' => true,
            'focused' => $isSelected,
            'borderStyle' => $isSelected ? 'double' : 'single',
            'paddingX' => 2,
            'children' => [
                new TuiText($item, [
                    'bold' => $isSelected,
                    'color' => $isSelected ? '#00ff00' : '#ffffff',
                ]),
            ],
        ]));
    }

    return $box;
};

$instance = tui_render($component);

tui_set_input_handler($instance, function(TuiKey $key) use (&$state, $instance) {
    $changed = false;

    if ($key->upArrow && $state['selected'] > 0) {
        $state['selected']--;
        $changed = true;
    }
    if ($key->downArrow && $state['selected'] < count($state['items']) - 1) {
        $state['selected']++;
        $changed = true;
    }
    if ($key->return) {
        echo "Selected: " . $state['items'][$state['selected']] . "\n";
        $instance->exit(0);
        return;
    }
    if ($key->escape) {
        $instance->exit(1);
        return;
    }

    if ($changed) {
        tui_rerender($instance);
    }
});

tui_set_focus_handler($instance, function($old, $new) {
    // Focus changed - update UI if needed
});

tui_set_resize_handler($instance, function($w, $h) use ($instance) {
    // Layout will auto-adjust, but we can do custom handling here
});

tui_wait_until_exit($instance);
```
