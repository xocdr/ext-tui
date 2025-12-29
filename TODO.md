# ext-tui: Component Testing Framework Specification

## Overview

This document specifies a component testing framework for ext-tui TUI applications. The framework enables automated testing of TUI components without requiring an actual terminal, similar to how React Testing Library works for React components.

---

## Feature Requirements

### 1. Headless Rendering

Render TUI components to an in-memory buffer without terminal output.

**PHP API:**
```php
use Xocdr\Tui\Testing\TestRenderer;

$renderer = new TestRenderer(80, 24);  // width, height
$instance = $renderer->render(fn($t) => new Box(['width' => 10], [
    new Text('Hello')
]));

// Get rendered content as string grid
$output = $renderer->getOutput();  // Returns 2D string array
$text = $renderer->toString();     // Returns single string with newlines
```

**C API:**
```c
tui_test_renderer* tui_test_renderer_create(int width, int height);
void tui_test_renderer_render(tui_test_renderer *renderer, tui_node *root);
const char** tui_test_renderer_get_output(tui_test_renderer *renderer);
void tui_test_renderer_destroy(tui_test_renderer *renderer);
```

**Implementation Location:** `src/testing/renderer.c`, `src/testing/renderer.h`

---

### 2. Input Simulation

Simulate keyboard input for testing interactive components.

**PHP API:**
```php
$renderer->sendInput('a');           // Single character
$renderer->sendKey(Key::ENTER);      // Special key
$renderer->sendKey(Key::ARROW_UP);
$renderer->sendKey(Key::CTRL_C);
$renderer->sendSequence('hello');    // Multiple characters
```

**C API:**
```c
void tui_test_send_input(tui_test_renderer *renderer, const char *input, int len);
void tui_test_send_key(tui_test_renderer *renderer, int key_code);
```

---

### 3. State Inspection

Query the rendered tree and component state.

**PHP API:**
```php
// Query by ID
$element = $renderer->getById('submit-btn');
$element->getText();      // Get text content
$element->isFocused();    // Check focus state
$element->getStyles();    // Get computed styles

// Query by text content
$elements = $renderer->getByText('Submit');

// Query by type
$boxes = $renderer->getAllByType(Box::class);
```

**C API:**
```c
tui_node* tui_test_get_by_id(tui_test_renderer *renderer, const char *id);
tui_node** tui_test_get_by_text(tui_test_renderer *renderer, const char *text, int *count);
int tui_test_node_is_focused(tui_node *node);
```

---

### 4. Snapshot Testing

Compare rendered output against saved snapshots.

**PHP API:**
```php
// Auto-generates snapshot file on first run
$renderer->assertMatchesSnapshot('my-component-initial');

// After interaction
$renderer->sendKey(Key::TAB);
$renderer->assertMatchesSnapshot('my-component-focused');
```

**Snapshot Format:**
```
--- Snapshot: my-component-initial ---
Width: 80, Height: 24
Checksum: abc123
---
┌────────┐
│ Hello  │
└────────┘
```

**Storage:** `.tui-snapshots/` directory alongside test files

---

### 5. Async Testing Support

Wait for state changes and re-renders.

**PHP API:**
```php
// Wait for next render
$renderer->waitForRender();

// Wait for condition
$renderer->waitFor(fn() => $renderer->getById('loading') === null);

// Wait for text to appear
$renderer->waitForText('Data loaded');

// With timeout
$renderer->waitFor(fn() => $condition, timeout: 5000);
```

---

### 6. Timer Mocking

Control time for testing timer-based behavior.

**PHP API:**
```php
$renderer = new TestRenderer(80, 24, ['mockTimers' => true]);

// Advance time
$renderer->advanceTimersByTime(1000);  // Advance 1 second

// Run all pending timers
$renderer->runAllTimers();

// Run only immediate timers
$renderer->runOnlyPendingTimers();
```

---

### 7. Assertions

Built-in assertion helpers.

**PHP API:**
```php
$renderer->assertTextPresent('Hello');
$renderer->assertTextNotPresent('Error');
$renderer->assertFocused('input-field');
$renderer->assertVisible('modal');
$renderer->assertHidden('tooltip');
$renderer->assertOutputEquals($expectedString);
$renderer->assertOutputContains('substring');
```

