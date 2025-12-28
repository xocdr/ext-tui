# Layout Guide

ext-tui uses Facebook's Yoga layout engine for CSS-like flexbox positioning.

## Flexbox Basics

### Flex Direction

Controls how children are laid out:

```php
// Vertical stack (default)
$box = new TuiBox(['flexDirection' => 'column']);

// Horizontal stack
$box = new TuiBox(['flexDirection' => 'row']);

// Reverse directions
$box = new TuiBox(['flexDirection' => 'column-reverse']);
$box = new TuiBox(['flexDirection' => 'row-reverse']);
```

### Align Items (Cross Axis)

Controls alignment perpendicular to the main axis:

```php
$box = new TuiBox([
    'flexDirection' => 'row',
    'alignItems' => 'center'  // center, flex-start, flex-end, stretch
]);
```

### Justify Content (Main Axis)

Controls spacing along the main axis:

```php
$box = new TuiBox([
    'flexDirection' => 'row',
    'justifyContent' => 'space-between'
    // flex-start, center, flex-end, space-between, space-around, space-evenly
]);
```

## Sizing

### Fixed Sizes

```php
$box = new TuiBox([
    'width' => 40,   // 40 cells wide
    'height' => 10   // 10 cells tall
]);
```

### Percentage Sizes

```php
$box = new TuiBox([
    'width' => '100%',  // Full width
    'height' => '50%'   // Half height
]);
```

### Flex Grow/Shrink

```php
// This box will grow to fill available space
$box = new TuiBox(['flexGrow' => 1]);

// Multiple boxes with different grow values
$left = new TuiBox(['flexGrow' => 1]);   // 1/3 of space
$right = new TuiBox(['flexGrow' => 2]);  // 2/3 of space
```

## Spacing

### Padding

Space inside the box, between border and content:

```php
// All sides
$box = new TuiBox(['padding' => 2]);

// Individual sides
$box = new TuiBox([
    'paddingTop' => 1,
    'paddingBottom' => 1,
    'paddingLeft' => 2,
    'paddingRight' => 2
]);

// Shorthand
$box = new TuiBox([
    'paddingX' => 2,  // left and right
    'paddingY' => 1   // top and bottom
]);
```

### Margin

Space outside the box:

```php
// All sides
$box = new TuiBox(['margin' => 1]);

// Individual or shorthand (same as padding)
$box = new TuiBox([
    'marginTop' => 2,
    'marginX' => 1
]);
```

### Gap

Space between children:

```php
$box = new TuiBox([
    'flexDirection' => 'column',
    'gap' => 1  // 1 cell between each child
]);
```

## Common Layouts

### Centered Content

```php
$container = new TuiBox([
    'width' => '100%',
    'height' => '100%',
    'alignItems' => 'center',
    'justifyContent' => 'center'
]);
$container->addChild(new TuiText("Centered!"));
```

### Header/Content/Footer

```php
$app = new TuiBox(['height' => '100%']);

$header = new TuiBox(['height' => 3]);
$content = new TuiBox(['flexGrow' => 1]);
$footer = new TuiBox(['height' => 1]);

$app->addChild($header);
$app->addChild($content);
$app->addChild($footer);
```

### Sidebar Layout

```php
$container = new TuiBox([
    'flexDirection' => 'row',
    'height' => '100%'
]);

$sidebar = new TuiBox(['width' => 20]);
$main = new TuiBox(['flexGrow' => 1]);

$container->addChild($sidebar);
$container->addChild($main);
```

### Equal Columns

```php
$row = new TuiBox(['flexDirection' => 'row', 'gap' => 1]);

$col1 = new TuiBox(['flexGrow' => 1]);
$col2 = new TuiBox(['flexGrow' => 1]);
$col3 = new TuiBox(['flexGrow' => 1]);

$row->addChild($col1);
$row->addChild($col2);
$row->addChild($col3);
```
