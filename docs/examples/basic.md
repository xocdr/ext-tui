# Basic Examples

[Back to Documentation](../README.md)

---

## Hello World

The simplest ext-tui program:

```php
<?php
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

$style = ['fg' => [100, 200, 255], 'bold' => true];
tui_buffer_write($buf, 10, 5, "Hello, World!", $style);

tui_render($buf);
sleep(2);

tui_buffer_free($buf);
```

---

## Centered Box with Title

```php
<?php
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

$box_width = 40;
$box_height = 10;
$x = ($width - $box_width) / 2;
$y = ($height - $box_height) / 2;

$box_style = ['fg' => [100, 150, 255]];
$title_style = ['fg' => [255, 200, 0], 'bold' => true];
$text_style = ['fg' => [200, 200, 200]];

// Draw box
tui_draw_box($buf, $x, $y, $box_width, $box_height, $box_style);

// Add title
tui_buffer_write($buf, $x + 2, $y, " My Application ", $title_style);

// Add content
tui_buffer_write($buf, $x + 2, $y + 3, "Welcome to ext-tui!", $text_style);
tui_buffer_write($buf, $x + 2, $y + 5, "Press any key to continue...", $text_style);

tui_render($buf);

// Wait for key
tui_on_key(function($key) {
    tui_stop();
});
tui_run();

tui_buffer_free($buf);
```

---

## Simple Menu

```php
<?php
if (!tui_is_interactive()) {
    die("Interactive terminal required\n");
}

[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

$selected = 0;
$items = [
    'New Project',
    'Open Project',
    'Recent Files',
    'Settings',
    'Exit',
];

function draw() {
    global $buf, $width, $height, $selected, $items;

    tui_buffer_clear($buf);

    $title_style = ['fg' => [255, 200, 100], 'bold' => true];
    $normal_style = ['fg' => [180, 180, 180]];
    $selected_style = ['fg' => [0, 0, 0], 'bg' => [100, 150, 255]];
    $help_style = ['fg' => [100, 100, 100]];

    tui_buffer_write($buf, 5, 2, "Main Menu", $title_style);
    tui_draw_line($buf, 5, 3, 20, 3, '─', $normal_style);

    foreach ($items as $i => $item) {
        $style = ($i === $selected) ? $selected_style : $normal_style;
        $prefix = ($i === $selected) ? ' > ' : '   ';
        $text = $prefix . tui_pad($item, 20);
        tui_buffer_write($buf, 5, 5 + $i, $text, $style);
    }

    tui_buffer_write($buf, 5, $height - 2,
        "↑↓: Navigate | Enter: Select | q: Quit", $help_style);

    tui_render($buf);
}

tui_on_key(function($key) {
    global $selected, $items;

    switch ($key) {
        case 'Up':
            $selected = max(0, $selected - 1);
            break;
        case 'Down':
            $selected = min(count($items) - 1, $selected + 1);
            break;
        case 'Enter':
            if ($items[$selected] === 'Exit') {
                tui_stop();
                return;
            }
            break;
        case 'q':
            tui_stop();
            return;
    }
    draw();
});

draw();
tui_run();
tui_buffer_free($buf);

echo "Selected: {$items[$selected]}\n";
```

---

## Progress Bar Demo

```php
<?php
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

$style = ['fg' => [100, 200, 150]];
$label_style = ['fg' => [200, 200, 200]];

for ($i = 0; $i <= 100; $i += 2) {
    tui_buffer_clear($buf);

    $progress = $i / 100;
    $bar = tui_progress_bar($progress, 50, $style);
    $label = sprintf(" %3d%% Complete", $i);

    tui_buffer_write($buf, 5, 5, "Downloading files...", $label_style);
    tui_buffer_write($buf, 5, 7, $bar . $label, $style);

    tui_render($buf);
    usleep(50000);
}

tui_buffer_write($buf, 5, 9, "Done!", ['fg' => [100, 255, 100], 'bold' => true]);
tui_render($buf);
sleep(1);

tui_buffer_free($buf);
```

