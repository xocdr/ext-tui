# Progress & Spinners

[← Tables](tables.md) | [Back to Documentation](../README.md) | [Next: Sprites →](sprites.md)

---

Progress indicators show task completion status or ongoing activity through progress bars and animated spinners.

## Overview

Two types of progress indicators:

- **Progress Bars**: Show completion percentage (deterministic)
- **Spinners**: Show ongoing activity (indeterminate)

## Progress Bar Functions

### tui_progress_bar()

Generate a progress bar string.

```php
string tui_progress_bar(float $progress, int $width, array $style = [])
```

**Parameters**:
- `$progress`: Progress value from 0.0 to 1.0
- `$width`: Total width in characters
- `$style`: Optional style array

**Returns**: Formatted progress bar string

**Example**:
```php
// 75% complete, 40 characters wide
$bar = tui_progress_bar(0.75, 40, $style);
echo $bar;  // "██████████████████████████████░░░░░░░░░░"

// With percentage label
$progress = 0.75;
$bar = tui_progress_bar($progress, 30, $style);
$label = sprintf(" %3d%%", (int)($progress * 100));
echo $bar . $label;
```

---

### tui_progress_animated()

Generate an animated progress bar frame.

```php
string tui_progress_animated(int $frame, int $width, array $style = [])
```

**Parameters**:
- `$frame`: Current animation frame number
- `$width`: Total width in characters
- `$style`: Optional style array

**Returns**: Animated progress bar string for indeterminate progress

**Example**:
```php
$frame = 0;
while ($task_running) {
    $bar = tui_progress_animated($frame++, 40, $style);
    tui_buffer_write($buf, 5, 10, $bar, $style);
    tui_render($buf);
    usleep(50000);  // 20 FPS
}
```

---

## Spinner Functions

### tui_spinner()

Get a spinner character for the current frame.

```php
string tui_spinner(int $frame, int $style = TUI_SPINNER_DOTS)
```

**Parameters**:
- `$frame`: Current animation frame number
- `$style`: Spinner style constant

**Spinner Styles**:
| Constant | Characters | Description |
|----------|------------|-------------|
| `TUI_SPINNER_DOTS` | `⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏` | Braille dots pattern |
| `TUI_SPINNER_LINE` | `\|/-\` | Classic ASCII line |
| `TUI_SPINNER_CIRCLE` | `◐◓◑◒` | Quarter circle |
| `TUI_SPINNER_SQUARE` | `◰◳◲◱` | Quarter square |
| `TUI_SPINNER_ARROW` | `←↖↑↗→↘↓↙` | Rotating arrow |

**Returns**: Single spinner character

**Example**:
```php
$frame = 0;
while ($loading) {
    $spinner = tui_spinner($frame++, TUI_SPINNER_DOTS);
    tui_buffer_write($buf, 5, 5, "$spinner Loading...", $style);
    tui_render($buf);
    usleep(80000);  // ~12 FPS
}
```

---

### tui_spinner_frames()

Get all frames for a spinner style.

```php
array tui_spinner_frames(int $style = TUI_SPINNER_DOTS)
```

**Parameters**:
- `$style`: Spinner style constant

**Returns**: Array of frame characters

**Example**:
```php
$frames = tui_spinner_frames(TUI_SPINNER_DOTS);
// ['⠋', '⠙', '⠹', '⠸', '⠼', '⠴', '⠦', '⠧', '⠇', '⠏']
```

---

## Practical Patterns

### Progress with ETA

```php
class ProgressTracker
{
    private float $start_time;
    private int $total;
    private int $current = 0;

    public function __construct(int $total)
    {
        $this->total = $total;
        $this->start_time = microtime(true);
    }

    public function increment(int $amount = 1): void
    {
        $this->current += $amount;
    }

    public function render(int $width): string
    {
        $progress = $this->total > 0 ? $this->current / $this->total : 0;
        $bar = tui_progress_bar($progress, $width - 25, []);  // Reserve space for text

        $percent = sprintf("%3d%%", (int)($progress * 100));
        $count = sprintf("%d/%d", $this->current, $this->total);
        $eta = $this->formatEta();

        return sprintf("%s %s [%s] %s", $bar, $percent, $count, $eta);
    }

    private function formatEta(): string
    {
        if ($this->current === 0) return "ETA: --:--";

        $elapsed = microtime(true) - $this->start_time;
        $rate = $this->current / $elapsed;
        $remaining = ($this->total - $this->current) / $rate;

        $mins = (int)($remaining / 60);
        $secs = (int)($remaining % 60);

        return sprintf("ETA: %02d:%02d", $mins, $secs);
    }
}

// Usage
$tracker = new ProgressTracker(1000);
foreach ($items as $item) {
    process($item);
    $tracker->increment();

    tui_buffer_clear($buf);
    tui_buffer_write($buf, 2, 5, $tracker->render(60), $style);
    tui_render($buf);
}
```

### Multi-Progress Display

```php
class MultiProgress
{
    private array $tasks = [];

    public function addTask(string $name, int $total): int
    {
        $id = count($this->tasks);
        $this->tasks[] = [
            'name' => $name,
            'total' => $total,
            'current' => 0,
        ];
        return $id;
    }

