--TEST--
Kitty graphics parameter validation
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\InvalidDimensionException;
use Xocdr\Tui\ValidationException;

// Test invalid dimensions
try {
    tui_image_create("data", -1, 10, 'rgba');
    echo "Should have thrown\n";
} catch (InvalidDimensionException $e) {
    echo "Caught: dimensions error\n";
}

// Test invalid format
try {
    tui_image_create("data", 10, 10, 'invalid');
    echo "Should have thrown\n";
} catch (ValidationException $e) {
    echo "Caught: format error\n";
}

// Test data length mismatch (10x10 RGBA = 400 bytes, but we pass 5)
try {
    tui_image_create("short", 10, 10, 'rgba');
    echo "Should have thrown\n";
} catch (ValidationException $e) {
    echo "Caught: data length error\n";
}

// Test RGB data length mismatch (2x2 RGB = 12 bytes, but we pass 16)
try {
    tui_image_create(str_repeat("x", 16), 2, 2, 'rgb');
    echo "Should have thrown\n";
} catch (ValidationException $e) {
    echo "Caught: rgb data length error\n";
}

echo "Validation tests passed\n";
?>
--EXPECT--
Caught: dimensions error
Caught: format error
Caught: data length error
Caught: rgb data length error
Validation tests passed
