# Styling

Colors, text attributes, and visual styling in ext-tui.

## Colors

### RGB Arrays

Specify colors as `[red, green, blue]` with values 0-255:

```php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

new ContentNode([
    'content' => 'Colored text',
    'color' => [255, 100, 100],           // Foreground
    'backgroundColor' => [50, 0, 0],       // Background
]);

new ContainerNode([
    'borderStyle' => 'single',
    'borderColor' => [100, 150, 255],
]);
```

### Hex Strings

Use standard hex color notation:

```php
use Xocdr\Tui\Ext\ContentNode;

new ContentNode([
    'content' => 'Hex color',
    'color' => '#ff6600',
]);
```

### Color Utilities

```php
// Parse hex to RGB
$rgb = tui_color_from_hex('#ff6600');
// Returns [255, 102, 0]

// Interpolate between colors
$blended = tui_lerp_color([255, 0, 0], [0, 255, 0], 0.5);
// Returns [127, 127, 0]

// Create gradient
$gradient = tui_gradient([255, 0, 0], [0, 0, 255], 10);
// Returns array of 10 colors from red to blue
```

## Text Attributes

### Bold

```php
use Xocdr\Tui\Ext\ContentNode;

new ContentNode([
    'content' => 'Important',
    'bold' => true,
]);
```

### Dim

Reduced intensity text:

```php
new ContentNode([
    'content' => 'Less important',
    'dim' => true,
]);
```

### Italic

```php
new ContentNode([
    'content' => 'Emphasized',
    'italic' => true,
]);
```

### Underline

```php
new ContentNode([
    'content' => 'Linked text',
    'underline' => true,
]);
```

### Inverse

Swap foreground and background:

```php
new ContentNode([
    'content' => 'Selected',
    'inverse' => true,
]);
```

### Strikethrough

```php
new ContentNode([
    'content' => 'Deleted',
    'strikethrough' => true,
]);
```

### Combining Styles

```php
new ContentNode([
    'content' => 'Error!',
    'color' => [255, 100, 100],
    'bold' => true,
    'underline' => true,
]);
```

## Border Styles

Available border styles for Box:

| Style | Characters | Example |
|-------|------------|---------|
| `single` | `─│┌┐└┘` | Box with single lines |
| `double` | `═║╔╗╚╝` | Box with double lines |
| `round` | `─│╭╮╰╯` | Box with rounded corners |
| `bold` | `━┃┏┓┗┛` | Box with thick lines |

```php
use Xocdr\Tui\Ext\ContainerNode;

new ContainerNode([
    'borderStyle' => 'round',
    'borderColor' => [100, 200, 255],
    'children' => [...],
]);
```

## Common Color Schemes

### Success/Error/Warning

```php
$success = [100, 255, 100];  // Green
$error = [255, 100, 100];    // Red
$warning = [255, 200, 0];    // Yellow
$info = [100, 200, 255];     // Blue
```

### Dark Theme

```php
$text = [220, 220, 220];      // Light gray text
$dimText = [128, 128, 128];   // Dim text
$background = [30, 30, 30];   // Dark background
$border = [80, 80, 80];       // Subtle borders
$accent = [100, 150, 255];    // Blue accent
```

### Syntax Highlighting

```php
$keyword = [198, 120, 221];   // Purple
$string = [152, 195, 121];    // Green
$number = [209, 154, 102];    // Orange
$comment = [92, 99, 112];     // Gray
$function = [97, 175, 239];   // Blue
```

## State-Based Styling

Change appearance based on state:

```php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

function renderButton(string $label, bool $focused, bool $disabled): Box
{
    $textColor = $disabled
        ? [100, 100, 100]
        : ($focused ? [0, 0, 0] : [255, 255, 255]);

    $bgColor = $disabled
        ? null
        : ($focused ? [100, 200, 255] : null);

    return new ContainerNode([
        'padding' => 1,
        'borderStyle' => 'single',
        'borderColor' => $focused ? [100, 200, 255] : [100, 100, 100],
        'focusable' => !$disabled,
        'children' => [
            new ContentNode([
                'content' => $label,
                'color' => $textColor,
                'backgroundColor' => $bgColor,
                'dim' => $disabled,
            ]),
        ],
    ]);
}
```

## Hyperlinks (OSC 8)

Make text clickable with terminal hyperlinks:

```php
use Xocdr\Tui\Ext\ContentNode;

// Simple hyperlink
new ContentNode([
    'content' => 'Visit our website',
    'hyperlink' => 'https://example.com',
    'underline' => true,
    'color' => [100, 150, 255],
]);

// With ID for grouping (same ID = same link)
new ContentNode([
    'content' => 'Click here',
    'hyperlink' => [
        'url' => 'https://example.com/docs',
        'id' => 'docs-link',
    ],
]);
```

### How It Works

Hyperlinks use OSC 8 escape sequences:
- `ESC ] 8 ; id=ID ; URL ESC \` - Start hyperlink
- `ESC ] 8 ; ; ESC \` - End hyperlink

The `id` parameter groups multiple text spans as a single link (clicking any part opens the same URL).

### Terminal Support

| Terminal | Support |
|----------|---------|
| iTerm2 | ✅ Full |
| Kitty | ✅ Full |
| GNOME Terminal 3.26+ | ✅ Full |
| Windows Terminal | ✅ Full |
| Alacritty | ⚠️ Limited |
| macOS Terminal.app | ❌ None |

**Tip:** Combine with underline and color to make links visually obvious even in terminals without hyperlink support.

---

## Terminal Compatibility

### True Color Support

ext-tui uses 24-bit RGB colors. Most modern terminals support this:

- ✅ iTerm2
- ✅ Terminal.app (macOS)
- ✅ Kitty
- ✅ Alacritty
- ✅ Windows Terminal
- ⚠️ Older terminals may fall back to 256 colors

### Text Attribute Support

Not all terminals support all attributes:

| Attribute | Support |
|-----------|---------|
| Bold | Universal |
| Dim | Most terminals |
| Italic | Most modern terminals |
| Underline | Universal |
| Inverse | Universal |
| Strikethrough | Some terminals |

## See Also

- [Components](components.md) - Box and Text
- [Animation](animation.md) - Color transitions
- [Reference: Classes](../reference/classes.md) - All properties
