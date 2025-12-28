# Input Handling Guide

ext-tui provides keyboard input handling through the event loop.

## Keyboard Events

When a key is pressed, the input handler receives a `TuiKey` object with information about the keypress.

### TuiKey Properties

```php
$key->key           // string - The character pressed (e.g., "a", "1", " ")
$key->upArrow       // bool - Up arrow key
$key->downArrow     // bool - Down arrow key
$key->leftArrow     // bool - Left arrow key
$key->rightArrow    // bool - Right arrow key
$key->return        // bool - Enter/Return key
$key->escape        // bool - Escape key
$key->backspace     // bool - Backspace key
$key->delete        // bool - Delete key
$key->tab           // bool - Tab key
$key->ctrl          // bool - Ctrl modifier held
$key->meta          // bool - Alt/Meta modifier held
$key->shift         // bool - Shift modifier held
```

## Common Patterns

### Arrow Key Navigation

```php
function handleInput(TuiKey $key) use (&$selectedIndex, &$items) {
    if ($key->upArrow && $selectedIndex > 0) {
        $selectedIndex--;
    }
    if ($key->downArrow && $selectedIndex < count($items) - 1) {
        $selectedIndex++;
    }
}
```

### Character Input

```php
function handleInput(TuiKey $key) use (&$text) {
    if ($key->backspace && strlen($text) > 0) {
        $text = substr($text, 0, -1);
    } elseif ($key->key && !$key->ctrl && !$key->meta) {
        $text .= $key->key;
    }
}
```

### Ctrl+Key Combinations

```php
function handleInput(TuiKey $key) use ($instance) {
    if ($key->ctrl) {
        switch ($key->key) {
            case 's':
                save();
                break;
            case 'q':
                $instance->exit();
                break;
        }
    }
}
```

### Escape Handling

```php
function handleInput(TuiKey $key) use (&$mode) {
    if ($key->escape) {
        $mode = 'normal';
    }
}
```

## Auto Exit on Ctrl+C

By default, Ctrl+C exits the application. Disable this to handle it yourself:

```php
$instance = tui_render($component, [
    'exitOnCtrlC' => false
]);
```

Then handle Ctrl+C in your input handler:

```php
function handleInput(TuiKey $key) use ($instance, &$hasUnsavedChanges) {
    if ($key->ctrl && $key->key === 'c') {
        if ($hasUnsavedChanges) {
            // Show confirmation dialog
        } else {
            $instance->exit();
        }
    }
}
```

## Event Loop

The `waitUntilExit()` method runs the event loop, processing input:

```php
$instance = tui_render($component);
$instance->waitUntilExit();  // Blocks here, processing input
// After exit...
```

## Triggering Re-renders

After handling input that changes state, trigger a re-render:

```php
function handleInput(TuiKey $key) use ($instance, &$state) {
    if ($key->upArrow) {
        $state['index']--;
        tui_rerender($instance);
    }
}
```

Or use the instance method:

```php
$instance->rerender();
```
