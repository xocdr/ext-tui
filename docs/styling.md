# Styling Guide

ext-tui supports rich text styling with colors and text decorations.

## Colors

### Hex Colors

```php
$text = new TuiText("Red text", [
    'color' => '#ff0000'
]);

$text = new TuiText("Blue background", [
    'backgroundColor' => '#0000ff'
]);
```

### RGB Arrays

```php
$text = new TuiText("Green", [
    'color' => [0, 255, 0]
]);
```

### Common Colors

```php
// Red
'color' => '#ff0000'
// Green
'color' => '#00ff00'
// Blue
'color' => '#0000ff'
// Yellow
'color' => '#ffff00'
// Cyan
'color' => '#00ffff'
// Magenta
'color' => '#ff00ff'
// White
'color' => '#ffffff'
// Gray
'color' => '#808080'
```

## Text Styles

### Bold

```php
$text = new TuiText("Important!", ['bold' => true]);
```

### Dim

```php
$text = new TuiText("Secondary info", ['dim' => true]);
```

### Italic

```php
$text = new TuiText("Emphasis", ['italic' => true]);
```

### Underline

```php
$text = new TuiText("Underlined", ['underline' => true]);
```

### Inverse

Swaps foreground and background colors:

```php
$text = new TuiText("Highlighted", ['inverse' => true]);
```

### Strikethrough

```php
$text = new TuiText("Deprecated", ['strikethrough' => true]);
```

### Combined Styles

```php
$text = new TuiText("Bold Red Text", [
    'bold' => true,
    'color' => '#ff0000',
    'underline' => true
]);
```

## Dynamic Styling

Properties can be set after construction:

```php
$text = new TuiText("Hello");

if ($isError) {
    $text->color = '#ff0000';
    $text->bold = true;
}

if ($isSelected) {
    $text->inverse = true;
}
```

## Styling Containers

TuiBox doesn't have text styles but can have border colors:

```php
$box = new TuiBox([
    'borderStyle' => 'single',
    'borderColor' => '#00ff00'
]);
```

Note: Border rendering is planned for a future version.

## Terminal Compatibility

- True color (24-bit RGB) requires a modern terminal
- Most modern terminals support true color: iTerm2, Terminal.app, Windows Terminal, GNOME Terminal, etc.
- Colors will render correctly on any terminal with RGB support