---

## Integration with PHPUnit

```php
use Xocdr\Tui\Testing\TuiTestCase;

class MyComponentTest extends TuiTestCase
{
    public function testRenders()
    {
        $this->render(fn($t) => new Box(['width' => 20], [
            new Text('Hello World')
        ]));

        $this->assertTextPresent('Hello World');
    }

    public function testInteraction()
    {
        $this->render(fn($t) => new MyForm());

        $this->sendKey(Key::TAB);
        $this->assertFocused('email-input');

        $this->sendSequence('test@example.com');
        $this->sendKey(Key::ENTER);

        $this->waitForText('Form submitted');
    }
}
```

---

## Implementation Plan

### Phase 1: Core Testing Infrastructure

1. **TestRenderer class**
   - Create `src/testing/renderer.c` with headless buffer rendering
   - PHP wrapper class `Xocdr\Tui\Testing\TestRenderer`
   - No terminal I/O, pure in-memory rendering

2. **Output Capture**
   - Reuse existing `tui_buffer` for off-screen rendering
   - Add `tui_buffer_to_string()` for text extraction
   - Handle ANSI codes (strip or preserve based on config)

### Phase 2: Input Simulation

3. **Input injection**
   - Add `tui_test_inject_input()` to bypass terminal
   - Map key constants to escape sequences
   - Trigger input handlers synchronously

4. **Focus simulation**
   - Simulate Tab/Shift+Tab navigation
   - Direct focus via ID

### Phase 3: Querying

5. **Element queries**
   - Tree traversal helpers for finding nodes
   - Text content extraction
   - Style/state inspection

6. **Wait utilities**
   - Polling-based wait with timeout
   - Integrate with PHP's event loop (if async)

### Phase 4: Snapshot Testing

7. **Snapshot capture/compare**
   - File I/O for snapshots
   - Diff output on mismatch
   - Update mode for regenerating

### Phase 5: Framework Integration

8. **PHPUnit integration**
   - Base test case class
   - Custom assertions
   - CI/CD compatibility (no terminal required)

---

## Files to Create

| File | Purpose |
|------|---------|
| `src/testing/renderer.c` | Headless test renderer |
| `src/testing/renderer.h` | Public C API |
| `src/testing/query.c` | Element query functions |
| `src/testing/input.c` | Input simulation |
| `src/testing/snapshot.c` | Snapshot file handling |
| `tests/testing/*.phpt` | Test framework tests |

**PHP Package:** This feature is best implemented in `xocdr/tui-testing` composer package, with minimal C-level support in ext-tui for headless rendering.

---

## Example: Full Test

```php
<?php

use Xocdr\Tui\Testing\TuiTestCase;
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;
use Xocdr\Tui\Ext\Key;

class CounterTest extends TuiTestCase
{
    public function testCounterIncrements()
    {
        // Render counter component
        $this->render(function ($t) {
            [$count, $setCount] = $t->state(0);

            $t->onInput(function ($input, $key) use ($setCount, $count) {
                if ($key->name === 'return') {
                    $setCount($count + 1);
                }
            });

            return new Box(['padding' => 1], [
                new Text("Count: {$count}"),
                new Text(['id' => 'hint'], 'Press Enter to increment')
            ]);
        });

        // Initial state
        $this->assertTextPresent('Count: 0');

        // Simulate Enter key
        $this->sendKey(Key::ENTER);
        $this->waitForRender();

        // Verify increment
        $this->assertTextPresent('Count: 1');

        // Snapshot test
        $this->assertMatchesSnapshot('counter-incremented');
    }
}
```

---

## Status

**Priority:** Future Enhancement
**Delegated to:** `xocdr/tui-testing` (new package)
**ext-tui support required:** Minimal - headless buffer mode only

---

## References

- [React Testing Library](https://testing-library.com/docs/react-testing-library/intro/)
- [Ink Testing](https://github.com/vadimdemedes/ink#testing)
- [Jest Snapshot Testing](https://jestjs.io/docs/snapshot-testing)
