# Terminal Functions

[← Quick Start](../getting-started/quickstart.md) | [Back to Documentation](../README.md) | [Next: Text Utilities →](text.md)

---

Terminal functions help you detect the environment and get terminal information before building your UI.

## Overview

Before rendering any graphics, you should check:
1. Is the program running in an interactive terminal?
2. What are the terminal dimensions?
3. Is this a CI/automated environment?

## Functions

### tui_get_terminal_size()

Get the current terminal dimensions in characters.

```php
array tui_get_terminal_size()
```

**Returns**: Array with `[width, height]` in character cells.

**Example**:
```php
[$width, $height] = tui_get_terminal_size();
echo "Terminal is {$width}x{$height} characters\n";

// Create a full-screen buffer
$buf = tui_buffer_create($width, $height);
```

**Notes**:
- Returns the current size; terminal may be resized later
- Use `tui_on_resize()` to handle dimension changes
- Typical sizes: 80×24 (minimum), 120×40 (common), 200×50+ (large)

---

### tui_is_interactive()

Check if the program is running in an interactive terminal.

```php
bool tui_is_interactive()
```

**Returns**: `true` if running in an interactive terminal, `false` otherwise.

**Example**:
```php
if (!tui_is_interactive()) {
    // Fall back to simple text output
    echo "Results:\n";
    foreach ($items as $item) {
        echo "- $item\n";
    }
    exit(0);
}

// Interactive mode - use full TUI
$buf = tui_buffer_create(...);
```

**When it returns `false`**:
- Output is piped: `php script.php | less`
- Input is piped: `echo "data" | php script.php`
- Running in background: `php script.php &`
- No TTY attached (cron jobs, some Docker containers)

---

### tui_is_ci()

Check if the program is running in a CI/CD environment.

```php
bool tui_is_ci()
```

**Returns**: `true` if a CI environment is detected, `false` otherwise.

**Example**:
```php
if (tui_is_ci()) {
    // Disable colors and animations in CI
    $style = [];  // No styling
    $animate = false;
} else {
    $style = ['fg' => [100, 200, 255], 'bold' => true];
    $animate = true;
}
```

**Detection**:
Checks for common CI environment variables:
- `CI=true`
- `GITHUB_ACTIONS`
- `GITLAB_CI`
- `JENKINS_URL`
- `TRAVIS`
- `CIRCLECI`
- And others

---

## Practical Patterns

### Graceful Degradation

```php
<?php
function render_output(array $data): void
{
    // Level 1: Not interactive - plain text
    if (!tui_is_interactive()) {
        foreach ($data as $row) {
            echo implode("\t", $row) . "\n";
        }
        return;
    }

    // Level 2: CI environment - simple formatting
    if (tui_is_ci()) {
        echo str_repeat('=', 60) . "\n";
        foreach ($data as $row) {
            echo "| " . implode(" | ", $row) . " |\n";
        }
        echo str_repeat('=', 60) . "\n";
        return;
    }

    // Level 3: Full interactive - TUI with colors
    [$width, $height] = tui_get_terminal_size();
    $buf = tui_buffer_create($width, $height);

    // ... full TUI rendering
}
```

### Responsive Layout

```php
<?php
function create_layout(): array
{
    [$width, $height] = tui_get_terminal_size();

    if ($width < 80) {
        // Narrow terminal - stack vertically
        return [
            'layout' => 'vertical',
            'sidebar_width' => 0,
            'content_width' => $width - 2,
        ];
    } elseif ($width < 120) {
        // Medium terminal - small sidebar
        return [
            'layout' => 'horizontal',
            'sidebar_width' => 20,
            'content_width' => $width - 22,
        ];
    } else {
        // Wide terminal - full layout
        return [
            'layout' => 'horizontal',
            'sidebar_width' => 30,
            'content_width' => $width - 32,
        ];
    }
}
```

### Handle Resize Events

```php
<?php
$buf = null;

function setup_buffer(): void
{
    global $buf;
    [$width, $height] = tui_get_terminal_size();

    if ($buf) {
        tui_buffer_free($buf);
    }
    $buf = tui_buffer_create($width, $height);
}

// Initial setup
setup_buffer();

// Handle resize
tui_on_resize(function($width, $height) {
    setup_buffer();
    redraw_ui();
});

tui_run();
```

## Related Functions

- [tui_on_resize()](../api/events.md#tui_on_resize) - Handle terminal resize events
- [tui_buffer_create()](../api/buffers.md#tui_buffer_create) - Create screen buffer
- [tui_run()](../api/events.md#tui_run) - Start event loop

## See Also

- [Buffers & Rendering](buffers.md) - Creating and rendering buffers
- [Event Handling](events.md) - Handling terminal events
- [API Reference: Terminal](../api/terminal.md) - Complete API details

---

[← Quick Start](../getting-started/quickstart.md) | [Back to Documentation](../README.md) | [Next: Text Utilities →](text.md)
