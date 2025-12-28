# Components & Layout

ext-tui uses a component-based architecture with flexbox layout powered by Facebook's Yoga engine.

## TuiBox

Container component for layout and structure.

### Creating Boxes

```php
// Empty box with properties
$box = new TuiBox([
    'width' => 40,
    'height' => 10,
    'padding' => 1,
]);

// Box with children
$box = new TuiBox([
    'flexDirection' => 'column',
    'children' => [
        new TuiText(['content' => 'Item 1']),
        new TuiText(['content' => 'Item 2']),
    ],
]);

// Adding children dynamically
$box = new TuiBox(['flexDirection' => 'row']);
$box->addChild(new TuiText(['content' => 'Left']));
$box->addChild(new TuiText(['content' => 'Right']));
```

### Flex Direction

Control the main axis for child layout:

```php
// Vertical stack (default)
new TuiBox([
    'flexDirection' => 'column',
    'children' => [...],
]);

// Horizontal row
new TuiBox([
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
new TuiBox([
    'width' => 40,
    'height' => 10,
]);

// Percentage of parent
new TuiBox([
    'width' => '50%',
    'height' => '100%',
]);

// Flexible growth
new TuiBox([
    'flexGrow' => 1,    // Take available space
    'flexShrink' => 0,  // Don't shrink
]);
```

### Padding & Margin

```php
// All sides
new TuiBox(['padding' => 2, 'margin' => 1]);

// Individual sides
new TuiBox([
    'paddingTop' => 1,
    'paddingBottom' => 1,
    'paddingLeft' => 2,
    'paddingRight' => 2,
]);

// Shorthand (X = horizontal, Y = vertical)
new TuiBox([
    'paddingX' => 2,
    'paddingY' => 1,
]);
```

### Alignment

Control how children are positioned:

```php
// Main axis (along flexDirection)
new TuiBox([
    'justifyContent' => 'flex-start',   // Pack at start (default)
    // 'justifyContent' => 'center',    // Center items
    // 'justifyContent' => 'flex-end',  // Pack at end
    // 'justifyContent' => 'space-between',  // Equal space between
    // 'justifyContent' => 'space-around',   // Equal space around
    // 'justifyContent' => 'space-evenly',   // Equal space including edges
]);

// Cross axis (perpendicular to flexDirection)
new TuiBox([
    'alignItems' => 'stretch',      // Fill available space (default)
    // 'alignItems' => 'flex-start',  // Align at start
    // 'alignItems' => 'center',      // Center items
    // 'alignItems' => 'flex-end',    // Align at end
]);
```

### Gap

Space between children:

```php
new TuiBox([
    'flexDirection' => 'column',
    'gap' => 1,  // 1 row gap between children
    'children' => [
        new TuiText(['content' => 'Item 1']),
        new TuiText(['content' => 'Item 2']),
        new TuiText(['content' => 'Item 3']),
    ],
]);
```

### Borders

```php
new TuiBox([
    'borderStyle' => 'single',  // ┌─┐│└─┘
    // 'borderStyle' => 'double',  // ╔═╗║╚═╝
    // 'borderStyle' => 'round',   // ╭─╮│╰─╯
    // 'borderStyle' => 'bold',    // ┏━┓┃┗━┛
    'borderColor' => [100, 150, 255],
]);
```

## TuiText

Display styled text content.

### Creating Text

```php
// Simple text
new TuiText(['content' => 'Hello, World!']);

// Styled text
new TuiText([
    'content' => 'Important!',
    'bold' => true,
    'color' => [255, 100, 100],
]);
```

### Text Styles

```php
new TuiText([
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
new TuiText([
    'content' => 'Red text',
    'color' => [255, 0, 0],
    'backgroundColor' => [50, 0, 0],
]);

// Hex string
new TuiText([
    'content' => 'Blue text',
    'color' => '#0066ff',
]);
```

## Layout Examples

### Header-Body-Footer

```php
new TuiBox([
    'width' => '100%',
    'height' => '100%',
    'flexDirection' => 'column',
    'children' => [
        // Header
        new TuiBox([
            'height' => 3,
            'borderStyle' => 'single',
            'justifyContent' => 'center',
            'alignItems' => 'center',
            'children' => [
                new TuiText(['content' => 'My App', 'bold' => true]),
            ],
        ]),
        // Body
        new TuiBox([
            'flexGrow' => 1,
            'padding' => 1,
        ]),
        // Footer
        new TuiBox([
            'height' => 1,
            'children' => [
                new TuiText(['content' => 'Press q to quit', 'dim' => true]),
            ],
        ]),
    ],
]);
```

### Sidebar Layout

```php
new TuiBox([
    'width' => '100%',
    'height' => '100%',
    'flexDirection' => 'row',
    'children' => [
        // Sidebar
        new TuiBox([
            'width' => 25,
            'borderStyle' => 'single',
            'flexShrink' => 0,
        ]),
        // Main content
        new TuiBox([
            'flexGrow' => 1,
            'padding' => 1,
        ]),
    ],
]);
```

### Centered Content

```php
new TuiBox([
    'width' => '100%',
    'height' => '100%',
    'justifyContent' => 'center',
    'alignItems' => 'center',
    'children' => [
        new TuiBox([
            'padding' => 2,
            'borderStyle' => 'round',
            'children' => [
                new TuiText(['content' => 'Centered!']),
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
    $children[] = new TuiText([
        'content' => ($isSelected ? '> ' : '  ') . $item,
        'color' => $isSelected ? [100, 255, 100] : [200, 200, 200],
        'bold' => $isSelected,
    ]);
}

new TuiBox([
    'flexDirection' => 'column',
    'padding' => 1,
    'borderStyle' => 'single',
    'children' => $children,
]);
```

## Focus Management

Make boxes focusable for tab navigation:

```php
$app = function () {
    return new TuiBox([
        'flexDirection' => 'column',
        'gap' => 1,
        'children' => [
            new TuiBox([
                'focusable' => true,
                'padding' => 1,
                'borderStyle' => 'single',
                'children' => [new TuiText(['content' => 'Button 1'])],
            ]),
            new TuiBox([
                'focusable' => true,
                'padding' => 1,
                'borderStyle' => 'single',
                'children' => [new TuiText(['content' => 'Button 2'])],
            ]),
        ],
    ]);
};

$instance = tui_render($app);

tui_set_input_handler($instance, function (TuiKey $key) use ($instance) {
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
