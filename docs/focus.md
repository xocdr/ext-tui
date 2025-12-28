# ext-tui Focus System

This document describes how focus management works in ext-tui for building interactive terminal UIs.

## Overview

Focus management allows users to navigate between interactive elements using the keyboard. This is essential for:

- Form inputs
- Menu selection
- Button navigation
- List selection

## Making Elements Focusable

Only `TuiBox` elements can be focusable. Set the `focusable` property to `true`:

```php
$button = new TuiBox([
    'focusable' => true,
    'borderStyle' => 'single',
    'padding' => 1,
    'children' => [new TuiText('Click Me')],
]);
```

## Keyboard Navigation

### Built-in Navigation

ext-tui automatically handles focus navigation:

| Key | Action |
|-----|--------|
| **Tab** | Focus next element |
| **Shift+Tab** | Focus previous element |

Focus cycles through elements in tree order (depth-first).

### Programmatic Navigation

```php
// Move to next focusable element
tui_focus_next($instance);

// Move to previous focusable element
tui_focus_prev($instance);
```

## Focus Order

Focus order is determined by the component tree structure:

```php
$root = new TuiBox([
    'children' => [
        new TuiBox(['focusable' => true]),  // Focus order: 1
        new TuiBox([
            'children' => [
                new TuiBox(['focusable' => true]),  // Focus order: 2
                new TuiBox(['focusable' => true]),  // Focus order: 3
            ],
        ]),
        new TuiBox(['focusable' => true]),  // Focus order: 4
    ],
]);
```

Focus traverses depth-first: parent before children, siblings in order.

## Focus State

### Checking Focus State

The `focused` property indicates current focus state:

```php
$box = new TuiBox([
    'focusable' => true,
    'focused' => true,  // This element starts with focus
]);
```

### Focus Change Events

Register a handler to be notified of focus changes:

```php
tui_set_focus_handler($instance, function(?array $oldNode, ?array $newNode) {
    if ($oldNode) {
        // An element lost focus
    }
    if ($newNode) {
        // An element gained focus
    }
});
```

## Visual Focus Indication

Users need visual feedback for which element is focused. Common patterns:

### Border Highlighting

```php
function renderButton($label, $isFocused) {
    return new TuiBox([
        'focusable' => true,
        'borderStyle' => $isFocused ? 'double' : 'single',
        'borderColor' => $isFocused ? '#00ff00' : '#888888',
        'padding' => 1,
        'children' => [new TuiText($label)],
    ]);
}
```

### Color Change

```php
function renderMenuItem($label, $isFocused) {
    return new TuiBox([
        'focusable' => true,
        'children' => [
            new TuiText($label, [
                'color' => $isFocused ? '#ffffff' : '#888888',
                'backgroundColor' => $isFocused ? '#0066cc' : null,
                'bold' => $isFocused,
            ]),
        ],
    ]);
}
```

### Prefix/Indicator

```php
function renderOption($label, $isFocused) {
    $prefix = $isFocused ? '> ' : '  ';
    return new TuiBox([
        'focusable' => true,
        'flexDirection' => 'row',
        'children' => [
            new TuiText($prefix, ['color' => '#00ff00']),
            new TuiText($label),
        ],
    ]);
}
```

## Focus Patterns

### Single Selection List

```php
$state = ['selectedIndex' => 0];

function renderList($items, $selectedIndex) {
    $box = new TuiBox(['flexDirection' => 'column']);

    foreach ($items as $i => $item) {
        $focused = $i === $selectedIndex;
        $box->addChild(new TuiBox([
            'focusable' => true,
            'focused' => $focused,
            'children' => [
                new TuiText(
                    ($focused ? '> ' : '  ') . $item,
                    ['bold' => $focused]
                ),
            ],
        ]));
    }

    return $box;
}

tui_set_input_handler($instance, function(TuiKey $key) use (&$state, $instance, $items) {
    if ($key->upArrow && $state['selectedIndex'] > 0) {
        $state['selectedIndex']--;
        tui_rerender($instance);
    }
    if ($key->downArrow && $state['selectedIndex'] < count($items) - 1) {
        $state['selectedIndex']++;
        tui_rerender($instance);
    }
});
```

### Form with Multiple Inputs

