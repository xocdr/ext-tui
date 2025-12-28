# Animation

[← Canvas](canvas.md) | [Back to Documentation](../README.md) | [Next: Tables →](tables.md)

---

The animation system provides smooth, eased animations with 15+ built-in easing functions for natural-looking motion.

## Overview

Animations interpolate values over time with optional easing:

- **Linear**: Constant speed
- **Ease In**: Start slow, end fast
- **Ease Out**: Start fast, end slow
- **Ease In-Out**: Slow at both ends
- **Special**: Bounce, elastic, back

## Easing Functions

| Constant | Description | Use Case |
|----------|-------------|----------|
| `TUI_EASE_LINEAR` | Constant speed | Mechanical motion |
| `TUI_EASE_IN_QUAD` | Accelerate (quadratic) | Objects starting to move |
| `TUI_EASE_OUT_QUAD` | Decelerate (quadratic) | Objects coming to rest |
| `TUI_EASE_IN_OUT_QUAD` | Smooth both ends | General animations |
| `TUI_EASE_IN_CUBIC` | Strong acceleration | Dramatic entrances |
| `TUI_EASE_OUT_CUBIC` | Strong deceleration | Satisfying stops |
| `TUI_EASE_IN_OUT_CUBIC` | Smooth, pronounced | UI transitions |
| `TUI_EASE_IN_QUART` | Very strong accel | Heavy objects |
| `TUI_EASE_OUT_QUART` | Very strong decel | Impactful stops |
| `TUI_EASE_IN_OUT_QUART` | Very smooth | Polished UIs |
| `TUI_EASE_IN_SINE` | Gentle acceleration | Subtle motion |
| `TUI_EASE_OUT_SINE` | Gentle deceleration | Natural stops |
| `TUI_EASE_IN_OUT_SINE` | Natural flow | Breathing effects |
| `TUI_EASE_OUT_BOUNCE` | Bouncy end | Playful feedback |
| `TUI_EASE_OUT_ELASTIC` | Springy overshoot | Attention-grabbing |
| `TUI_EASE_OUT_BACK` | Slight overshoot | Snappy UI |

## Functions

### tui_animation_create()

Create a new animation.

```php
resource tui_animation_create(float $from, float $to, int $frames, int $easing = TUI_EASE_LINEAR)
```

**Parameters**:
- `$from`: Starting value
- `$to`: Ending value
- `$frames`: Number of frames (duration)
- `$easing`: Easing function constant

**Returns**: Animation resource

**Example**:
```php
// Animate X position from 0 to 60 over 30 frames
$anim = tui_animation_create(0, 60, 30, TUI_EASE_OUT_QUAD);
```

---

### tui_animation_value()

Get the current interpolated value.

```php
float tui_animation_value(resource $animation)
```

**Example**:
```php
$x = tui_animation_value($anim);
// Use $x for positioning
```

---

### tui_animation_step()

Advance the animation by one frame.

```php
void tui_animation_step(resource $animation)
```

**Example**:
```php
while (!tui_animation_done($anim)) {
    $value = tui_animation_value($anim);
    // ... render ...
    tui_animation_step($anim);
}
```

---

### tui_animation_done()

Check if the animation has completed.

```php
bool tui_animation_done(resource $animation)
```

**Returns**: `true` if animation is complete, `false` otherwise

---

### tui_animation_free()

Free an animation resource.

```php
void tui_animation_free(resource $animation)
```

---

## Practical Patterns

### Basic Animation Loop

```php
<?php
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

$style = ['fg' => [100, 200, 255], 'bold' => true];

// Create animation: move from left to right
$anim = tui_animation_create(0, $width - 5, 60, TUI_EASE_OUT_BOUNCE);

while (!tui_animation_done($anim)) {
    $x = (int)tui_animation_value($anim);

    tui_buffer_clear($buf);
    tui_buffer_write($buf, $x, $height / 2, "●", $style);
    tui_render($buf);

    tui_animation_step($anim);
    usleep(16666);  // ~60 FPS
}

tui_animation_free($anim);
tui_buffer_free($buf);
```

