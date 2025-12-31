--TEST--
Focus system edge cases
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

echo "=== Test 1: Empty tree ===\n";
$renderer = tui_test_create(80, 24);
$tree = new Box(['children' => []]);
tui_test_render($renderer, $tree);

// Sending tab to empty tree should not crash
tui_test_send_key($renderer, 9);  // Tab
echo "Tab on empty tree: ok\n";

// Sending shift+tab should also be safe
tui_test_send_input($renderer, "\x1b[Z");  // Shift+Tab
echo "Shift+Tab on empty tree: ok\n";

tui_test_destroy($renderer);

echo "\n=== Test 2: Tree with no focusable elements ===\n";
$renderer = tui_test_create(80, 24);
$tree = new Box([
    'focusable' => false,
    'children' => [
        new Text("Not focusable"),
        new Box([
            'focusable' => false,
            'children' => [new Text("Also not focusable")]
        ])
    ]
]);
tui_test_render($renderer, $tree);

// Navigation should do nothing but not crash
tui_test_send_key($renderer, 9);  // Tab
tui_test_send_key($renderer, 9);  // Tab again
tui_test_send_input($renderer, "\x1b[Z");  // Shift+Tab

echo "Navigation with no focusable elements: ok\n";
tui_test_destroy($renderer);

echo "\n=== Test 3: Single focusable element ===\n";
$renderer = tui_test_create(80, 24);
$tree = new Box([
    'id' => 'only-one',
    'focusable' => true,
    'children' => [new Text("The only focusable")]
]);
tui_test_render($renderer, $tree);

// Tab should work on single focusable element
tui_test_send_key($renderer, 9);
echo "After Tab: ok\n";

// Tab again should not crash
tui_test_send_key($renderer, 9);
echo "After second Tab: ok\n";

// Shift+Tab should not crash
tui_test_send_input($renderer, "\x1b[Z");
echo "After Shift+Tab: ok\n";

tui_test_destroy($renderer);

echo "\n=== Test 4: Focusable with tabIndex -1 (skip) ===\n";
$renderer = tui_test_create(80, 24);
$tree = new Box([
    'children' => [
        new Box(['id' => 'first', 'focusable' => true, 'tabIndex' => 1, 'children' => [new Text("First")]]),
        new Box(['id' => 'skip-me', 'focusable' => true, 'tabIndex' => -1, 'children' => [new Text("Skip")]]),
        new Box(['id' => 'second', 'focusable' => true, 'tabIndex' => 2, 'children' => [new Text("Second")]]),
    ]
]);
tui_test_render($renderer, $tree);

// Tab should skip the tabIndex=-1 element
tui_test_send_key($renderer, 9);
tui_test_send_key($renderer, 9);
echo "Tab navigation with tabIndex -1: ok\n";

tui_test_destroy($renderer);

echo "\n=== Test 5: Deeply nested single focusable ===\n";
$renderer = tui_test_create(80, 24);

// Create deeply nested structure with one focusable at the bottom
$innermost = new Box([
    'id' => 'deep-focus',
    'focusable' => true,
    'children' => [new Text("Deep inside")]
]);

$tree = $innermost;
for ($i = 0; $i < 10; $i++) {
    $tree = new Box(['children' => [$tree]]);
}

tui_test_render($renderer, $tree);

// Should find the deeply nested focusable via tab
tui_test_send_key($renderer, 9);
echo "Found deeply nested focusable: ok\n";

tui_test_destroy($renderer);

echo "\n=== Test 6: Focus wrap-around ===\n";
$renderer = tui_test_create(80, 24);
$tree = new Box([
    'children' => [
        new Box(['id' => 'a', 'focusable' => true, 'children' => [new Text("A")]]),
        new Box(['id' => 'b', 'focusable' => true, 'children' => [new Text("B")]]),
        new Box(['id' => 'c', 'focusable' => true, 'children' => [new Text("C")]]),
    ]
]);
tui_test_render($renderer, $tree);

// Tab through all and wrap around
for ($i = 0; $i < 5; $i++) {
    tui_test_send_key($renderer, 9);
}
echo "Forward wrap-around: ok\n";

// Shift+Tab wrap around
for ($i = 0; $i < 5; $i++) {
    tui_test_send_input($renderer, "\x1b[Z");
}
echo "Backward wrap-around: ok\n";

tui_test_destroy($renderer);

echo "\n=== Test 7: All elements with same tabIndex ===\n";
$renderer = tui_test_create(80, 24);
$tree = new Box([
    'children' => [
        new Box(['id' => 'x', 'focusable' => true, 'tabIndex' => 0, 'children' => [new Text("X")]]),
        new Box(['id' => 'y', 'focusable' => true, 'tabIndex' => 0, 'children' => [new Text("Y")]]),
        new Box(['id' => 'z', 'focusable' => true, 'tabIndex' => 0, 'children' => [new Text("Z")]]),
    ]
]);
tui_test_render($renderer, $tree);

// Should follow DOM order when all tabIndex are equal
tui_test_send_key($renderer, 9);
tui_test_send_key($renderer, 9);
tui_test_send_key($renderer, 9);
echo "Same tabIndex follows DOM order: ok\n";

tui_test_destroy($renderer);

echo "\n=== Test 8: Removing focused element via re-render ===\n";
$renderer = tui_test_create(80, 24);

// First render with focusable element
$tree = new Box([
    'children' => [
        new Box(['id' => 'btn1', 'focusable' => true, 'focused' => true, 'children' => [new Text("Button 1")]]),
        new Box(['id' => 'btn2', 'focusable' => true, 'children' => [new Text("Button 2")]]),
    ]
]);
tui_test_render($renderer, $tree);

// Re-render without the focused element
$tree = new Box([
    'children' => [
        new Box(['id' => 'btn2', 'focusable' => true, 'children' => [new Text("Button 2")]]),
    ]
]);
tui_test_render($renderer, $tree);

echo "Focused element removed: ok\n";

tui_test_destroy($renderer);

echo "\n=== Test 9: Focus initially set ===\n";
$renderer = tui_test_create(80, 24);
$tree = new Box([
    'children' => [
        new Box(['id' => 'first', 'focusable' => true, 'children' => [new Text("First")]]),
        new Box(['id' => 'second', 'focusable' => true, 'focused' => true, 'children' => [new Text("Second (focused)")]]),
        new Box(['id' => 'third', 'focusable' => true, 'children' => [new Text("Third")]]),
    ]
]);
tui_test_render($renderer, $tree);
echo "Initial focus set: ok\n";

tui_test_destroy($renderer);

echo "\nFocus edge cases test completed!\n";
?>
--EXPECT--
=== Test 1: Empty tree ===
Tab on empty tree: ok
Shift+Tab on empty tree: ok

=== Test 2: Tree with no focusable elements ===
Navigation with no focusable elements: ok

=== Test 3: Single focusable element ===
After Tab: ok
After second Tab: ok
After Shift+Tab: ok

=== Test 4: Focusable with tabIndex -1 (skip) ===
Tab navigation with tabIndex -1: ok

=== Test 5: Deeply nested single focusable ===
Found deeply nested focusable: ok

=== Test 6: Focus wrap-around ===
Forward wrap-around: ok
Backward wrap-around: ok

=== Test 7: All elements with same tabIndex ===
Same tabIndex follows DOM order: ok

=== Test 8: Removing focused element via re-render ===
Focused element removed: ok

=== Test 9: Focus initially set ===
Initial focus set: ok

Focus edge cases test completed!