```php
$state = [
    'focusedField' => 0,
    'fields' => ['name', 'email', 'message'],
    'values' => ['', '', ''],
];

function renderForm($state) {
    $box = new TuiBox([
        'flexDirection' => 'column',
        'gap' => 1,
    ]);

    foreach ($state['fields'] as $i => $field) {
        $focused = $i === $state['focusedField'];
        $box->addChild(new TuiBox([
            'flexDirection' => 'row',
            'gap' => 1,
            'children' => [
                new TuiText(ucfirst($field) . ':', ['width' => 10]),
                new TuiBox([
                    'focusable' => true,
                    'focused' => $focused,
                    'borderStyle' => $focused ? 'double' : 'single',
                    'flexGrow' => 1,
                    'children' => [
                        new TuiText($state['values'][$i] ?: '(empty)'),
                    ],
                ]),
            ],
        ]));
    }

    return $box;
}
```

### Button Group

```php
function renderButtons($buttons, $focusedIndex) {
    $box = new TuiBox([
        'flexDirection' => 'row',
        'gap' => 2,
        'justifyContent' => 'center',
    ]);

    foreach ($buttons as $i => $label) {
        $focused = $i === $focusedIndex;
        $box->addChild(new TuiBox([
            'focusable' => true,
            'focused' => $focused,
            'borderStyle' => 'round',
            'borderColor' => $focused ? '#00ff00' : '#666666',
            'paddingX' => 3,
            'paddingY' => 1,
            'children' => [
                new TuiText($label, [
                    'bold' => $focused,
                ]),
            ],
        ]));
    }

    return $box;
}
```

## Focus Events

### Handling Focus Changes

```php
tui_set_focus_handler($instance, function($old, $new) use (&$state, $instance) {
    // Track which element is focused
    if ($new) {
        $state['lastFocused'] = time();
    }

    // Optionally trigger a re-render
    tui_rerender($instance);
});
```

### Preventing Default Focus

Focus navigation (Tab/Shift+Tab) happens before your input handler is called. If you need to override it, you can manage focus state manually:

```php
$state = ['focusIndex' => 0, 'maxIndex' => 4];

tui_set_input_handler($instance, function(TuiKey $key) use (&$state, $instance) {
    // Custom Tab handling (happens after built-in)
    // For complete control, track focus in state instead of using focusable
});
```

## Best Practices

### 1. Always Indicate Focus

Users must always know which element is focused:

```php
// Good - clear visual distinction
$box = new TuiBox([
    'borderStyle' => $focused ? 'double' : 'single',
    'borderColor' => $focused ? '#00ff00' : '#444444',
]);

// Bad - no visual indication
$box = new TuiBox(['focusable' => true]);
```

### 2. Logical Focus Order

Arrange focusable elements in logical reading order:

```php
// Good - tab order matches visual order
$form = new TuiBox([
    'flexDirection' => 'column',
    'children' => [
        $usernameInput,  // Tab order: 1
        $passwordInput,  // Tab order: 2
        $submitButton,   // Tab order: 3
    ],
]);
```

### 3. Keep Focus Count Reasonable

Too many focusable elements is confusing. Group related items:

```php
// Good - focused group, arrow keys within
$menuGroup = new TuiBox([
    'focusable' => true,
    'children' => [...menuItems],  // Use arrow keys to navigate items
]);

// Avoid - too many tab stops
foreach ($menuItems as $item) {
    $box->addChild(new TuiBox(['focusable' => true, ...]));
}
```

### 4. Support Keyboard Shortcuts

For common actions, provide keyboard shortcuts in addition to focus navigation:

```php
tui_set_input_handler($instance, function(TuiKey $key) use ($instance) {
    // Direct shortcuts
    if ($key->key === '1') selectItem(0);
    if ($key->key === '2') selectItem(1);
    if ($key->escape) $instance->exit(0);

    // Arrow navigation within focused component
    if ($key->upArrow) moveUp();
    if ($key->downArrow) moveDown();
});
```

## Accessibility Considerations

### Visual Contrast

Ensure focused elements have sufficient contrast:

```php
// Good contrast
'borderColor' => $focused ? '#00ff00' : '#444444',
'color' => $focused ? '#ffffff' : '#888888',

// Poor contrast - avoid
'borderColor' => $focused ? '#009900' : '#008800',
```

### Consistent Behavior

Focus should behave predictably:
- Tab always moves forward
- Shift+Tab always moves backward
- Focus wraps at boundaries
- Focus order matches visual layout
