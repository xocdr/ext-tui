# Graphics (Kitty Protocol)

Display images in terminals that support the Kitty graphics protocol.

## Overview

The Kitty graphics protocol allows displaying pixel-perfect images directly in the terminal. This is supported by:

| Terminal | Support |
|----------|---------|
| Kitty | Full |
| WezTerm | Full |
| Konsole | Partial |
| iTerm2 | No (uses different protocol) |
| Apple Terminal | No |

## Checking Support

Always check if the terminal supports graphics before using:

```php
if (!tui_graphics_supported()) {
    echo "This terminal doesn't support Kitty graphics\n";
    exit(1);
}
```

## Loading Images

### From File

Load a PNG image from disk:

```php
$image = tui_image_load('/path/to/image.png');
```

### From Raw Data

Create an image from raw pixel data:

```php
// RGBA format (4 bytes per pixel)
$width = 100;
$height = 100;
$rgba = str_repeat("\xFF\x00\x00\xFF", $width * $height);  // Red pixels
$image = tui_image_create($rgba, $width, $height, 'rgba');

// RGB format (3 bytes per pixel)
$rgb = str_repeat("\x00\xFF\x00", $width * $height);  // Green pixels
$image = tui_image_create($rgb, $width, $height, 'rgb');

// PNG data (already encoded)
$pngData = file_get_contents('image.png');
$image = tui_image_create($pngData, 0, 0, 'png');
```

## Displaying Images

### Basic Display

Display an image at a position:

```php
$image = tui_image_load('logo.png');

// Display at column 0, row 0
tui_image_display($image, 0, 0);
```

### With Size Control

Specify the display size in terminal cells:

```php
// Display as 40 columns × 20 rows
tui_image_display($image, 0, 0, 40, 20);

// Only specify width (height auto-calculated)
tui_image_display($image, 0, 0, 40, 0);

// Only specify height (width auto-calculated)
tui_image_display($image, 0, 0, 0, 20);
```

### Transmission vs Display

Images are transmitted to terminal memory before display. This is automatic, but you can control it:

```php
$image = tui_image_load('large-image.png');

// Explicitly transmit (useful for pre-loading)
tui_image_transmit($image);

// Display at multiple positions (uses cached image)
tui_image_display($image, 0, 0, 20, 10);
tui_image_display($image, 25, 0, 20, 10);
tui_image_display($image, 50, 0, 20, 10);
```

## Image Information

Get metadata about an image:

```php
$image = tui_image_load('photo.png');
$info = tui_image_get_info($image);

echo "Size: {$info['width']}x{$info['height']} pixels\n";
echo "Format: {$info['format']}\n";  // 'png', 'rgb', or 'rgba'
echo "State: {$info['state']}\n";    // 'loaded', 'transmitted', 'displayed'
echo "Data size: {$info['data_size']} bytes\n";
```

## Cleanup

### Delete Single Image

Remove an image from terminal memory:

```php
tui_image_delete($image);
```

### Clear All Images

Remove all images from the terminal:

```php
tui_image_clear();
```

### Destroy Resource

Free the PHP resource (also deletes from terminal):

```php
tui_image_destroy($image);
```

## Complete Example

```php
<?php
// Check support
if (!tui_graphics_supported()) {
    die("Kitty graphics not supported\n");
}

// Enter alternate screen
echo "\033[?1049h";
echo "\033[2J\033[H";  // Clear screen

// Load and display logo
$logo = tui_image_load(__DIR__ . '/logo.png');
tui_image_display($logo, 5, 2, 30, 15);

// Display info
$info = tui_image_get_info($logo);
echo "\033[20;1H";  // Move cursor below image
echo "Image: {$info['width']}x{$info['height']} pixels\n";
echo "Press Enter to exit...";

// Wait for input
fgets(STDIN);

// Cleanup
tui_image_destroy($logo);

// Exit alternate screen
echo "\033[?1049l";
```

## Use Cases

### Splash Screen

```php
if (tui_graphics_supported()) {
    $splash = tui_image_load('splash.png');
    tui_image_display($splash, 0, 0, 80, 24);
    sleep(2);
    tui_image_destroy($splash);
}
```

### Thumbnail Gallery

```php
$images = glob('photos/*.png');
$x = 0;

foreach ($images as $path) {
    $img = tui_image_load($path);
    tui_image_display($img, $x, 0, 15, 10);
    $x += 16;
    // Keep images in memory for scrolling
}
```

### Dynamic Image Generation

```php
// Generate a gradient
$width = 256;
$height = 64;
$data = '';

for ($y = 0; $y < $height; $y++) {
    for ($x = 0; $x < $width; $x++) {
        $data .= chr($x);       // R
        $data .= chr(0);        // G
        $data .= chr(255 - $x); // B
        $data .= chr(255);      // A
    }
}

$gradient = tui_image_create($data, $width, $height, 'rgba');
tui_image_display($gradient, 0, 0, 64, 8);
```

## Limitations

1. **Terminal Support**: Only works in Kitty, WezTerm, and partial Konsole support
2. **No Animation**: Static images only (use multiple images for animation)
3. **Memory**: Large images consume terminal memory; use `tui_image_delete()` when done
4. **SSH**: Works over SSH but transmission is slower due to base64 encoding

## Performance Tips

1. **Pre-transmit**: Call `tui_image_transmit()` before the image is needed
2. **Reuse images**: Transmitted images can be displayed multiple times
3. **Size appropriately**: Specify cell dimensions to avoid terminal-side scaling
4. **Clean up**: Delete unused images to free terminal memory
