--TEST--
Security: Fuzz testing node tree operations with edge cases
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Test that node tree operations handle edge cases safely.
 * This tests the C-level node manipulation for robustness.
 */

use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

echo "Test 1: Empty tree operations\n";
$renderer = tui_test_create(80, 24);
$tree = new ContainerNode([]);
tui_test_render($renderer, $tree);
echo "Empty tree: OK\n";
tui_test_destroy($renderer);

echo "\nTest 2: Single node tree\n";
$renderer = tui_test_create(80, 24);
$tree = new ContentNode("hello");
tui_test_render($renderer, $tree);
echo "Single node: OK\n";
tui_test_destroy($renderer);

echo "\nTest 3: Wide tree (many siblings)\n";
$renderer = tui_test_create(80, 24);
$children = [];
for ($i = 0; $i < 500; $i++) {
    $children[] = new ContentNode("child$i");
}
$tree = new ContainerNode(['children' => $children]);
tui_test_render($renderer, $tree);
echo "Wide tree (500 siblings): OK\n";
tui_test_destroy($renderer);

echo "\nTest 4: Deep tree (nested containers)\n";
$renderer = tui_test_create(80, 24);
// Build 100 levels deep (within MAX_TREE_DEPTH of 256)
$tree = new ContentNode("leaf");
for ($i = 0; $i < 100; $i++) {
    $tree = new ContainerNode(['children' => [$tree]]);
}
tui_test_render($renderer, $tree);
echo "Deep tree (100 levels): OK\n";
tui_test_destroy($renderer);

echo "\nTest 5: Mixed wide and deep\n";
$renderer = tui_test_create(80, 24);
$tree = new ContainerNode(['children' => [
    new ContainerNode(['children' => [
        new ContainerNode(['children' => [
            new ContentNode("a"),
            new ContentNode("b"),
            new ContentNode("c"),
        ]]),
        new ContainerNode(['children' => [
            new ContentNode("d"),
            new ContentNode("e"),
        ]]),
    ]]),
    new ContainerNode(['children' => [
        new ContentNode("f"),
        new ContentNode("g"),
    ]]),
]]);
tui_test_render($renderer, $tree);
echo "Mixed tree: OK\n";
tui_test_destroy($renderer);

echo "\nTest 6: Empty text nodes\n";
$renderer = tui_test_create(80, 24);
$tree = new ContainerNode(['children' => [
    new ContentNode(""),
    new ContentNode(""),
    new ContentNode(""),
]]);
tui_test_render($renderer, $tree);
echo "Empty text nodes: OK\n";
tui_test_destroy($renderer);

echo "\nTest 7: Unicode in keys and IDs\n";
$renderer = tui_test_create(80, 24);
$tree = new ContainerNode([
    'key' => 'key-日本語',
    'id' => 'id-émoji-🎉',
    'children' => [
        new ContentNode("content", ['key' => 'text-键']),
    ]
]);
tui_test_render($renderer, $tree);
echo "Unicode keys/IDs: OK\n";
tui_test_destroy($renderer);

echo "\nTest 8: Rapid tree replacement\n";
$renderer = tui_test_create(80, 24);
for ($i = 0; $i < 100; $i++) {
    $tree = new ContainerNode(['children' => [
        new ContentNode("iteration $i"),
    ]]);
    tui_test_render($renderer, $tree);
}
echo "Rapid replacement (100 iterations): OK\n";
tui_test_destroy($renderer);

echo "\nTest 9: Special characters in content\n";
$renderer = tui_test_create(80, 24);
$special = [
    "\x00\x01\x02",       // Control chars
    "\x1b[31mred\x1b[0m", // ANSI escapes
    "line1\nline2",       // Newlines
    "tab\there",          // Tabs
    str_repeat("x", 1000), // Long content
];
foreach ($special as $i => $content) {
    $tree = new ContainerNode(['children' => [new ContentNode($content)]]);
    tui_test_render($renderer, $tree);
}
echo "Special characters: OK\n";
tui_test_destroy($renderer);

echo "\nTest 10: Maximum key/id length (256 bytes)\n";
$renderer = tui_test_create(80, 24);
$maxKey = str_repeat("k", 256);
$maxId = str_repeat("i", 256);
$tree = new ContainerNode([
    'key' => $maxKey,
    'id' => $maxId,
    'children' => [new ContentNode("ok")]
]);
tui_test_render($renderer, $tree);
echo "Max key/id length: OK\n";
tui_test_destroy($renderer);

echo "\nNode tree fuzz test completed safely!\n";
?>
--EXPECTF--
Test 1: Empty tree operations
Empty tree: OK

Test 2: Single node tree
Single node: OK

Test 3: Wide tree (many siblings)
Wide tree (500 siblings): OK

Test 4: Deep tree (nested containers)
Deep tree (100 levels): OK

Test 5: Mixed wide and deep
Mixed tree: OK

Test 6: Empty text nodes
Empty text nodes: OK

Test 7: Unicode in keys and IDs
Unicode keys/IDs: OK

Test 8: Rapid tree replacement
Rapid replacement (100 iterations): OK

Test 9: Special characters in content
Special characters: OK

Test 10: Maximum key/id length (256 bytes)
Max key/id length: OK

Node tree fuzz test completed safely!
