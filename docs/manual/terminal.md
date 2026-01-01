# Terminal Control

Control terminal appearance, detect capabilities, and customize window behavior.

## Cursor Control

Control the cursor shape and visibility.

### Cursor Shape

Set the cursor appearance:

```php
// Block cursor (default for most terminals)
tui_cursor_shape('block');

// Blinking block
tui_cursor_shape('block_blink');

// Underline cursor
tui_cursor_shape('underline');

// Blinking underline
tui_cursor_shape('underline_blink');

// Vertical bar (I-beam)
tui_cursor_shape('bar');

// Blinking bar
tui_cursor_shape('bar_blink');

// Reset to terminal default
tui_cursor_shape('default');
```

### Common Use Cases

**Text input mode:**
```php
// Show I-beam cursor for text input
tui_cursor_shape('bar');
```

**Normal/command mode:**
```php
// Show block cursor
tui_cursor_shape('block');
```

**Replace/overwrite mode:**
```php
// Show underline cursor
tui_cursor_shape('underline');
```

### Cursor Visibility

```php
// Hide cursor (useful during rendering)
tui_cursor_hide();

// Show cursor
tui_cursor_show();
```

**Typical pattern:**
```php
// Hide cursor during render to prevent flicker
tui_cursor_hide();
renderContent();
tui_cursor_show();
```

---

## Window Title

Set the terminal window or tab title.

### Setting Title

```php
// Set window title
tui_set_title('My Application');

// Dynamic title updates
tui_set_title('Processing: 50%');

// Clear/reset title
tui_reset_title();
```

### Use Cases

**Show application state:**
```php
function updateTitle(string $state): void
{
    tui_set_title("MyApp - $state");
}

updateTitle('Ready');
// Later...
updateTitle('Saving...');
```

**Show progress:**
```php
function showProgress(float $progress): void
{
    $pct = round($progress * 100);
    tui_set_title("Processing: {$pct}%");
}
```

**Cleanup on exit:**
```php
// Reset title before exiting
register_shutdown_function(function () {
    tui_reset_title();
});
```

---

## Capability Detection

Detect what features the terminal supports for graceful degradation.

### Get All Capabilities

```php
$caps = tui_get_capabilities();

// Terminal identification
echo "Terminal: {$caps['terminal']}\n";      // 'kitty', 'iterm2', 'vte', etc.
echo "Name: {$caps['name']}\n";
echo "Version: {$caps['version']}\n";

// Color support
echo "Color depth: {$caps['color_depth']}\n"; // 8, 256, or 16777216 (24-bit)

// Feature flags
var_dump($caps['capabilities']);
```

### Available Capabilities

| Capability | Description |
|------------|-------------|
| `true_color` | 24-bit RGB color support |
| `256_color` | 256 color palette support |
| `mouse` | Basic mouse tracking |
| `mouse_sgr` | SGR mouse protocol (better coordinates) |
| `bracketed_paste` | Paste detection |
| `clipboard_osc52` | Clipboard read/write via OSC 52 |
| `hyperlinks_osc8` | Clickable hyperlinks |
| `sync_output` | Synchronized output (prevents flicker) |
| `unicode` | Unicode character support |
| `kitty_keyboard` | Extended keyboard protocol |
| `kitty_graphics` | Image display support |
| `sixel` | Sixel graphics support |
| `cursor_shape` | Cursor shape control |
| `title` | Window title control |
| `focus_events` | Focus in/out events |
| `alternate_screen` | Alternate screen buffer |

### Check Specific Capability

```php
if (tui_has_capability('true_color')) {
    // Use 24-bit RGB colors
    $color = [123, 45, 67];
} else {
    // Fall back to ANSI colors
    $color = [255, 0, 0];
}

if (tui_has_capability('kitty_graphics')) {
    // Can display images
    $logo = tui_image_load('logo.png');
    tui_image_display($logo, 0, 0);
} else {
    // Show ASCII art fallback
    echo "[LOGO]";
}
```

### Feature Detection Patterns

**Color support:**
```php
$caps = tui_get_capabilities();

if ($caps['color_depth'] >= 16777216) {
    // True color - use any RGB
    $accent = [100, 150, 255];
} elseif ($caps['color_depth'] >= 256) {
    // 256 colors - use palette
    $accent = [95, 135, 255];  // Close approximation
} else {
    // 8 colors - use basic colors
    $accent = [0, 0, 255];
}
```

