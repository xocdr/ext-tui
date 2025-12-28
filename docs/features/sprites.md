# Sprites

[← Progress](progress.md) | [Back to Documentation](../README.md) | [Next: Events →](events.md)

---

Sprites are movable graphical elements that can be positioned, animated, and rendered on buffers. They're useful for games, visualizations, and animated UIs.

## Overview

Sprites provide:
- **Position tracking**: X, Y coordinates with sub-character precision
- **Velocity**: Automatic movement per frame
- **Collision bounds**: For hit detection
- **Frame animation**: Multiple frames for animated sprites
- **Z-ordering**: Layer control for overlapping sprites

## Functions

### tui_sprite_create()

Create a new sprite.

```php
resource tui_sprite_create(array $frames, int $width, int $height)
```

**Parameters**:
- `$frames`: Array of frame strings (multi-line strings)
- `$width`: Sprite width in characters
- `$height`: Sprite height in characters

**Returns**: Sprite resource

**Example**:
```php
// Single-frame sprite
$ship = tui_sprite_create([
    "  ^  \n" .
    " /|\\ \n" .
    "/___\\"
], 5, 3);

// Multi-frame animated sprite
$explosion = tui_sprite_create([
    " * \n" .
    "*X*\n" .
    " * ",

    " . \n" .
    ".o.\n" .
    " . ",

    "   \n" .
    " . \n" .
    "   ",
], 3, 3);
```

---

### tui_sprite_free()

Free a sprite resource.

```php
void tui_sprite_free(resource $sprite)
```

---

### tui_sprite_set_position()

Set sprite position.

```php
void tui_sprite_set_position(resource $sprite, float $x, float $y)
```

**Parameters**:
- `$sprite`: Sprite resource
- `$x`: X coordinate
- `$y`: Y coordinate

---

### tui_sprite_get_position()

Get current sprite position.

```php
array tui_sprite_get_position(resource $sprite)
```

**Returns**: Array `[x, y]`

---

### tui_sprite_set_velocity()

Set sprite velocity for automatic movement.

```php
void tui_sprite_set_velocity(resource $sprite, float $vx, float $vy)
```

**Parameters**:
- `$sprite`: Sprite resource
- `$vx`: X velocity (units per frame)
- `$vy`: Y velocity (units per frame)

---

### tui_sprite_update()

Update sprite position based on velocity.

```php
void tui_sprite_update(resource $sprite)
```

Call this each frame to apply velocity to position.

---

### tui_sprite_set_frame()

Set the current animation frame.

```php
void tui_sprite_set_frame(resource $sprite, int $frame)
```

**Parameters**:
- `$sprite`: Sprite resource
- `$frame`: Frame index (0-based)

---

### tui_sprite_next_frame()

Advance to the next animation frame (wraps around).

```php
void tui_sprite_next_frame(resource $sprite)
```

---

### tui_sprite_render()

Render sprite to a buffer.

```php
void tui_sprite_render(resource $sprite, resource $buffer, array $style = [])
```

**Parameters**:
- `$sprite`: Sprite resource
- `$buffer`: Target buffer
- `$style`: Optional style array

---

## Practical Patterns

### Animated Character

```php
<?php
// Define walking animation frames
$walk_frames = [
    "  O  \n" .
    " /|\\ \n" .
    " / \\",

    "  O  \n" .
    " /|\\ \n" .
    " | |",

    "  O  \n" .
    " /|\\ \n" .
    " \\ /",
];

$player = tui_sprite_create($walk_frames, 5, 3);
tui_sprite_set_position($player, 10, 10);

$frame = 0;
while ($running) {
    tui_buffer_clear($buf);

    // Animate every 10 frames
    if ($frame % 10 === 0) {
        tui_sprite_next_frame($player);
    }

    // Move right
    [$x, $y] = tui_sprite_get_position($player);
    tui_sprite_set_position($player, $x + 0.5, $y);

    // Render
    tui_sprite_render($player, $buf, $style);
    tui_render($buf);

    $frame++;
    usleep(50000);
}

tui_sprite_free($player);
```

