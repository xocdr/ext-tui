--TEST--
Stress test: Wide tree with many siblings (1000+ nodes)
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

// Create a wide tree with many siblings
$width = 1000;
$root = new ContainerNode(['key' => 'root']);

for ($i = 0; $i < $width; $i++) {
    $child = new ContainerNode(['key' => "child-$i"]);
    $child->addChild(new ContentNode("Text $i"));
    $root->addChild($child);
}

echo "Created tree with $width children\n";

// Verify count
$count = count($root->children);
echo "Verified child count: $count\n";

// Access some random children to verify integrity
var_dump($root->children[0]->getKey() === 'child-0');
var_dump($root->children[500]->getKey() === 'child-500');
var_dump($root->children[999]->getKey() === 'child-999');

// Cleanup (tests bulk destruction)
unset($root);
echo "Cleanup successful\n";

echo "Done\n";
?>
--EXPECT--
Created tree with 1000 children
Verified child count: 1000
bool(true)
bool(true)
bool(true)
Cleanup successful
Done
