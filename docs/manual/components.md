# Components & Layout

ext-tui uses a component-based architecture with flexbox layout powered by Facebook's Yoga engine.

All classes are in the `Xocdr\Tui\Ext` namespace.

## ContainerNode

Container component for layout and structure.

### Creating Containers

```php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

// Empty container with properties
$box = new ContainerNode([
    'width' => 40,
    'height' => 10,
    'padding' => 1,
]);

// Adding children dynamically
$box = new ContainerNode(['flexDirection' => 'row']);
$box->addChild(new ContentNode(['content' => 'Left']));
$box->addChild(new ContentNode(['content' => 'Right']));
```

### Flex Direction

Control the main axis for child layout:

```php
// Vertical stack (default)
new ContainerNode(['flexDirection' => 'column']);

// Horizontal row
new ContainerNode(['flexDirection' => 'row']);

// Reversed directions
'flexDirection' => 'column-reverse'  // Bottom to top
'flexDirection' => 'row-reverse'     // Right to left
```

### Sizing

```php
// Fixed size
new ContainerNode([
    'width' => 40,
    'height' => 10,
]);

// Percentage of parent
new ContainerNode([
    'width' => '50%',
    'height' => '100%',
]);

// Flexible growth
new ContainerNode([
    'flexGrow' => 1,    // Take available space
    'flexShrink' => 0,  // Don't shrink
]);
```

### Padding & Margin

```php
// All sides
new ContainerNode(['padding' => 2, 'margin' => 1]);

// Individual sides
new ContainerNode([
    'paddingTop' => 1,
    'paddingBottom' => 1,
    'paddingLeft' => 2,
    'paddingRight' => 2,
]);

// Shorthand (X = horizontal, Y = vertical)
new ContainerNode([
    'paddingX' => 2,
    'paddingY' => 1,
]);
```

### Alignment

Control how children are positioned:

```php
// Main axis (along flexDirection)
new ContainerNode([
    'justifyContent' => 'flex-start',   // Pack at start (default)
    // 'justifyContent' => 'center',    // Center items
    // 'justifyContent' => 'flex-end',  // Pack at end
    // 'justifyContent' => 'space-between',  // Equal space between
    // 'justifyContent' => 'space-around',   // Equal space around
    // 'justifyContent' => 'space-evenly',   // Equal space including edges
]);

// Cross axis (perpendicular to flexDirection)
new ContainerNode([
    'alignItems' => 'stretch',      // Fill available space (default)
    // 'alignItems' => 'flex-start',  // Align at start
    // 'alignItems' => 'center',      // Center items
    // 'alignItems' => 'flex-end',    // Align at end
]);
```

### Gap

Space between children:

```php
$box = new ContainerNode([
    'flexDirection' => 'column',
    'gap' => 1,  // 1 row gap between children
]);
$box->addChild(new ContentNode(['content' => 'Item 1']));
$box->addChild(new ContentNode(['content' => 'Item 2']));
$box->addChild(new ContentNode(['content' => 'Item 3']));
```

### Borders

```php
new ContainerNode([
    'borderStyle' => 'single',  // ┌─┐│└─┘
    // 'borderStyle' => 'double',  // ╔═╗║╚═╝
    // 'borderStyle' => 'round',   // ╭─╮│╰─╯
    // 'borderStyle' => 'bold',    // ┏━┓┃┗━┛
    'borderColor' => [100, 150, 255],
]);
```

## ContentNode

Display styled text content.

### Creating Text

```php
// Simple text
new ContentNode(['content' => 'Hello, World!']);

// Styled text
new ContentNode([
    'content' => 'Important!',
    'bold' => true,
    'color' => [255, 100, 100],
]);
```

### Text Styles

```php
new ContentNode([
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
new ContentNode([
    'content' => 'Red text',
    'color' => [255, 0, 0],
    'backgroundColor' => [50, 0, 0],
]);

// Hex string
new ContentNode([
    'content' => 'Blue text',
    'color' => '#0066ff',
]);
```

## Layout Examples

### Header-Body-Footer

```php
$app = new ContainerNode([
    'width' => '100%',
    'height' => '100%',
    'flexDirection' => 'column',
]);

// Header
$header = new ContainerNode([
    'height' => 3,
    'borderStyle' => 'single',
    'justifyContent' => 'center',
    'alignItems' => 'center',
]);
$header->addChild(new ContentNode(['content' => 'My App', 'bold' => true]));
$app->addChild($header);

// Body
$app->addChild(new ContainerNode(['flexGrow' => 1, 'padding' => 1]));

// Footer
$footer = new ContainerNode(['height' => 1]);
$footer->addChild(new ContentNode(['content' => 'Press q to quit', 'dim' => true]));
$app->addChild($footer);
```

### Sidebar Layout

```php
$app = new ContainerNode([
    'width' => '100%',
    'height' => '100%',
    'flexDirection' => 'row',
]);

// Sidebar
$app->addChild(new ContainerNode([
    'width' => 25,
    'borderStyle' => 'single',
    'flexShrink' => 0,
]));

// Main content
$app->addChild(new ContainerNode(['flexGrow' => 1, 'padding' => 1]));
```

### Centered Content

```php
$outer = new ContainerNode([
    'width' => '100%',
    'height' => '100%',
    'justifyContent' => 'center',
    'alignItems' => 'center',
]);

$inner = new ContainerNode(['padding' => 2, 'borderStyle' => 'round']);
$inner->addChild(new ContentNode(['content' => 'Centered!']));
$outer->addChild($inner);
```

### List with Selection

```php
$items = ['Apple', 'Banana', 'Cherry'];
$selected = 1;

$box = new ContainerNode([
    'flexDirection' => 'column',
    'padding' => 1,
    'borderStyle' => 'single',
]);

foreach ($items as $i => $item) {
    $isSelected = $i === $selected;
    $box->addChild(new ContentNode([
        'content' => ($isSelected ? '> ' : '  ') . $item,
        'color' => $isSelected ? [100, 255, 100] : [200, 200, 200],
        'bold' => $isSelected,
    ]));
}
```

## Focus Management

Make containers focusable for tab navigation:

```php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;
use Xocdr\Tui\Ext\Key;

$app = function () {
    $box = new ContainerNode(['flexDirection' => 'column', 'gap' => 1]);

    $btn1 = new ContainerNode(['focusable' => true, 'padding' => 1, 'borderStyle' => 'single']);
    $btn1->addChild(new ContentNode(['content' => 'Button 1']));
    $box->addChild($btn1);

    $btn2 = new ContainerNode(['focusable' => true, 'padding' => 1, 'borderStyle' => 'single']);
    $btn2->addChild(new ContentNode(['content' => 'Button 2']));
    $box->addChild($btn2);

    return $box;
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