---

## Spinner Demo

```php
<?php
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

$spinners = [
    ['style' => TUI_SPINNER_DOTS, 'name' => 'Dots'],
    ['style' => TUI_SPINNER_LINE, 'name' => 'Line'],
    ['style' => TUI_SPINNER_CIRCLE, 'name' => 'Circle'],
    ['style' => TUI_SPINNER_SQUARE, 'name' => 'Square'],
    ['style' => TUI_SPINNER_ARROW, 'name' => 'Arrow'],
];

$frame = 0;
for ($i = 0; $i < 200; $i++) {
    tui_buffer_clear($buf);

    tui_buffer_write($buf, 5, 2, "Spinner Styles", ['bold' => true]);

    foreach ($spinners as $idx => $spinner) {
        $char = tui_spinner($frame, $spinner['style']);
        $text = sprintf("%s %s", $char, $spinner['name']);
        tui_buffer_write($buf, 5, 4 + $idx, $text, []);
    }

    tui_render($buf);
    $frame++;
    usleep(80000);
}

tui_buffer_free($buf);
```

---

## Table Display

```php
<?php
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

$headers = ['Name', 'Role', 'Status'];
$rows = [
    ['Alice Johnson', 'Developer', 'Active'],
    ['Bob Smith', 'Designer', 'Active'],
    ['Carol White', 'Manager', 'Away'],
    ['David Brown', 'Developer', 'Active'],
];

$table = tui_table_create($headers, $rows, [
    'border' => true,
    'header_style' => ['fg' => [255, 200, 100], 'bold' => true],
    'align' => ['left', 'left', 'center'],
]);

$lines = tui_table_render($table, 60);

tui_buffer_write($buf, 5, 2, "Team Members", ['bold' => true]);

foreach ($lines as $i => $line) {
    tui_buffer_write($buf, 5, 4 + $i, $line, []);
}

tui_render($buf);
tui_table_free($table);

sleep(3);
tui_buffer_free($buf);
```

---

## Text Wrapping

```php
<?php
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

$text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. " .
        "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. " .
        "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris.";

$box_width = 40;
$lines = tui_wrap_text($text, $box_width - 4, TUI_WRAP_WORD);

// Draw box
tui_draw_box($buf, 5, 2, $box_width, count($lines) + 4, ['fg' => [150, 150, 200]]);
tui_buffer_write($buf, 7, 2, " Wrapped Text ", ['bold' => true]);

// Display wrapped lines
foreach ($lines as $i => $line) {
    tui_buffer_write($buf, 7, 4 + $i, $line, []);
}

tui_render($buf);
sleep(3);
tui_buffer_free($buf);
```

---

## Unicode and Colors

```php
<?php
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

// Color palette
$colors = [
    [255, 0, 0],     // Red
    [255, 127, 0],   // Orange
    [255, 255, 0],   // Yellow
    [0, 255, 0],     // Green
    [0, 0, 255],     // Blue
    [75, 0, 130],    // Indigo
    [148, 0, 211],   // Violet
];

tui_buffer_write($buf, 5, 2, "Color & Unicode Demo", ['bold' => true]);

// Rainbow blocks
for ($i = 0; $i < count($colors); $i++) {
    $style = ['fg' => $colors[$i]];
    tui_buffer_write($buf, 5 + $i * 4, 4, "████", $style);
}

// Unicode characters
$unicode_examples = [
    "Box drawing: ┌──┬──┐│├──┼──┤└──┴──┘",
    "Blocks: ░▒▓█ ▀▄▌▐",
    "Symbols: ★☆✓✗♠♥♦♣",
    "Arrows: ←↑→↓↔↕",
    "Math: ∞≠≤≥±×÷",
    "CJK: 你好世界",
    "Emoji: 🎉🚀💡",
];

foreach ($unicode_examples as $i => $example) {
    tui_buffer_write($buf, 5, 6 + $i, $example, []);
}

tui_render($buf);
sleep(5);
tui_buffer_free($buf);
```

---

[Back to Documentation](../README.md)
