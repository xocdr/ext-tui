# Components & Layout

ext-tui uses a component-based architecture with flexbox layout powered by Facebook's Yoga engine.

All classes are in the `Xocdr\Tui\Ext` namespace.

## Box

Container component for layout and structure.

### Creating Boxes

```php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

// Empty box with properties
$box = new Box([
    'width' => 40,
    'height' => 10,
    'padding' => 1,
]);

// Box with children
$box = new Box([
    'flexDirection' => 'column',
    'children' => [
        new Text(['content' => 'Item 1']),
        new Text(['content' => 'Item 2']),
    ],
]);

// Adding children dynamically
$box = new Box(['flexDirection' => 'row']);
$box->addChild(new Text(['content' => 'Left']));
$box->addChild(new Text(['content' => 'Right']));
```

### Flex Direction

Control the main axis for child layout:

```php
// Vertical stack (default)
new Box([
    'flexDirection' => 'column',
    'children' => [...],
]);

// Horizontal row
new Box([
    'flexDirection' => 'row',
    'children' => [...],
]);

// Reversed directions
'flexDirection' => 'column-reverse'  // Bottom to top
'flexDirection' => 'row-reverse'     // Right to left
```

### Sizing

```php
// Fixed size
new Box([
    'width' => 40,
    'height' => 10,
]);

// Percentage of parent
new Box([
    'width' => '50%',
    'height' => '100%',
]);

// Flexible growth
new Box([
    'flexGrow' => 1,    // Take available space
    'flexShrink' => 0,  // Don't shrink
]);
```

### Padding & Margin

```php
// All sides
new Box(['padding' => 2, 'margin' => 1]);

// Individual sides
new Box([
    'paddingTop' => 1,
    'paddingBottom' => 1,
    'paddingLeft' => 2,
    'paddingRight' => 2,
]);

// Shorthand (X = horizontal, Y = vertical)
new Box([
    'paddingX' => 2,
    'paddingY' => 1,
]);
```

### Alignment

Control how children are positioned:

```php
// Main axis (along flexDirection)
new Box([
    'justifyContent' => 'flex-start',   // Pack at start (default)
    // 'justifyContent' => 'center',    // Center items
    // 'justifyContent' => 'flex-end',  // Pack at end
    // 'justifyContent' => 'space-between',  // Equal space between
    // 'justifyContent' => 'space-around',   // Equal space around
    // 'justifyContent' => 'space-evenly',   // Equal space including edges
]);

// Cross axis (perpendicular to flexDirection)
new Box([
    'alignItems' => 'stretch',      // Fill available space (default)
    // 'alignItems' => 'flex-start',  // Align at start
    // 'alignItems' => 'center',      // Center items
    // 'alignItems' => 'flex-end',    // Align at end
]);
```

### Gap

Space between children:

```php
new Box([
    'flexDirection' => 'column',
    'gap' => 1,  // 1 row gap between children
    'children' => [
        new Text(['content' => 'Item 1']),
        new Text(['content' => 'Item 2']),
        new Text(['content' => 'Item 3']),
    ],
]);
```

### Borders

```php
new Box([
    'borderStyle' => 'single',  // ┌─┐│└─┘
    // 'borderStyle' => 'double',  // ╔═╗║╚═╝
    // 'borderStyle' => 'round',   // ╭─╮│╰─╯
    // 'borderStyle' => 'bold',    // ┏━┓┃┗━┛
    'borderColor' => [100, 150, 255],
]);
```

## Text

Display styled text content.

### Creating Text

```php
// Simple text
new Text(['content' => 'Hello, World!']);

// Styled text
new Text([
    'content' => 'Important!',
    'bold' => true,
    'color' => [255, 100, 100],
]);
```

### Text Styles

```php
new Text([
    'content' => 'Styled',
    'bold' => true,
    'italic' => true,
    'underline' => true,
    'dim' => true,
    'inverse' => true,
    'strikethrough' => true,
]);
```

### Colors

```php
// RGB array
new Text([
    'content' => 'Red text',
    'color' => [255, 0, 0],
    'backgroundColor' => [50, 0, 0],
]);

// Hex string
new Text([
    'content' => 'Blue text',
    'color' => '#0066ff',
]);
```

## Layout Examples

### Header-Body-Footer

```php
new Box([
    'width' => '100%',
    'height' => '100%',
    'flexDirection' => 'column',
    'children' => [
        // Header
        new Box([
            'height' => 3,
            'borderStyle' => 'single',
            'justifyContent' => 'center',
            'alignItems' => 'center',
            'children' => [
                new Text(['content' => 'My App', 'bold' => true]),
            ],
        ]),
        // Body
        new Box([
            'flexGrow' => 1,
            'padding' => 1,
        ]),
        // Footer
        new Box([
            'height' => 1,
            'children' => [
                new Text(['content' => 'Press q to quit', 'dim' => true]),
            ],
        ]),
    ],
]);
```

### Sidebar Layout

```php
new Box([
    'width' => '100%',
    'height' => '100%',
    'flexDirection' => 'row',
    'children' => [
        // Sidebar
        new Box([
            'width' => 25,
            'borderStyle' => 'single',
            'flexShrink' => 0,
        ]),
        // Main content
        new Box([
            'flexGrow' => 1,
            'padding' => 1,
        ]),
    ],
]);
```

### Centered Content

```php
new Box([
    'width' => '100%',
    'height' => '100%',
    'justifyContent' => 'center',
    'alignItems' => 'center',
    'children' => [
        new Box([
            'padding' => 2,
            'borderStyle' => 'round',
            'children' => [
                new Text(['content' => 'Centered!']),
            ],
        ]),
    ],
]);
```

### List with Selection

```php
$items = ['Apple', 'Banana', 'Cherry'];
$selected = 1;

$children = [];
foreach ($items as $i => $item) {
    $isSelected = $i === $selected;
    $children[] = new Text([
        'content' => ($isSelected ? '> ' : '  ') . $item,
        'color' => $isSelected ? [100, 255, 100] : [200, 200, 200],
        'bold' => $isSelected,
    ]);
}

new Box([
    'flexDirection' => 'column',
    'padding' => 1,
    'borderStyle' => 'single',
    'children' => $children,
]);
```

## Focus Management

Make boxes focusable for tab navigation:

```php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;
use Xocdr\Tui\Ext\Key;

$app = function () {
    return new Box([
        'flexDirection' => 'column',
        'gap' => 1,
        'children' => [
            new Box([
                'focusable' => true,
                'padding' => 1,
                'borderStyle' => 'single',
                'children' => [new Text(['content' => 'Button 1'])],
            ]),
            new Box([
                'focusable' => true,
                'padding' => 1,
                'borderStyle' => 'single',
                'children' => [new Text(['content' => 'Button 2'])],
            ]),
        ],
    ]);
};

$instance = tui_render($app);

tui_set_input_handler($instance, function (Key $key) use ($instance) {
    if ($key->tab) {
        tui_focus_next($instance);
        tui_rerender($instance);
    }
});
```

## See Also

- [Styling](styling.md) - Colors and attributes
- [Input Handling](input.md) - Keyboard events
- [Reference: Classes](../reference/classes.md) - Complete property list