    public function update(int $id, int $current): void
    {
        if (isset($this->tasks[$id])) {
            $this->tasks[$id]['current'] = $current;
        }
    }

    public function render($buf, int $x, int $y, int $width): void
    {
        $label_width = 15;
        $bar_width = $width - $label_width - 8;  // 8 for percentage and spacing

        foreach ($this->tasks as $i => $task) {
            $progress = $task['total'] > 0 ? $task['current'] / $task['total'] : 0;

            // Label (truncated if needed)
            $label = tui_truncate($task['name'], $label_width);
            $label = tui_pad($label, $label_width, 'left');

            // Progress bar
            $bar = tui_progress_bar($progress, $bar_width, []);

            // Percentage
            $pct = sprintf("%3d%%", (int)($progress * 100));

            $line = "$label $bar $pct";
            tui_buffer_write($buf, $x, $y + $i, $line, []);
        }
    }
}

// Usage
$mp = new MultiProgress();
$download_id = $mp->addTask("Downloading", 100);
$extract_id = $mp->addTask("Extracting", 50);
$install_id = $mp->addTask("Installing", 200);

// Update in parallel
$mp->update($download_id, 75);
$mp->update($extract_id, 30);
$mp->update($install_id, 100);
$mp->render($buf, 2, 3, 60);
```

### Spinner with Status

```php
class SpinnerStatus
{
    private int $frame = 0;
    private int $style;
    private string $message;

    public function __construct(int $style = TUI_SPINNER_DOTS)
    {
        $this->style = $style;
        $this->message = "Loading...";
    }

    public function setMessage(string $message): void
    {
        $this->message = $message;
    }

    public function render(): string
    {
        $spinner = tui_spinner($this->frame++, $this->style);
        return "$spinner $this->message";
    }
}

// Usage
$spinner = new SpinnerStatus(TUI_SPINNER_DOTS);

$spinner->setMessage("Connecting to server...");
// render...

$spinner->setMessage("Authenticating...");
// render...

$spinner->setMessage("Fetching data...");
// render...
```

### Download Progress

```php
function render_download_progress(
    $buf,
    int $x,
    int $y,
    string $filename,
    int $downloaded,
    int $total,
    float $speed
): void {
    $progress = $total > 0 ? $downloaded / $total : 0;

    // Filename
    $name = tui_truncate($filename, 30);
    tui_buffer_write($buf, $x, $y, $name, ['bold' => true]);

    // Progress bar
    $bar = tui_progress_bar($progress, 40, []);
    tui_buffer_write($buf, $x, $y + 1, $bar, []);

    // Stats line
    $stats = sprintf(
        "%s / %s  (%s/s)  %d%%",
        format_bytes($downloaded),
        format_bytes($total),
        format_bytes($speed),
        (int)($progress * 100)
    );
    tui_buffer_write($buf, $x, $y + 2, $stats, ['dim' => true]);
}

function format_bytes(int $bytes): string
{
    $units = ['B', 'KB', 'MB', 'GB'];
    $i = 0;
    while ($bytes >= 1024 && $i < count($units) - 1) {
        $bytes /= 1024;
        $i++;
    }
    return sprintf("%.1f %s", $bytes, $units[$i]);
}
```

### Combined Spinner + Progress

```php
function render_task_status(
    $buf,
    int $x,
    int $y,
    int $frame,
    string $status,
    ?float $progress = null
): void {
    if ($progress !== null) {
        // Determinate: show progress bar
        $bar = tui_progress_bar($progress, 30, []);
        $pct = sprintf("%3d%%", (int)($progress * 100));
        tui_buffer_write($buf, $x, $y, "$bar $pct $status", []);
    } else {
        // Indeterminate: show spinner
        $spinner = tui_spinner($frame, TUI_SPINNER_DOTS);
        tui_buffer_write($buf, $x, $y, "$spinner $status", []);
    }
}

// Usage
$frame = 0;

// Phase 1: Connecting (indeterminate)
while (!$connected) {
    render_task_status($buf, 5, 10, $frame++, "Connecting...", null);
    tui_render($buf);
    usleep(80000);
}

// Phase 2: Downloading (determinate)
while ($downloaded < $total) {
    $progress = $downloaded / $total;
    render_task_status($buf, 5, 10, 0, "Downloading...", $progress);
    tui_render($buf);
    // ... download chunk ...
}
```

## Progress Bar Characters

```
Block style:
██████████░░░░░░░░░░  (default)

ASCII style:
==========----------

Gradient style:
▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░

Thin style:
━━━━━━━━━━──────────
```

## Related Functions

- [tui_animation_create()](../api/animation.md#tui_animation_create) - Animate progress changes
- [tui_buffer_write()](../api/buffers.md#tui_buffer_write) - Render to buffer

## See Also

- [Animation](animation.md) - Smooth progress animations
- [Buffers](buffers.md) - Rendering to screen
- [API Reference: Progress](../api/progress.md) - Complete API details

---

[← Tables](tables.md) | [Back to Documentation](../README.md) | [Next: Sprites →](sprites.md)
