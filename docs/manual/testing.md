# Testing TUI Components

This guide explains how to test TUI components using the headless testing framework provided by ext-tui.

## Overview

The testing framework provides a headless renderer that operates entirely in memory, without any terminal I/O. This allows you to:

- Render components without a terminal
- Inspect the rendered output as strings
- Simulate user input (keyboard)
- Advance time for timer-based tests
- Query nodes by ID or text content

## Getting Started

### Creating a Test Renderer

```php
// Create a headless renderer with 80x24 character buffer
$renderer = tui_test_create(80, 24);

// Don't forget to clean up
tui_test_destroy($renderer);
```

### Basic Rendering Test

```php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

$renderer = tui_test_create(80, 24);

// Create and render a component
$box = new Box(['width' => 80, 'height' => 24]);
$box->children = [new Text("Hello World")];
tui_test_render($renderer, $box);

// Get the output as a string
$output = tui_test_to_string($renderer);
assert(str_contains($output, "Hello World"));

tui_test_destroy($renderer);
```

## Inspecting Output

### As a Single String

```php
$output = tui_test_to_string($renderer);
echo $output;
```

Trailing whitespace and empty lines are automatically trimmed.

### As an Array of Lines

```php
$lines = tui_test_get_output($renderer);
foreach ($lines as $i => $line) {
    echo "Line $i: '$line'\n";
}
```

Each line is a string with trailing spaces trimmed.

## Querying Nodes

### Find by ID

Use IDs to locate specific components:

```php
$box = new Box(['id' => 'submit-button', 'focusable' => true]);
$box->children = [new Text("Submit")];
tui_test_render($renderer, $box);

$node = tui_test_get_by_id($renderer, 'submit-button');
if ($node) {
    echo "Button type: {$node['type']}\n";          // 'box'
    echo "Position: ({$node['x']}, {$node['y']})\n";
    echo "Size: {$node['width']}x{$node['height']}\n";
    echo "Focusable: " . ($node['focusable'] ? 'yes' : 'no') . "\n";
    echo "Focused: " . ($node['focused'] ? 'yes' : 'no') . "\n";
}
```

### Find by Text Content

Search for nodes containing specific text:

```php
$box = new Box();
$box->children = [
    new Text("Welcome to our app"),
    new Text("Click here to continue"),
];
tui_test_render($renderer, $box);

// Find all nodes containing "Click"
$nodes = tui_test_get_by_text($renderer, "Click");
echo "Found " . count($nodes) . " matching nodes\n";

// Check first match
if (count($nodes) > 0) {
    echo "Text: {$nodes[0]['text']}\n";
}
```

## Simulating Input

### Text Input

```php
// Type some text
tui_test_send_input($renderer, "Hello");

// Process the input
tui_test_advance_frame($renderer);
```

### Special Keys

```php
// Send Enter key
tui_test_send_key($renderer, TUI_KEY_ENTER);
tui_test_advance_frame($renderer);

// Send arrow keys
tui_test_send_key($renderer, TUI_KEY_DOWN);
tui_test_send_key($renderer, TUI_KEY_DOWN);
tui_test_advance_frame($renderer);

// Send Tab to move focus
tui_test_send_key($renderer, TUI_KEY_TAB);
tui_test_advance_frame($renderer);
```

### Multiple Inputs

You can queue multiple inputs before advancing:

```php
// Type text and press Enter
tui_test_send_input($renderer, "username");
tui_test_send_key($renderer, TUI_KEY_TAB);
tui_test_send_input($renderer, "password");
tui_test_send_key($renderer, TUI_KEY_ENTER);

// Process all at once
tui_test_advance_frame($renderer);
```

### Ctrl+Key Combinations

Use values 1-26 for Ctrl+key:

```php
// Ctrl+C
tui_test_send_key($renderer, 3);

// Ctrl+D
tui_test_send_key($renderer, 4);
```

## Testing Timers

Simulate the passage of time for timer-based components:

```php
// Create a component with a timer
// (assuming it updates every 1000ms)

// Advance 500ms - timer hasn't fired yet
tui_test_run_timers($renderer, 500);
$output1 = tui_test_to_string($renderer);

// Advance another 600ms - timer has now fired
tui_test_run_timers($renderer, 600);
$output2 = tui_test_to_string($renderer);

// Check that output changed
assert($output1 !== $output2);
```

