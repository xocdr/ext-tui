--TEST--
Resource cleanup and lifecycle tests
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

echo "=== Buffer lifecycle ===\n";

// Create and destroy
$buffer = tui_buffer_create(80, 24);
echo "Buffer created\n";
// Let it go out of scope - should be cleaned up

// Create multiple and destroy
$buffers = [];
for ($i = 0; $i < 10; $i++) {
    $buffers[] = tui_buffer_create(100, 50);
}
echo "Created 10 buffers\n";
$buffers = [];  // Clear all
echo "Cleared all buffers\n";

echo "\n=== History lifecycle ===\n";

// Create and explicitly destroy
$history = tui_history_create(100);
tui_history_add($history, "command 1");
tui_history_add($history, "command 2");
tui_history_destroy($history);
echo "History destroyed\n";

// Create multiple histories
$histories = [];
for ($i = 0; $i < 5; $i++) {
    $histories[$i] = tui_history_create(50);
    tui_history_add($histories[$i], "entry $i");
}
echo "Created 5 histories\n";

// Destroy in reverse order
for ($i = 4; $i >= 0; $i--) {
    tui_history_destroy($histories[$i]);
}
echo "Destroyed all histories in reverse\n";

echo "\n=== Test renderer lifecycle ===\n";

// Create and destroy
$renderer = tui_test_create(80, 24);
$tree = new ContainerNode(['children' => [new ContentNode("Hello")]]);
tui_test_render($renderer, $tree);
tui_test_destroy($renderer);
echo "Renderer created, used, destroyed\n";

// Multiple renders before destroy
$renderer = tui_test_create(80, 24);
for ($i = 0; $i < 5; $i++) {
    $tree = new ContainerNode(['children' => [new ContentNode("Render $i")]]);
    tui_test_render($renderer, $tree);
}
tui_test_destroy($renderer);
echo "Multiple renders completed\n";

echo "\n=== Node tree lifecycle ===\n";

// Deep tree with many nodes
function createDeepTree($depth) {
    if ($depth <= 0) {
        return new ContentNode("Leaf");
    }
    return new ContainerNode([
        'children' => [
            createDeepTree($depth - 1),
            createDeepTree($depth - 1),
        ]
    ]);
}

$tree = createDeepTree(5);  // 2^5 = 32 leaf nodes
echo "Created tree with depth 5\n";
$tree = null;  // Clear
echo "Cleared deep tree\n";

// Wide tree with many children
$box = new ContainerNode();
for ($i = 0; $i < 100; $i++) {
    $box->addChild(new ContentNode("Child $i"));
}
echo "Created box with 100 children\n";
$box = null;
echo "Cleared wide tree\n";

echo "\n=== Canvas lifecycle ===\n";

$canvas = tui_canvas_create(80, 40);
// Set many pixels
for ($y = 0; $y < 40; $y++) {
    for ($x = 0; $x < 80; $x++) {
        tui_canvas_set($canvas, $x, $y);
    }
}
echo "Canvas created and filled\n";
// Canvas should be cleaned up when going out of scope

echo "\n=== Mixed resource usage ===\n";

// Create various resources together
$buffer = tui_buffer_create(80, 24);
$history = tui_history_create(50);
$renderer = tui_test_create(80, 24);
$canvas = tui_canvas_create(40, 20);

// Use them
tui_buffer_clear($buffer);
tui_history_add($history, "test");
$tree = new ContainerNode(['children' => [new ContentNode("Test")]]);
tui_test_render($renderer, $tree);
tui_canvas_set($canvas, 0, 0);

echo "All resources created and used\n";

// Explicit cleanup
tui_history_destroy($history);
tui_test_destroy($renderer);
echo "Partial cleanup done\n";

echo "\n=== Reuse after operations ===\n";

$buffer = tui_buffer_create(80, 24);

// Draw, clear, draw again
tui_draw_rect($buffer, 0, 0, 20, 10, '#');
tui_buffer_clear($buffer);
tui_draw_circle($buffer, 40, 12, 5, 'o');
$output = tui_buffer_render($buffer);
echo "Buffer reused: " . strlen($output) . " bytes\n";

$history = tui_history_create(10);
// Fill, navigate, add more
for ($i = 0; $i < 5; $i++) {
    tui_history_add($history, "cmd$i");
}
tui_history_prev($history);
tui_history_prev($history);
tui_history_add($history, "new cmd");
tui_history_reset($history);
$last = tui_history_prev($history);
echo "History reused, last: $last\n";
tui_history_destroy($history);

echo "\n=== Stress test cleanup ===\n";

// Create and immediately destroy many times
for ($i = 0; $i < 100; $i++) {
    $h = tui_history_create(10);
    tui_history_add($h, "test");
    tui_history_destroy($h);
}
echo "Created and destroyed 100 histories\n";

// Create many buffers
for ($i = 0; $i < 50; $i++) {
    $b = tui_buffer_create(80, 24);
    tui_buffer_clear($b);
}
echo "Created 50 buffers\n";

// Create many test renderers
for ($i = 0; $i < 20; $i++) {
    $r = tui_test_create(80, 24);
    tui_test_render($r, new ContainerNode());
    tui_test_destroy($r);
}
echo "Created and destroyed 20 renderers\n";

echo "\nDone!\n";
?>
--EXPECTF--
=== Buffer lifecycle ===
Buffer created
Created 10 buffers
Cleared all buffers

=== History lifecycle ===
History destroyed
Created 5 histories
Destroyed all histories in reverse

=== Test renderer lifecycle ===
Renderer created, used, destroyed
Multiple renders completed

=== Node tree lifecycle ===
Created tree with depth 5
Cleared deep tree
Created box with 100 children
Cleared wide tree

=== Canvas lifecycle ===
Canvas created and filled

=== Mixed resource usage ===
All resources created and used
Partial cleanup done

=== Reuse after operations ===
Buffer reused: %d bytes
History reused, last: new cmd

=== Stress test cleanup ===
Created and destroyed 100 histories
Created 50 buffers
Created and destroyed 20 renderers

Done!
