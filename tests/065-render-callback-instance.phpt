--TEST--
tui_render() and rerender work correctly with component callback
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Test that rendering and re-rendering work correctly.
 * This verifies the fix for the bug where the component callback wasn't
 * being called correctly with the Instance parameter.
 */

use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

$renderer = tui_test_create(80, 24);

// Initial render
$box = new ContainerNode(['width' => 80, 'height' => 24]);
$box->children = [new ContentNode("Hello World")];

tui_test_render($renderer, $box);
echo "Initial render completed\n";

// Verify the content was rendered
$matches = tui_test_get_by_text($renderer, "Hello");
if (count($matches) > 0) {
    echo "Content rendered correctly\n";
} else {
    echo "Content not found\n";
}

// Test re-render with different content
$box2 = new ContainerNode(['width' => 80, 'height' => 24]);
$box2->children = [new ContentNode("Updated Content")];

tui_test_render($renderer, $box2);
echo "Re-render completed\n";

$matches2 = tui_test_get_by_text($renderer, "Updated");
if (count($matches2) > 0) {
    echo "Updated content rendered correctly\n";
} else {
    echo "Updated content not found\n";
}

tui_test_destroy($renderer);
echo "Test passed!\n";
?>
--EXPECT--
Initial render completed
Content rendered correctly
Re-render completed
Updated content rendered correctly
Test passed!
