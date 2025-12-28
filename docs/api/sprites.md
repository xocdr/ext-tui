# API Reference: Sprite Functions

[Back to Documentation](../README.md) | [Feature Guide](../features/sprites.md)

---

## tui_sprite_create

Create a new sprite.

```php
resource tui_sprite_create(array $frames, int $width, int $height)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$frames` | array | Array of frame strings |
| `$width` | int | Sprite width in characters |
| `$height` | int | Sprite height in characters |

### Return Value

Returns a sprite resource on success.

### Example

```php
// Single-frame sprite
$bullet = tui_sprite_create(["●"], 1, 1);

// Multi-line sprite
$ship = tui_sprite_create([
    "  ^  \n" .
    " /|\\ \n" .
    "/___\\"
], 5, 3);

// Animated sprite (multiple frames)
$explosion = tui_sprite_create([
    " * \n*X*\n * ",   // Frame 0
    " . \n.o.\n . ",   // Frame 1
    "   \n . \n   ",   // Frame 2
], 3, 3);
```

### Related

- [tui_sprite_free()](#tui_sprite_free) - Free sprite
- [tui_sprite_render()](#tui_sprite_render) - Render sprite

---

## tui_sprite_free

Free a sprite resource.

```php
void tui_sprite_free(resource $sprite)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$sprite` | resource | Sprite to free |

---

## tui_sprite_set_position

Set sprite position.

```php
void tui_sprite_set_position(resource $sprite, float $x, float $y)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$sprite` | resource | Target sprite |
| `$x` | float | X coordinate |
| `$y` | float | Y coordinate |

### Example

```php
tui_sprite_set_position($player, 10.5, 20.0);
```

### Notes

- Coordinates can be fractional for smooth movement
- Integer part is used for rendering position

---

## tui_sprite_get_position

Get current sprite position.

```php
array tui_sprite_get_position(resource $sprite)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$sprite` | resource | Source sprite |

### Return Value

Returns array `[x, y]` with current position.

### Example

```php
[$x, $y] = tui_sprite_get_position($player);
```

---

## tui_sprite_set_velocity

Set sprite velocity for automatic movement.

```php
void tui_sprite_set_velocity(resource $sprite, float $vx, float $vy)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$sprite` | resource | Target sprite |
| `$vx` | float | X velocity (units per frame) |
| `$vy` | float | Y velocity (units per frame) |

### Example

```php
// Move right at 0.5 units per frame
tui_sprite_set_velocity($ball, 0.5, 0);

// Diagonal movement
tui_sprite_set_velocity($bullet, 1.0, -0.5);
```

---

## tui_sprite_update

Update sprite position based on velocity.

```php
void tui_sprite_update(resource $sprite)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$sprite` | resource | Sprite to update |

### Example

```php
while ($running) {
    tui_sprite_update($ball);
    // position += velocity
}
```

### Notes

- Call once per frame
- Adds velocity to position

---

## tui_sprite_set_frame

Set the current animation frame.

```php
void tui_sprite_set_frame(resource $sprite, int $frame)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$sprite` | resource | Target sprite |
| `$frame` | int | Frame index (0-based) |

### Example

```php
// Show explosion frame 2
tui_sprite_set_frame($explosion, 2);
```

---

## tui_sprite_next_frame

Advance to the next animation frame.

```php
void tui_sprite_next_frame(resource $sprite)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$sprite` | resource | Target sprite |

### Example

```php
// Animate every 5 frames
if ($game_frame % 5 === 0) {
    tui_sprite_next_frame($player);
}
```

### Notes

- Wraps around to frame 0 after last frame

---

## tui_sprite_render

Render sprite to a buffer.

```php
void tui_sprite_render(resource $sprite, resource $buffer, array $style = [])
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$sprite` | resource | Sprite to render |
| `$buffer` | resource | Target buffer |
| `$style` | array | Optional style array |

### Example

```php
// Render all sprites
tui_sprite_render($player, $buf, ['fg' => [100, 200, 255]]);
tui_sprite_render($enemy, $buf, ['fg' => [255, 100, 100]]);
tui_sprite_render($bullet, $buf, ['fg' => [255, 255, 0]]);
```

---

## Complete Example

```php
<?php
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

// Create bouncing ball
$ball = tui_sprite_create(["●"], 1, 1);
tui_sprite_set_position($ball, $width / 2, $height / 2);
tui_sprite_set_velocity($ball, 0.8, 0.4);

$style = ['fg' => [255, 200, 0]];

while (true) {
    tui_buffer_clear($buf);

    // Update position
    tui_sprite_update($ball);

    // Bounce off walls
    [$x, $y] = tui_sprite_get_position($ball);

    if ($x <= 0 || $x >= $width - 1) {
        // Get current velocity, reverse X
        tui_sprite_set_velocity($ball, -0.8, 0.4);  // Simplified
    }
    if ($y <= 0 || $y >= $height - 1) {
        // Reverse Y
        tui_sprite_set_velocity($ball, 0.8, -0.4);  // Simplified
    }

    // Clamp to bounds
    $x = max(0, min($width - 1, $x));
    $y = max(0, min($height - 1, $y));
    tui_sprite_set_position($ball, $x, $y);

    // Render
    tui_sprite_render($ball, $buf, $style);
    tui_render($buf);

    usleep(33333);  // ~30 FPS
}

tui_sprite_free($ball);
tui_buffer_free($buf);
```

---

[Back to Documentation](../README.md) | [Feature Guide](../features/sprites.md)