**Synchronized output (flicker prevention):**
```php
if (tui_has_capability('sync_output')) {
    // Terminal supports synchronized output
    // ext-tui automatically uses this for rendering
}
```

**Clipboard access:**
```php
if (tui_has_capability('clipboard_osc52')) {
    tui_clipboard_copy($text);
} else {
    // Clipboard not available via terminal
    // Could use pbcopy on macOS, xclip on Linux, etc.
}
```

### Terminal Identification

```php
$caps = tui_get_capabilities();

switch ($caps['terminal']) {
    case 'kitty':
        // Full feature support, including graphics
        break;
    case 'iterm2':
        // Good support, different image protocol
        break;
    case 'wezterm':
        // Full Kitty protocol support
        break;
    case 'vte':
        // GNOME Terminal, Tilix, etc.
        break;
    case 'alacritty':
        // Fast, good basics, limited extras
        break;
    default:
        // Unknown - use safe defaults
        break;
}
```

---

## Terminal Size

Query the terminal dimensions.

```php
// Get terminal size (width, height in characters)
[$width, $height] = tui_get_terminal_size();
echo "Terminal: {$width}x{$height}\n";

// Check if running in a terminal
if (!tui_is_interactive()) {
    die("This program requires an interactive terminal\n");
}

// Check if running in CI
if (tui_is_ci()) {
    // Skip interactive features
}
```

### Size from Instance

```php
$instance = tui_render($app);

$size = tui_get_size($instance);
echo "Width: {$size['width']} chars\n";
echo "Height: {$size['height']} chars\n";
echo "Columns: {$size['columns']}\n";
echo "Rows: {$size['rows']}\n";
```

### Handling Resize

```php
tui_set_resize_handler($instance, function (int $width, int $height) use ($instance) {
    // Terminal was resized
    tui_rerender($instance);
});
```

---

## Complete Example

```php
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;
use Xocdr\Tui\Ext\Key;

// Check capabilities
$caps = tui_get_capabilities();
echo "Terminal: {$caps['terminal']}\n";
echo "True color: " . (tui_has_capability('true_color') ? 'yes' : 'no') . "\n";

if (!tui_is_interactive()) {
    die("Interactive terminal required\n");
}

// Set up terminal
tui_set_title('Capability Demo');
tui_cursor_shape('bar');

$app = function () use ($caps) {
    $features = [];
    foreach ($caps['capabilities'] as $name => $supported) {
        $status = $supported ? '✓' : '✗';
        $features[] = new Text([
            'content' => "$status $name",
            'color' => $supported ? [100, 255, 100] : [255, 100, 100],
        ]);
    }

    return new Box([
        'flexDirection' => 'column',
        'padding' => 1,
        'children' => $features,
    ]);
};

$instance = tui_render($app);

tui_set_input_handler($instance, function (Key $key) use ($instance) {
    if ($key->escape) {
        tui_cursor_shape('default');
        tui_reset_title();
        tui_unmount($instance);
    }
});

tui_wait_until_exit($instance);
```

---

## Terminal Compatibility

| Feature | Kitty | iTerm2 | WezTerm | GNOME Term | Alacritty |
|---------|-------|--------|---------|------------|-----------|
| True Color | ✅ | ✅ | ✅ | ✅ | ✅ |
| Cursor Shape | ✅ | ✅ | ✅ | ✅ | ✅ |
| Window Title | ✅ | ✅ | ✅ | ✅ | ✅ |
| OSC 52 Clipboard | ✅ | ✅ | ✅ | ⚠️ | ⚠️ |
| Hyperlinks | ✅ | ✅ | ✅ | ✅ | ⚠️ |
| Kitty Graphics | ✅ | ❌ | ✅ | ❌ | ❌ |
| Synchronized Output | ✅ | ✅ | ✅ | ✅ | ❌ |

## See Also

- [Graphics](graphics.md) - Kitty graphics protocol
- [Input Handling](input.md) - Clipboard functions
- [Reference: Functions](../reference/functions.md) - Complete API