### Parallel Animations

```php
// Animate multiple properties simultaneously
$anim_x = tui_animation_create(0, 60, 45, TUI_EASE_OUT_QUAD);
$anim_y = tui_animation_create(0, 20, 45, TUI_EASE_OUT_QUAD);
$anim_scale = tui_animation_create(0.1, 1.0, 45, TUI_EASE_OUT_ELASTIC);

while (!tui_animation_done($anim_x)) {
    $x = (int)tui_animation_value($anim_x);
    $y = (int)tui_animation_value($anim_y);
    $scale = tui_animation_value($anim_scale);

    // Draw with position and scale
    tui_buffer_clear($buf);
    $size = (int)(5 * $scale);
    tui_draw_box($buf, $x, $y, $size, $size, $style);
    tui_render($buf);

    tui_animation_step($anim_x);
    tui_animation_step($anim_y);
    tui_animation_step($anim_scale);
    usleep(16666);
}

tui_animation_free($anim_x);
tui_animation_free($anim_y);
tui_animation_free($anim_scale);
```

### Sequential Animations

```php
class AnimationSequence
{
    private array $animations = [];
    private int $current = 0;

    public function add(float $from, float $to, int $frames, int $easing): self
    {
        $this->animations[] = [
            'from' => $from,
            'to' => $to,
            'frames' => $frames,
            'easing' => $easing,
            'resource' => null,
        ];
        return $this;
    }

    public function start(): void
    {
        $this->current = 0;
        $this->startCurrent();
    }

    private function startCurrent(): void
    {
        if ($this->current < count($this->animations)) {
            $a = &$this->animations[$this->current];
            $a['resource'] = tui_animation_create(
                $a['from'], $a['to'], $a['frames'], $a['easing']
            );
        }
    }

    public function value(): float
    {
        if ($this->current >= count($this->animations)) {
            return $this->animations[count($this->animations) - 1]['to'];
        }
        return tui_animation_value($this->animations[$this->current]['resource']);
    }

    public function step(): void
    {
        if ($this->done()) return;

        tui_animation_step($this->animations[$this->current]['resource']);

        if (tui_animation_done($this->animations[$this->current]['resource'])) {
            tui_animation_free($this->animations[$this->current]['resource']);
            $this->current++;
            $this->startCurrent();
        }
    }

    public function done(): bool
    {
        return $this->current >= count($this->animations);
    }
}

// Usage: bounce in, pause, bounce out
$seq = new AnimationSequence();
$seq->add(0, 40, 30, TUI_EASE_OUT_BOUNCE)   // Enter
    ->add(40, 40, 30, TUI_EASE_LINEAR)       // Pause (same value)
    ->add(40, 80, 30, TUI_EASE_IN_BACK);     // Exit

$seq->start();
while (!$seq->done()) {
    $x = (int)$seq->value();
    // ... render at $x ...
    $seq->step();
    usleep(16666);
}
```

### Color Animation

```php
function animate_color(array $from, array $to, int $frames, int $easing): Generator
{
    $anim_r = tui_animation_create($from[0], $to[0], $frames, $easing);
    $anim_g = tui_animation_create($from[1], $to[1], $frames, $easing);
    $anim_b = tui_animation_create($from[2], $to[2], $frames, $easing);

    while (!tui_animation_done($anim_r)) {
        yield [
            (int)tui_animation_value($anim_r),
            (int)tui_animation_value($anim_g),
            (int)tui_animation_value($anim_b),
        ];

        tui_animation_step($anim_r);
        tui_animation_step($anim_g);
        tui_animation_step($anim_b);
    }

    tui_animation_free($anim_r);
    tui_animation_free($anim_g);
    tui_animation_free($anim_b);
}

// Usage: fade from red to green
$from_color = [255, 0, 0];
$to_color = [0, 255, 0];

foreach (animate_color($from_color, $to_color, 60, TUI_EASE_IN_OUT_SINE) as $color) {
    $style = ['fg' => $color];
    tui_buffer_clear($buf);
    tui_fill_rect($buf, 10, 5, 30, 10, '█', $style);
    tui_render($buf);
    usleep(16666);
}
```

