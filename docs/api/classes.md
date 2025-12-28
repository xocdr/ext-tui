# API Reference: Classes

[Back to Documentation](../README.md)

---

ext-tui provides classes for flexbox layouts using the Yoga layout engine.

## TuiBox

A flexbox container for layout purposes.

### Constructor

```php
public function __construct()
```

Creates a new box element with default Yoga node.

### Methods

#### appendChild

```php
public function appendChild(TuiBox|TuiText $child): void
```

Add a child element to this box.

#### removeChild

```php
public function removeChild(TuiBox|TuiText $child): void
```

Remove a child element from this box.

#### setWidth / setHeight

```php
public function setWidth(float|string $value): void
public function setHeight(float|string $value): void
```

Set dimensions. Use float for pixels or string with `%` for percentage.

```php
$box->setWidth(100);      // 100 units
$box->setWidth('50%');    // 50% of parent
```

#### setMinWidth / setMinHeight / setMaxWidth / setMaxHeight

```php
public function setMinWidth(float $value): void
public function setMinHeight(float $value): void
public function setMaxWidth(float $value): void
public function setMaxHeight(float $value): void
```

Set size constraints.

#### setFlexDirection

```php
public function setFlexDirection(string $direction): void
```

Set the main axis direction.

| Value | Description |
|-------|-------------|
| `'row'` | Left to right (default) |
| `'column'` | Top to bottom |
| `'row-reverse'` | Right to left |
| `'column-reverse'` | Bottom to top |

#### setFlexGrow / setFlexShrink / setFlexBasis

```php
public function setFlexGrow(float $value): void
public function setFlexShrink(float $value): void
public function setFlexBasis(float $value): void
```

Set flex properties.

```php
$sidebar->setWidth(200);
$sidebar->setFlexShrink(0);  // Don't shrink

$content->setFlexGrow(1);    // Take remaining space
```

#### setJustifyContent

```php
public function setJustifyContent(string $value): void
```

Align children along main axis.

| Value | Description |
|-------|-------------|
| `'flex-start'` | Pack at start (default) |
| `'flex-end'` | Pack at end |
| `'center'` | Center items |
| `'space-between'` | Equal space between |
| `'space-around'` | Equal space around |
| `'space-evenly'` | Equal space including edges |

#### setAlignItems

```php
public function setAlignItems(string $value): void
```

Align children along cross axis.

| Value | Description |
|-------|-------------|
| `'stretch'` | Stretch to fill (default) |
| `'flex-start'` | Align at start |
| `'flex-end'` | Align at end |
| `'center'` | Center items |

#### setPadding

```php
public function setPadding(float $all): void
public function setPaddingTop(float $value): void
public function setPaddingBottom(float $value): void
public function setPaddingLeft(float $value): void
public function setPaddingRight(float $value): void
```

Set internal padding.

#### setMargin

```php
public function setMargin(float $all): void
public function setMarginTop(float $value): void
public function setMarginBottom(float $value): void
public function setMarginLeft(float $value): void
public function setMarginRight(float $value): void
```

Set external margin.

#### setGap

```php
public function setGap(float $all): void
public function setRowGap(float $value): void
public function setColumnGap(float $value): void
```

Set gap between children.

#### setBorderWidth

```php
public function setBorderWidth(float $all): void
public function setBorderTop(float $value): void
public function setBorderBottom(float $value): void
public function setBorderLeft(float $value): void
public function setBorderRight(float $value): void
```

Set border width (for layout calculation only).

#### calculateLayout

```php
public function calculateLayout(): void
```

Calculate layout for this node and all descendants.

#### getComputedLayout

```php
public function getComputedLayout(): array
```

Get computed layout after `calculateLayout()`.

Returns:
```php
[
    'x' => float,      // Left position
    'y' => float,      // Top position
    'width' => float,  // Computed width
    'height' => float, // Computed height
]
```

### Example