### Bouncing Ball

```php
$ball = tui_sprite_create(["●"], 1, 1);
tui_sprite_set_position($ball, 20, 10);
tui_sprite_set_velocity($ball, 1.0, 0.5);

while ($running) {
    tui_buffer_clear($buf);

    // Update position
    tui_sprite_update($ball);

    // Bounce off walls
    [$x, $y] = tui_sprite_get_position($ball);
    $vx = 1.0;
    $vy = 0.5;

    if ($x <= 0 || $x >= $width - 1) {
        $vx = -$vx;
    }
    if ($y <= 0 || $y >= $height - 1) {
        $vy = -$vy;
    }
    tui_sprite_set_velocity($ball, $vx, $vy);

    // Clamp to bounds
    $x = max(0, min($width - 1, $x));
    $y = max(0, min($height - 1, $y));
    tui_sprite_set_position($ball, $x, $y);

    // Render
    tui_sprite_render($ball, $buf, ['fg' => [255, 200, 0]]);
    tui_render($buf);

    usleep(50000);
}
```

### Sprite Manager

```php
class SpriteManager
{
    private array $sprites = [];
    private int $next_id = 0;

    public function add($sprite, int $z_order = 0): int
    {
        $id = $this->next_id++;
        $this->sprites[$id] = [
            'sprite' => $sprite,
            'z_order' => $z_order,
            'visible' => true,
        ];
        $this->sort();
        return $id;
    }

    public function remove(int $id): void
    {
        if (isset($this->sprites[$id])) {
            tui_sprite_free($this->sprites[$id]['sprite']);
            unset($this->sprites[$id]);
        }
    }

    public function setVisible(int $id, bool $visible): void
    {
        if (isset($this->sprites[$id])) {
            $this->sprites[$id]['visible'] = $visible;
        }
    }

    public function setZOrder(int $id, int $z_order): void
    {
        if (isset($this->sprites[$id])) {
            $this->sprites[$id]['z_order'] = $z_order;
            $this->sort();
        }
    }

    public function updateAll(): void
    {
        foreach ($this->sprites as $data) {
            tui_sprite_update($data['sprite']);
        }
    }

    public function renderAll($buffer, array $style = []): void
    {
        foreach ($this->sprites as $data) {
            if ($data['visible']) {
                tui_sprite_render($data['sprite'], $buffer, $style);
            }
        }
    }

    private function sort(): void
    {
        uasort($this->sprites, fn($a, $b) => $a['z_order'] <=> $b['z_order']);
    }

    public function __destruct()
    {
        foreach ($this->sprites as $data) {
            tui_sprite_free($data['sprite']);
        }
    }
}
```

### Particle System

