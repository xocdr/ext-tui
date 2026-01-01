--TEST--
Stress test: Deep tree nesting (100+ levels)
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

// Create a very deep tree (150 levels - well within MAX_TREE_DEPTH of 256)
$depth = 150;
$root = new ContainerNode(['key' => 'root']);
$current = $root;

for ($i = 0; $i < $depth; $i++) {
    $child = new ContainerNode(['key' => "level-$i"]);
    $current->addChild($child);
    $current = $child;
}

// Add a text node at the deepest level
$current->addChild(new ContentNode('Deep text'));

echo "Created tree with depth: $depth\n";

// Verify structure by walking down (ContainerNodes only)
$level = 0;
$node = $root;
while ($node instanceof ContainerNode && $node->children && count($node->children) > 0) {
    $node = $node->children[0];
    $level++;
}
echo "Verified depth by traversal: $level\n";

// Cleanup (tests iterative destruction)
unset($root);
echo "Cleanup successful\n";

echo "Done\n";
?>
--EXPECT--
Created tree with depth: 150
Verified depth by traversal: 151
Cleanup successful
Done
