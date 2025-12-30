--TEST--
Error handling for invalid dimensions in canvas/buffer creation
--EXTENSIONS--
tui
--FILE--
<?php
echo "=== Canvas dimension validation ===\n";

// Test valid dimensions
$canvas = tui_canvas_create(10, 10);
echo "canvas(10, 10): " . ($canvas ? "created" : "failed") . "\n";

// Test minimum valid dimensions
$canvas = tui_canvas_create(1, 1);
echo "canvas(1, 1): " . ($canvas ? "created" : "failed") . "\n";

// Test zero dimensions - should fail gracefully
$canvas = @tui_canvas_create(0, 0);
echo "canvas(0, 0): " . ($canvas ? "created (unexpected)" : "failed (expected)") . "\n";

// Test zero width
$canvas = @tui_canvas_create(0, 10);
echo "canvas(0, 10): " . ($canvas ? "created (unexpected)" : "failed (expected)") . "\n";

// Test zero height
$canvas = @tui_canvas_create(10, 0);
echo "canvas(10, 0): " . ($canvas ? "created (unexpected)" : "failed (expected)") . "\n";

// Test large dimensions - canvas may have different limits than buffer
$canvas = @tui_canvas_create(10000, 10000);
echo "canvas(10000, 10000): " . ($canvas ? "created" : "failed") . "\n";

echo "\n=== Box dimension properties ===\n";

// Test Box with valid dimensions
$box = new Xocdr\Tui\Ext\Box(['width' => 10, 'height' => 10]);
echo "Box(10x10): created\n";

// Test Box with zero dimensions
$box = new Xocdr\Tui\Ext\Box(['width' => 0, 'height' => 0]);
echo "Box(0x0): created (dimensions are style hints)\n";

// Test Box with null dimensions (auto-size)
$box = new Xocdr\Tui\Ext\Box(['width' => null, 'height' => null]);
echo "Box(null): created (auto-size)\n";

// Test Box with percentage dimensions
$box = new Xocdr\Tui\Ext\Box(['width' => '50%', 'height' => '100%']);
echo "Box(50%x100%): created\n";

echo "\nAll dimension tests completed.";
?>
--EXPECT--
=== Canvas dimension validation ===
canvas(10, 10): created
canvas(1, 1): created
canvas(0, 0): failed (expected)
canvas(0, 10): failed (expected)
canvas(10, 0): failed (expected)
canvas(10000, 10000): created

=== Box dimension properties ===
Box(10x10): created
Box(0x0): created (dimensions are style hints)
Box(null): created (auto-size)
Box(50%x100%): created

All dimension tests completed.