```php
class ParticleSystem
{
    private array $particles = [];
    private int $max_particles;

    public function __construct(int $max_particles = 100)
    {
        $this->max_particles = $max_particles;
    }

    public function emit(float $x, float $y, int $count = 5): void
    {
        for ($i = 0; $i < $count; $i++) {
            if (count($this->particles) >= $this->max_particles) {
                break;
            }

            $chars = ['*', '.', '·', '°', '•'];
            $char = $chars[array_rand($chars)];

            $sprite = tui_sprite_create([$char], 1, 1);
            tui_sprite_set_position($sprite, $x, $y);

            // Random velocity
            $vx = (mt_rand(-100, 100) / 100) * 2;
            $vy = (mt_rand(-150, -50) / 100);
            tui_sprite_set_velocity($sprite, $vx, $vy);

            $this->particles[] = [
                'sprite' => $sprite,
                'life' => 30 + mt_rand(0, 20),
                'age' => 0,
            ];
        }
    }

    public function update(): void
    {
        foreach ($this->particles as $i => &$p) {
            tui_sprite_update($p['sprite']);

            // Apply gravity
            [$x, $y] = tui_sprite_get_position($p['sprite']);
            // Assuming we stored velocity somewhere, add gravity
            $p['age']++;
        }

        // Remove dead particles
        $this->particles = array_filter($this->particles, function($p) {
            if ($p['age'] >= $p['life']) {
                tui_sprite_free($p['sprite']);
                return false;
            }
            return true;
        });
    }

    public function render($buffer): void
    {
        foreach ($this->particles as $p) {
            // Fade based on age
            $brightness = 255 - (int)(($p['age'] / $p['life']) * 200);
            $style = ['fg' => [$brightness, $brightness, $brightness]];
            tui_sprite_render($p['sprite'], $buffer, $style);
        }
    }

    public function __destruct()
    {
        foreach ($this->particles as $p) {
            tui_sprite_free($p['sprite']);
        }
    }
}

// Usage
$particles = new ParticleSystem(200);

while ($running) {
    // Emit particles on click or event
    if ($emit) {
        $particles->emit($mouse_x, $mouse_y, 10);
    }

    tui_buffer_clear($buf);
    $particles->update();
    $particles->render($buf);
    tui_render($buf);

    usleep(33333);
}
```

### Space Invaders Style

```php
// Invader animation
$invader_frames = [
    " /\"\\ \n" .
    "{o o}\n" .
    " \" \" ",

    " /\"\\ \n" .
    "{o o}\n" .
    "\"   \"",
];

$invaders = [];
for ($row = 0; $row < 3; $row++) {
    for ($col = 0; $col < 8; $col++) {
        $invader = tui_sprite_create($invader_frames, 5, 3);
        tui_sprite_set_position($invader, 5 + $col * 7, 2 + $row * 4);
        tui_sprite_set_velocity($invader, 0.2, 0);
        $invaders[] = $invader;
    }
}

// Player ship
$player = tui_sprite_create([
    "  ^  \n" .
    " /|\\ \n" .
    "/___\\"
], 5, 3);
tui_sprite_set_position($player, $width / 2 - 2, $height - 4);

// Game loop
$frame = 0;
while ($running) {
    tui_buffer_clear($buf);

    // Animate invaders
    if ($frame % 20 === 0) {
        foreach ($invaders as $invader) {
            tui_sprite_next_frame($invader);
        }
    }

    // Update invaders
    foreach ($invaders as $invader) {
        tui_sprite_update($invader);

        // Check for edge bounce
        [$x, $y] = tui_sprite_get_position($invader);
        if ($x <= 0 || $x >= $width - 5) {
            // Reverse direction and move down
            // ... game logic ...
        }

        tui_sprite_render($invader, $buf, ['fg' => [0, 255, 0]]);
    }

    // Render player
    tui_sprite_render($player, $buf, ['fg' => [100, 200, 255]]);

    tui_render($buf);
    $frame++;
    usleep(50000);
}
```

## Sprite Art Tips

```
Simple characters:
●○◉◎  - Balls, bullets
★☆✦✧  - Stars
▲▼◄►  - Arrows, ships
♠♥♦♣  - Cards, UI

Box drawing for structures:
┌──┐
│  │
└──┘

Block shading for 3D effect:
▓▓▓░
▓██░
░░░░
```

## Related Functions

- [tui_buffer_create()](../api/buffers.md#tui_buffer_create) - Create render target
- [tui_animation_create()](../api/animation.md#tui_animation_create) - Animate sprite properties
- [tui_canvas_render()](../api/canvas.md#tui_canvas_render) - High-res sprite graphics

## See Also

- [Animation](animation.md) - Eased animations
- [Canvas](canvas.md) - High-resolution graphics
- [API Reference: Sprites](../api/sprites.md) - Complete API details

---

[← Progress](progress.md) | [Back to Documentation](../README.md) | [Next: Events →](events.md)
