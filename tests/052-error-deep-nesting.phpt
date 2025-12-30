--TEST--
Error handling for deeply nested node trees
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

echo "=== Testing tree depth limits ===\n";

// Test moderate nesting (should work fine)
function createNestedBoxes(int $depth): Box {
    if ($depth <= 0) {
        return new Box(['children' => [new Text('leaf')]]);
    }
    return new Box(['children' => [createNestedBoxes($depth - 1)]]);
}

// Test depth 10 - should work
$root = createNestedBoxes(10);
echo "Depth 10: created successfully\n";

// Test depth 50 - should work
$root = createNestedBoxes(50);
echo "Depth 50: created successfully\n";

// Test depth 90 - should work (under default max of 100)
$root = createNestedBoxes(90);
echo "Depth 90: created successfully\n";

// Note: Depths beyond max_tree_depth (default 100) would trigger warnings
// during rendering, not during construction. The tree structure itself
// can be created, but rendering/layout will be limited.

echo "\n=== Testing wide trees ===\n";

// Test many children (wide tree)
$children = [];
for ($i = 0; $i < 100; $i++) {
    $children[] = new Text("child $i");
}
$wideBox = new Box(['children' => $children]);
echo "100 children: created successfully\n";

// Test 1000 children
$children = [];
for ($i = 0; $i < 1000; $i++) {
    $children[] = new Text("child $i");
}
$wideBox = new Box(['children' => $children]);
echo "1000 children: created successfully\n";

echo "\n=== Testing mixed deep and wide ===\n";

// Create a tree that's both deep and wide
function createMixedTree(int $depth, int $width): Box {
    if ($depth <= 0) {
        return new Box(['children' => [new Text('leaf')]]);
    }

    $children = [];
    for ($i = 0; $i < $width; $i++) {
        $children[] = createMixedTree($depth - 1, max(1, $width - 1));
    }
    return new Box(['children' => $children]);
}

// Moderate mixed tree
$root = createMixedTree(3, 3);
echo "Depth 3, Width 3: created successfully\n";

// Deeper but narrower
$root = createMixedTree(10, 2);
echo "Depth 10, Width 2: created successfully\n";

echo "\nAll depth tests completed.\n";
?>
--EXPECT--
=== Testing tree depth limits ===
Depth 10: created successfully
Depth 50: created successfully
Depth 90: created successfully

=== Testing wide trees ===
100 children: created successfully
1000 children: created successfully

=== Testing mixed deep and wide ===
Depth 3, Width 3: created successfully
Depth 10, Width 2: created successfully

All depth tests completed.