## Complete Example: Testing a Counter

```php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

function testCounter() {
    $renderer = tui_test_create(40, 10);

    // Initial render with count 0
    $counter = new Box(['id' => 'counter']);
    $counter->children = [new Text("Count: 0")];
    tui_test_render($renderer, $counter);

    // Verify initial state
    $output = tui_test_to_string($renderer);
    assert(str_contains($output, "Count: 0"), "Initial count should be 0");

    // Find the counter node
    $node = tui_test_get_by_id($renderer, 'counter');
    assert($node !== null, "Counter node should exist");
    assert($node['type'] === 'box', "Counter should be a box");

    // Find by text
    $textNodes = tui_test_get_by_text($renderer, "Count:");
    assert(count($textNodes) === 1, "Should find one text node with 'Count:'");

    // Simulate increment (re-render with new state)
    $counter->children = [new Text("Count: 1")];
    tui_test_render($renderer, $counter);

    $output = tui_test_to_string($renderer);
    assert(str_contains($output, "Count: 1"), "Count should be 1 after increment");

    tui_test_destroy($renderer);
    echo "All counter tests passed!\n";
}

testCounter();
```

## Complete Example: Testing Focus Navigation

```php
function testFocusNavigation() {
    $renderer = tui_test_create(40, 10);

    // Create a form with focusable elements
    $form = new Box(['flexDirection' => 'column']);
    $form->children = [
        $input1 = new Box(['id' => 'username', 'focusable' => true]),
        $input2 = new Box(['id' => 'password', 'focusable' => true]),
        $submit = new Box(['id' => 'submit', 'focusable' => true]),
    ];
    $input1->children = [new Text("[Username]")];
    $input2->children = [new Text("[Password]")];
    $submit->children = [new Text("[Submit]")];

    tui_test_render($renderer, $form);

    // Initially, first element should be focused
    $username = tui_test_get_by_id($renderer, 'username');
    // Note: Focus state depends on app handling

    // Press Tab to move to next field
    tui_test_send_key($renderer, TUI_KEY_TAB);
    tui_test_advance_frame($renderer);

    // Press Tab again to move to submit
    tui_test_send_key($renderer, TUI_KEY_TAB);
    tui_test_advance_frame($renderer);

    // Press Enter to submit
    tui_test_send_key($renderer, TUI_KEY_ENTER);
    tui_test_advance_frame($renderer);

    tui_test_destroy($renderer);
    echo "Focus navigation tests passed!\n";
}

testFocusNavigation();
```

## Best Practices

### 1. Use Descriptive IDs

Give meaningful IDs to components you need to query:

```php
$box = new Box(['id' => 'user-profile-card']);
```

### 2. Clean Up Resources

Always destroy renderers when done:

```php
try {
    $renderer = tui_test_create(80, 24);
    // ... tests ...
} finally {
    tui_test_destroy($renderer);
}
```

### 3. Advance Frames After Input

Input is queued, not processed immediately:

```php
tui_test_send_input($renderer, "test");
// Output hasn't changed yet!

tui_test_advance_frame($renderer);
// Now input has been processed
```

### 4. Test One Thing at a Time

Write focused tests for specific behaviors:

```php
function testButtonDisabledState() { /* ... */ }
function testButtonClickHandler() { /* ... */ }
function testButtonKeyboardShortcut() { /* ... */ }
```

### 5. Use Helper Functions

Create reusable test utilities:

```php
function assertContains($renderer, $text, $message = '') {
    $output = tui_test_to_string($renderer);
    assert(str_contains($output, $text), $message ?: "Output should contain '$text'");
}

function assertNodeExists($renderer, $id, $message = '') {
    $node = tui_test_get_by_id($renderer, $id);
    assert($node !== null, $message ?: "Node '$id' should exist");
    return $node;
}
```

## Limitations

- No mouse input simulation (keyboard only)
- No color/style inspection (text content only)
- Timer simulation requires proper app setup
- Maximum buffer size: 1000x1000 characters

## See Also

- [Testing Reference](../reference/testing.md) - Complete API reference
- [Components](components.md) - Building components
- [Input Handling](input.md) - Keyboard input