### Staggered Animation

```php
// Animate multiple items with delay between each
function staggered_animation(int $count, int $delay_frames): array
{
    $animations = [];

    for ($i = 0; $i < $count; $i++) {
        $animations[] = [
            'anim' => null,
            'delay' => $i * $delay_frames,
            'started' => false,
        ];
    }

    return $animations;
}

// Usage
$items = staggered_animation(5, 10);
$frame = 0;

while (true) {
    $all_done = true;

    foreach ($items as $i => &$item) {
        // Start animation after delay
        if (!$item['started'] && $frame >= $item['delay']) {
            $item['anim'] = tui_animation_create(0, 50, 30, TUI_EASE_OUT_BACK);
            $item['started'] = true;
        }

        // Get value and render
        if ($item['anim']) {
            $x = (int)tui_animation_value($item['anim']);
            tui_buffer_write($buf, $x, 2 + $i * 2, "Item $i", $style);

            if (!tui_animation_done($item['anim'])) {
                $all_done = false;
                tui_animation_step($item['anim']);
            }
        } else {
            $all_done = false;
        }
    }

    tui_render($buf);
    $frame++;

    if ($all_done) break;
    usleep(16666);
}
```

### Progress Bar Animation

```php
function animated_progress(float $from, float $to, int $duration_frames): void
{
    global $buf, $width;

    $anim = tui_animation_create($from * 100, $to * 100, $duration_frames, TUI_EASE_OUT_CUBIC);
    $bar_width = 50;

    while (!tui_animation_done($anim)) {
        $progress = tui_animation_value($anim) / 100;

        tui_buffer_clear($buf);

        // Draw progress bar
        $filled = (int)($bar_width * $progress);
        $bar = str_repeat('█', $filled) . str_repeat('░', $bar_width - $filled);
        $label = sprintf(" %3d%%", (int)($progress * 100));

        tui_buffer_write($buf, 5, 10, "[" . $bar . "]" . $label, $style);
        tui_render($buf);

        tui_animation_step($anim);
        usleep(16666);
    }

    tui_animation_free($anim);
}

// Animate from 0% to 75%
animated_progress(0, 0.75, 60);
```

## Timing and Frame Rates

| Target FPS | usleep() value | Frame time |
|------------|----------------|------------|
| 60 FPS | 16,666 µs | 16.67 ms |
| 30 FPS | 33,333 µs | 33.33 ms |
| 24 FPS | 41,666 µs | 41.67 ms |
| 15 FPS | 66,666 µs | 66.67 ms |

### Compensating for Processing Time

```php
$target_frame_time = 16666;  // ~60 FPS in microseconds

while (!$done) {
    $frame_start = hrtime(true);

    // ... animation and rendering ...

    $elapsed = (hrtime(true) - $frame_start) / 1000;  // Convert to µs
    $sleep_time = max(0, $target_frame_time - $elapsed);
    usleep((int)$sleep_time);
}
```

## Related Functions

- [tui_spinner()](../api/progress.md#tui_spinner) - Animated loading spinner
- [tui_progress_animated()](../api/progress.md#tui_progress_animated) - Animated progress bar
- [tui_canvas_*](../api/canvas.md) - Animate canvas graphics

## See Also

- [Progress & Spinners](progress.md) - Loading indicators
- [Canvas](canvas.md) - High-resolution animated graphics
- [API Reference: Animation](../api/animation.md) - Complete API details

---

[← Canvas](canvas.md) | [Back to Documentation](../README.md) | [Next: Tables →](tables.md)
