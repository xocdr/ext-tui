--TEST--
Stress test for memory pool and allocation
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

echo "=== Memory pool stress test ===\n";

// Enable metrics to track allocations
tui_metrics_enable();

$renderer = tui_test_create(80, 24);

// Test 1: Create and destroy many nodes
echo "\n--- Test 1: Node creation/destruction cycles ---\n";

$initialMetrics = tui_get_pool_metrics();
echo "Initial pool allocations: " . ($initialMetrics['allocations'] ?? 0) . "\n";

for ($cycle = 0; $cycle < 10; $cycle++) {
    // Create a tree with many nodes
    $root = new Box([
        'id' => 'cycle-root',
        'width' => 80,
        'height' => 24,
        'flexDirection' => 'column',
    ]);

    for ($i = 0; $i < 50; $i++) {
        $child = new Box([
            'id' => "node-$cycle-$i",
            'height' => 1,
        ]);
        $child->addChild(new Text("Node $i in cycle $cycle"));
        $root->addChild($child);
    }

    tui_test_render($renderer, $root);

    // Let PHP GC clean up
    unset($root);
}

$afterMetrics = tui_get_pool_metrics();
echo "After 10 cycles with 50 nodes each\n";
echo "Total allocations: " . ($afterMetrics['allocations'] ?? 'n/a') . "\n";
echo "Active allocations: " . ($afterMetrics['active'] ?? 'n/a') . "\n";

// Test 2: Deep recursion stress
echo "\n--- Test 2: Deep recursion stress ---\n";

function createDeepTree($depth) {
    if ($depth <= 0) {
        return new Text("Leaf at depth 0");
    }

    $box = new Box([
        'id' => "depth-$depth",
        'padding' => 1,
    ]);
    $box->addChild(createDeepTree($depth - 1));
    return $box;
}

// Test multiple depths
$depths = [10, 25, 50, 75, 100];
foreach ($depths as $depth) {
    $deepRoot = new Box([
        'id' => 'deep-root',
        'width' => 80,
        'height' => 24,
    ]);

    try {
        $deepRoot->addChild(createDeepTree($depth));
        tui_test_render($renderer, $deepRoot);
        echo "Depth $depth: OK\n";
    } catch (Exception $e) {
        echo "Depth $depth: " . $e->getMessage() . "\n";
    }

    unset($deepRoot);
}

// Test 3: Wide tree stress
echo "\n--- Test 3: Wide tree stress ---\n";

$widths = [100, 250, 500, 750, 1000];
foreach ($widths as $width) {
    $wideRoot = new Box([
        'id' => 'wide-root',
        'width' => 80,
        'height' => 24,
        'flexWrap' => 'wrap',
    ]);

    for ($i = 0; $i < $width; $i++) {
        $wideRoot->addChild(new Box(['id' => "w-$i"]));
    }

    $startMem = memory_get_usage();
    tui_test_render($renderer, $wideRoot);
    $endMem = memory_get_usage();

    $memDiff = $endMem - $startMem;
    echo "Width $width: rendered (mem delta: " . number_format($memDiff) . " bytes)\n";

    unset($wideRoot);
}

// Test 4: Rapid allocation/deallocation
echo "\n--- Test 4: Rapid alloc/dealloc ---\n";

$startTime = microtime(true);
$iterations = 1000;

for ($i = 0; $i < $iterations; $i++) {
    $box = new Box(['id' => "rapid-$i"]);
    $box->addChild(new Text("Content $i"));

    // Immediately release
    unset($box);
}

$elapsed = microtime(true) - $startTime;
echo "$iterations rapid iterations in " . number_format($elapsed * 1000, 2) . " ms\n";

// Test 5: Buffer operations stress
echo "\n--- Test 5: Buffer operations stress ---\n";

$bufferSizes = [
    [40, 10],
    [80, 24],
    [120, 40],
    [200, 60],
];

foreach ($bufferSizes as [$width, $height]) {
    try {
        $buffer = tui_buffer_create($width, $height);

        // Draw many elements
        for ($y = 0; $y < $height; $y += 2) {
            tui_draw_line($buffer, 0, $y, $width - 1, $y);
        }
        for ($x = 0; $x < $width; $x += 10) {
            tui_draw_line($buffer, $x, 0, $x, $height - 1);
        }

        // Fill some rectangles
        if ($width > 20 && $height > 10) {
            tui_fill_rect($buffer, 5, 2, 10, 5);
            tui_draw_rect($buffer, 20, 2, 15, 8);
        }

        tui_buffer_clear($buffer);
        echo "Buffer {$width}x{$height}: OK\n";
    } catch (Exception $e) {
        echo "Buffer {$width}x{$height}: " . $e->getMessage() . "\n";
    }
}

