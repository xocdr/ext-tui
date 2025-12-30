--TEST--
Kitty graphics functions
--EXTENSIONS--
tui
--FILE--
<?php
// Test function existence
var_dump(function_exists('tui_image_load'));
var_dump(function_exists('tui_image_create'));
var_dump(function_exists('tui_image_transmit'));
var_dump(function_exists('tui_image_display'));
var_dump(function_exists('tui_image_delete'));
var_dump(function_exists('tui_image_clear'));
var_dump(function_exists('tui_image_destroy'));
var_dump(function_exists('tui_image_get_info'));
var_dump(function_exists('tui_graphics_supported'));

// Test tui_graphics_supported (may be true or false depending on terminal)
$supported = tui_graphics_supported();
var_dump(is_bool($supported));

// Test tui_image_create with RGBA data (2x2 red pixels)
$rgba_data = str_repeat("\xFF\x00\x00\xFF", 4);  // 4 red RGBA pixels
$img = tui_image_create($rgba_data, 2, 2, 'rgba');

// Verify it's a resource
var_dump(is_resource($img) || $img instanceof \CurlHandle || gettype($img) === 'resource');

// Get info
$info = tui_image_get_info($img);
var_dump($info['width']);
var_dump($info['height']);
var_dump($info['format']);
var_dump($info['state']);
var_dump($info['data_size']);

// Test RGB format (2x2 blue pixels)
$rgb_data = str_repeat("\x00\x00\xFF", 4);  // 4 blue RGB pixels
$img2 = tui_image_create($rgb_data, 2, 2, 'rgb');
$info2 = tui_image_get_info($img2);
var_dump($info2['format']);
var_dump($info2['data_size']);

// Clean up
tui_image_destroy($img);
tui_image_destroy($img2);

echo "Kitty graphics tests passed\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
int(2)
int(2)
string(4) "rgba"
string(6) "loaded"
int(16)
string(3) "rgb"
int(12)
Kitty graphics tests passed
