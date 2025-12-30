--TEST--
Cursor shape control functions
--EXTENSIONS--
tui
--FILE--
<?php
// Test cursor shape function exists and accepts valid shapes
$shapes = ['default', 'block', 'block_blink', 'underline', 'underline_blink', 'bar', 'bar_blink'];

foreach ($shapes as $shape) {
    tui_cursor_shape($shape);
}
echo "All shapes: OK\n";

// Test cursor show/hide
tui_cursor_hide();
tui_cursor_show();
echo "cursor_hide/show: OK\n";

// Test invalid shape throws
try {
    tui_cursor_shape('invalid');
    echo "Should have thrown\n";
} catch (Exception $e) {
    echo "Invalid shape throws: OK\n";
}
?>
--EXPECTREGEX--
.*All shapes: OK
.*cursor_hide\/show: OK
Invalid shape throws: OK