```php
<?php
[$width, $height] = tui_get_terminal_size();

// Create layout
$root = new TuiBox();
$root->setWidth($width);
$root->setHeight($height);
$root->setFlexDirection('row');
$root->setPadding(1);

// Sidebar
$sidebar = new TuiBox();
$sidebar->setWidth(20);
$sidebar->setFlexShrink(0);
$root->appendChild($sidebar);

// Content
$content = new TuiBox();
$content->setFlexGrow(1);
$content->setPadding(1);
$root->appendChild($content);

// Calculate
$root->calculateLayout();

// Use computed positions
$sidebar_layout = $sidebar->getComputedLayout();
tui_draw_box($buf,
    (int)$sidebar_layout['x'],
    (int)$sidebar_layout['y'],
    (int)$sidebar_layout['width'],
    (int)$sidebar_layout['height'],
    $style
);
```

---

## TuiText

A text element for layout purposes.

### Constructor

```php
public function __construct(string $text = '')
```

Creates a text node with optional initial text.

### Methods

#### setText

```php
public function setText(string $text): void
```

Set the text content.

#### getText

```php
public function getText(): string
```

Get the text content.

### Notes

- Text nodes automatically measure their content
- Width is calculated using `tui_string_width()`
- Height is calculated from line count

### Example

```php
$label = new TuiText("Hello, World!");
$container->appendChild($label);

// After calculateLayout()
$layout = $label->getComputedLayout();
tui_buffer_write($buf,
    (int)$layout['x'],
    (int)$layout['y'],
    $label->getText(),
    $style
);
```

---

## TuiInstance

Represents an ext-tui application instance.

### Constructor

```php
public function __construct()
```

Creates a new TUI instance.

### Methods

#### run

```php
public function run(): void
```

Start the application.

#### stop

```php
public function stop(): void
```

Stop the application.

#### getWidth / getHeight

```php
public function getWidth(): int
public function getHeight(): int
```

Get terminal dimensions.

### Example

```php
$app = new TuiInstance();
// ... setup ...
$app->run();
```

---

## Complete Layout Example

```php
<?php
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

// Build layout tree
$root = new TuiBox();
$root->setWidth($width);
$root->setHeight($height);
$root->setFlexDirection('column');

// Header
$header = new TuiBox();
$header->setHeight(3);
$header->setFlexShrink(0);
$root->appendChild($header);

// Body (row with sidebar + content)
$body = new TuiBox();
$body->setFlexDirection('row');
$body->setFlexGrow(1);
$root->appendChild($body);

// Sidebar
$sidebar = new TuiBox();
$sidebar->setWidth(25);
$sidebar->setFlexShrink(0);
$body->appendChild($sidebar);

// Main content
$main = new TuiBox();
$main->setFlexGrow(1);
$body->appendChild($main);

// Footer
$footer = new TuiBox();
$footer->setHeight(1);
$footer->setFlexShrink(0);
$root->appendChild($footer);

// Calculate all layouts
$root->calculateLayout();

// Render each component
function draw_component($buf, $box, $title, $style) {
    $layout = $box->getComputedLayout();
    $x = (int)$layout['x'];
    $y = (int)$layout['y'];
    $w = (int)$layout['width'];
    $h = (int)$layout['height'];

    if ($w > 0 && $h > 0) {
        tui_draw_box($buf, $x, $y, $w, $h, $style);
        if ($title && $w > 4) {
            tui_buffer_write($buf, $x + 2, $y, " $title ", $style);
        }
    }
}

draw_component($buf, $header, "Header", ['fg' => [255, 200, 0]]);
draw_component($buf, $sidebar, "Sidebar", ['fg' => [100, 200, 255]]);
draw_component($buf, $main, "Content", ['fg' => [100, 255, 100]]);
draw_component($buf, $footer, null, ['fg' => [150, 150, 150]]);

tui_render($buf);
```

---

[Back to Documentation](../README.md)
