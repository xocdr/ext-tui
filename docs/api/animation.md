# API Reference: Animation Functions

[Back to Documentation](../README.md) | [Feature Guide](../features/animation.md)

---

## Constants

### Easing Functions

| Constant | Description |
|----------|-------------|
| `TUI_EASE_LINEAR` | Constant speed (no easing) |
| `TUI_EASE_IN_QUAD` | Quadratic acceleration |
| `TUI_EASE_OUT_QUAD` | Quadratic deceleration |
| `TUI_EASE_IN_OUT_QUAD` | Quadratic ease in and out |
| `TUI_EASE_IN_CUBIC` | Cubic acceleration |
| `TUI_EASE_OUT_CUBIC` | Cubic deceleration |
| `TUI_EASE_IN_OUT_CUBIC` | Cubic ease in and out |
| `TUI_EASE_IN_QUART` | Quartic acceleration |
| `TUI_EASE_OUT_QUART` | Quartic deceleration |
| `TUI_EASE_IN_OUT_QUART` | Quartic ease in and out |
| `TUI_EASE_IN_SINE` | Sinusoidal acceleration |
| `TUI_EASE_OUT_SINE` | Sinusoidal deceleration |
| `TUI_EASE_IN_OUT_SINE` | Sinusoidal ease in and out |
| `TUI_EASE_OUT_BOUNCE` | Bounce effect at end |
| `TUI_EASE_OUT_ELASTIC` | Elastic/springy effect at end |
| `TUI_EASE_OUT_BACK` | Slight overshoot at end |

---

## tui_animation_create

Create a new animation.

```php
resource tui_animation_create(float $from, float $to, int $frames, int $easing = TUI_EASE_LINEAR)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$from` | float | Starting value |
| `$to` | float | Ending value |
| `$frames` | int | Duration in frames |
| `$easing` | int | Easing function constant |

### Return Value

Returns an animation resource on success.

### Example

```php
// Animate from 0 to 100 over 60 frames with bounce
$anim = tui_animation_create(0, 100, 60, TUI_EASE_OUT_BOUNCE);
```

### Related

- [tui_animation_value()](#tui_animation_value) - Get current value
- [tui_animation_free()](#tui_animation_free) - Free animation

---

## tui_animation_value

Get the current interpolated value.

```php
float tui_animation_value(resource $animation)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$animation` | resource | Animation resource |

### Return Value

Returns the current interpolated value based on frame and easing.

### Example

```php
$value = tui_animation_value($anim);
$x = (int)$value;  // Use as position
```

### Notes

- Value is interpolated between `from` and `to`
- May overshoot for bounce/elastic/back easings

---

## tui_animation_step

Advance the animation by one frame.

```php
void tui_animation_step(resource $animation)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$animation` | resource | Animation resource |

### Example

```php
while (!tui_animation_done($anim)) {
    $value = tui_animation_value($anim);
    render_at($value);
    tui_animation_step($anim);
    usleep(16666);
}
```

### Notes

- Call once per frame
- After final frame, animation stays at `to` value

---

## tui_animation_done

Check if the animation has completed.

```php
bool tui_animation_done(resource $animation)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$animation` | resource | Animation resource |

### Return Value

Returns `true` if all frames have been played, `false` otherwise.

### Example

```php
if (tui_animation_done($anim)) {
    tui_animation_free($anim);
    start_next_animation();
}
```

---

## tui_animation_free

Free an animation resource.

```php
void tui_animation_free(resource $animation)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$animation` | resource | Animation to free |

### Example

```php
while (!tui_animation_done($anim)) {
    // ... animation loop ...
}
tui_animation_free($anim);
```

---

## Complete Example

```php
<?php
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

// Create animations for X and Y
$anim_x = tui_animation_create(0, $width - 5, 90, TUI_EASE_OUT_ELASTIC);
$anim_y = tui_animation_create(0, $height - 3, 90, TUI_EASE_OUT_BOUNCE);

$style = ['fg' => [100, 200, 255], 'bold' => true];

while (!tui_animation_done($anim_x) || !tui_animation_done($anim_y)) {
    tui_buffer_clear($buf);

    $x = (int)tui_animation_value($anim_x);
    $y = (int)tui_animation_value($anim_y);

    tui_buffer_write($buf, $x, $y, "Hello!", $style);
    tui_render($buf);

    if (!tui_animation_done($anim_x)) {
        tui_animation_step($anim_x);
    }
    if (!tui_animation_done($anim_y)) {
        tui_animation_step($anim_y);
    }

    usleep(16666);  // ~60 FPS
}

tui_animation_free($anim_x);
tui_animation_free($anim_y);
tui_buffer_free($buf);
```

---

[Back to Documentation](../README.md) | [Feature Guide](../features/animation.md)