// Test 6: String processing stress
echo "\n--- Test 6: String processing stress ---\n";

$testStrings = [
    str_repeat('A', 1000),
    str_repeat('日本語', 100),
    str_repeat("Mixed 中文 Content", 50),
    str_repeat("🎉🎊🎁", 100),
];

foreach ($testStrings as $i => $str) {
    $start = microtime(true);

    for ($j = 0; $j < 100; $j++) {
        tui_string_width($str);
    }

    $elapsed = (microtime(true) - $start) * 1000;
    $strLen = strlen($str);
    echo "String $i ({$strLen} bytes): 100 width calcs in " . number_format($elapsed, 2) . " ms\n";
}

// Test 7: Canvas stress
echo "\n--- Test 7: Canvas stress ---\n";

$canvas = tui_canvas_create(160, 80);

// Draw many points
$points = 5000;
$startTime = microtime(true);

for ($i = 0; $i < $points; $i++) {
    $x = $i % 160;
    $y = (int)($i / 160) % 80;
    tui_canvas_set($canvas, $x, $y);
}

$elapsed = (microtime(true) - $startTime) * 1000;
echo "$points canvas points in " . number_format($elapsed, 2) . " ms\n";

// Draw lines
$lines = 500;
$startTime = microtime(true);

for ($i = 0; $i < $lines; $i++) {
    $x1 = $i % 160;
    $y1 = 0;
    $x2 = 159 - ($i % 160);
    $y2 = 79;
    tui_canvas_line($canvas, $x1, $y1, $x2, $y2);
}

$elapsed = (microtime(true) - $startTime) * 1000;
echo "$lines canvas lines in " . number_format($elapsed, 2) . " ms\n";

// Clear and check
tui_canvas_clear($canvas);
$res = tui_canvas_get_resolution($canvas);
echo "Canvas resolution: {$res['width']}x{$res['height']}\n";

// Test 8: Final memory check
echo "\n--- Test 8: Final memory status ---\n";

$finalMetrics = tui_get_pool_metrics();
echo "Final pool allocations: " . ($finalMetrics['allocations'] ?? 'n/a') . "\n";
echo "Final active: " . ($finalMetrics['active'] ?? 'n/a') . "\n";

$peakMem = memory_get_peak_usage(true);
echo "PHP peak memory: " . number_format($peakMem / 1024 / 1024, 2) . " MB\n";

tui_test_destroy($renderer);
tui_metrics_disable();

echo "\n=== Memory pool stress tests passed ===\n";
?>
--EXPECTF--
=== Memory pool stress test ===

--- Test 1: Node creation/destruction cycles ---
Initial pool allocations: %d
After 10 cycles with 50 nodes each
Total allocations: %s
Active allocations: %s

--- Test 2: Deep recursion stress ---
Depth 10: OK
Depth 25: OK
Depth 50: OK
Depth 75: OK
Depth 100: OK

--- Test 3: Wide tree stress ---
Width 100: rendered (mem delta: %s bytes)
Width 250: rendered (mem delta: %s bytes)
Width 500: rendered (mem delta: %s bytes)
Width 750: rendered (mem delta: %s bytes)
Width 1000: rendered (mem delta: %s bytes)

--- Test 4: Rapid alloc/dealloc ---
1000 rapid iterations in %s ms

--- Test 5: Buffer operations stress ---
Buffer 40x10: OK
Buffer 80x24: OK
Buffer 120x40: OK
Buffer 200x60: OK

--- Test 6: String processing stress ---
String 0 (%d bytes): 100 width calcs in %s ms
String 1 (%d bytes): 100 width calcs in %s ms
String 2 (%d bytes): 100 width calcs in %s ms
String 3 (%d bytes): 100 width calcs in %s ms

--- Test 7: Canvas stress ---
5000 canvas points in %s ms
500 canvas lines in %s ms
Canvas resolution: %dx%d

--- Test 8: Final memory status ---
Final pool allocations: %s
Final active: %s
PHP peak memory: %s MB

=== Memory pool stress tests passed ===
