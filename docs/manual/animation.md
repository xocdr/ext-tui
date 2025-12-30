# Animation

Smooth animations with easing functions.

## Overview

ext-tui provides three functions for animation:

- `tui_ease(t, easing)` - Apply easing to a time value
- `tui_lerp(start, end, t)` - Linear interpolation
- `tui_lerp_color(from, to, t)` - Color interpolation

## Basic Animation

The animation pattern:

1. Calculate progress `t` (0.0 to 1.0)
2. Apply easing with `tui_ease()`
3. Interpolate values with `tui_lerp()`
4. Draw and wait for next frame

```php
$duration = 60;  // frames

for ($frame = 0; $frame < $duration; $frame++) {
    // Calculate linear progress
    $t = $frame / ($duration - 1);  // 0.0 to 1.0

    // Apply easing
    $eased = tui_ease($t, TUI_EASE_OUT_BOUNCE);

    // Interpolate position
    $x = (int) tui_lerp(0, 50, $eased);

    // Draw
    tui_buffer_clear($buffer);
    tui_fill_rect($buffer, $x, 10, 3, 1, $style);
    tui_buffer_render($buffer);

    usleep(16666);  // ~60 FPS
}
```

## Easing Functions

### Linear

Constant speed:

```php
TUI_EASE_LINEAR
```

### Quadratic

Moderate acceleration/deceleration:

```php
TUI_EASE_IN_QUAD      // Slow start
TUI_EASE_OUT_QUAD     // Slow end
TUI_EASE_IN_OUT_QUAD  // Slow both ends
```

### Cubic

Stronger curves:

```php
TUI_EASE_IN_CUBIC
TUI_EASE_OUT_CUBIC
TUI_EASE_IN_OUT_CUBIC
```

### Quartic

Even stronger:

```php
TUI_EASE_IN_QUART
TUI_EASE_OUT_QUART
TUI_EASE_IN_OUT_QUART
```

### Sinusoidal

Gentle, natural movement:

```php
TUI_EASE_IN_SINE
TUI_EASE_OUT_SINE
TUI_EASE_IN_OUT_SINE
```

### Special Effects

```php
TUI_EASE_OUT_BOUNCE    // Bouncy landing
TUI_EASE_OUT_ELASTIC   // Springy overshoot
TUI_EASE_OUT_BACK      // Slight overshoot
```

## Color Animation

Transition between colors:

```php
$from = [255, 0, 0];    // Red
$to = [0, 0, 255];      // Blue

for ($frame = 0; $frame < 60; $frame++) {
    $t = $frame / 59;
    $eased = tui_ease($t, TUI_EASE_IN_OUT_SINE);

    $color = tui_lerp_color($from, $to, $eased);

    // Use $color for drawing
}
```

## Parallel Animation

Animate multiple properties:

```php
for ($frame = 0; $frame < 60; $frame++) {
    $t = $frame / 59;

    // Each property can use different easing
    $x = tui_lerp(0, 50, tui_ease($t, TUI_EASE_OUT_QUAD));
    $y = tui_lerp(0, 20, tui_ease($t, TUI_EASE_OUT_QUAD));
    $size = tui_lerp(1, 10, tui_ease($t, TUI_EASE_OUT_ELASTIC));

    // Draw box with animated position and size
}
```

## Staggered Animation

Animate items sequentially:

```php
$items = 5;
$delay = 10;  // frames between items
$itemDuration = 30;
$totalFrames = $itemDuration + ($items - 1) * $delay;

for ($frame = 0; $frame < $totalFrames; $frame++) {
    tui_buffer_clear($buffer);

    for ($i = 0; $i < $items; $i++) {
        $start = $i * $delay;

        if ($frame < $start) continue;

        $itemT = min(1.0, ($frame - $start) / ($itemDuration - 1));
        $eased = tui_ease($itemT, TUI_EASE_OUT_BACK);
        $x = (int) tui_lerp(0, 40, $eased);

        tui_fill_rect($buffer, $x, 2 + $i * 2, 5, 1, $style);
    }

    tui_buffer_render($buffer);
    usleep(16666);
}
```

## Frame Rate Control

### Fixed Frame Time

```php
$targetFps = 60;
$frameTime = 16666;  // microseconds

for ($frame = 0; $frame < $duration; $frame++) {
    $start = hrtime(true);

    // Animation and drawing...

    $elapsed = (hrtime(true) - $start) / 1000;
    $sleep = max(0, $frameTime - $elapsed);
    usleep((int) $sleep);
}
```

### Frame Times

| FPS | usleep |
|-----|--------|
| 60 | 16,666 |
| 30 | 33,333 |
| 24 | 41,666 |

## Gradients

Create color gradients for visual effects:

```php
$gradient = tui_gradient([255, 0, 0], [0, 0, 255], 20);
// Returns array of 20 colors from red to blue

foreach ($gradient as $i => $color) {
    tui_fill_rect($buffer, $i * 2, 5, 2, 3, [
        'char' => '█',
        'fg' => $color,
    ]);
}
```

## Example: Animated Progress Bar

```php
function animateProgress(float $from, float $to, int $frames): void
{
    global $buffer;
    $barWidth = 50;

    for ($frame = 0; $frame < $frames; $frame++) {
        $t = $frame / ($frames - 1);
        $eased = tui_ease($t, TUI_EASE_OUT_CUBIC);
        $progress = tui_lerp($from, $to, $eased);

        tui_buffer_clear($buffer);

        $filled = (int)($barWidth * $progress);
        tui_fill_rect($buffer, 5, 10, $filled, 1, [
            'char' => '█',
            'fg' => [100, 255, 100],
        ]);
        tui_fill_rect($buffer, 5 + $filled, 10, $barWidth - $filled, 1, [
            'char' => '░',
            'fg' => [100, 100, 100],
        ]);

        tui_buffer_render($buffer);
        usleep(16666);
    }
}

animateProgress(0.0, 0.75, 60);
```

## Example: Bouncing Ball

```php
$duration = 120;
$startY = 2;
$groundY = 20;

for ($frame = 0; $frame < $duration; $frame++) {
    // Ball drops, bounces back up, drops again
    $cycle = ($frame % 60) / 59;
    $eased = tui_ease($cycle, TUI_EASE_OUT_BOUNCE);
    $y = (int) tui_lerp($startY, $groundY, $eased);

    tui_buffer_clear($buffer);
    tui_fill_rect($buffer, 20, $y, 1, 1, [
        'char' => '●',
        'fg' => [255, 100, 100],
    ]);
    tui_buffer_render($buffer);

    usleep(16666);
}
```

## See Also

- [Drawing](drawing.md) - Buffer and canvas graphics
- [Reference: Functions](../reference/functions.md) - Animation functions
