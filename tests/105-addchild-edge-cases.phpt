--TEST--
Box::addChild() edge cases and error handling
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

echo "=== Add single child ===\n";
$box = new ContainerNode();
$box->addChild(new ContentNode("Child"));
echo "Added 1 child\n";

echo "\n=== Add multiple children sequentially ===\n";
$box = new ContainerNode();
for ($i = 0; $i < 10; $i++) {
    $box->addChild(new ContentNode("Child $i"));
}
echo "Added 10 children\n";

echo "\n=== Add nested boxes ===\n";
$box = new ContainerNode();
$inner = new ContainerNode();
$inner->addChild(new ContentNode("Nested content"));
$box->addChild($inner);
echo "Added nested box\n";

echo "\n=== Add deeply nested structure ===\n";
$root = new ContainerNode();
$current = $root;
for ($i = 0; $i < 50; $i++) {
    $child = new ContainerNode();
    $current->addChild($child);
    $current = $child;
}
$current->addChild(new ContentNode("Deep leaf"));
echo "Added 50 levels of nesting\n";

echo "\n=== Add many children (stress test) ===\n";
$box = new ContainerNode();
for ($i = 0; $i < 500; $i++) {
    $box->addChild(new ContentNode("Child $i"));
}
echo "Added 500 children\n";

echo "\n=== Mixed Box and Text children ===\n";
$box = new ContainerNode();
$box->addChild(new ContentNode("Text 1"));
$box->addChild(new ContainerNode(['children' => [new ContentNode("Nested")]]));
$box->addChild(new ContentNode("Text 2"));
$box->addChild(new ContainerNode());
echo "Added mixed children\n";

echo "\n=== Children with keys ===\n";
$box = new ContainerNode();
$box->addChild(new ContentNode("First", ['key' => 'first']));
$box->addChild(new ContentNode("Second", ['key' => 'second']));
$box->addChild(new ContentNode("Third", ['key' => 'third']));
echo "Added keyed children\n";

echo "\n=== Add same child multiple times ===\n";
$box = new ContainerNode();
$child = new ContentNode("Shared");
$box->addChild($child);
$box->addChild($child);  // Adding same instance again
echo "Added same child twice (should work)\n";

echo "\n=== Add child to multiple parents ===\n";
$child = new ContentNode("Shared child");
$box1 = new ContainerNode();
$box2 = new ContainerNode();
$box1->addChild($child);
$box2->addChild($child);
echo "Added to multiple parents (reference semantics)\n";

echo "\nDone!\n";
?>
--EXPECT--
=== Add single child ===
Added 1 child

=== Add multiple children sequentially ===
Added 10 children

=== Add nested boxes ===
Added nested box

=== Add deeply nested structure ===
Added 50 levels of nesting

=== Add many children (stress test) ===
Added 500 children

=== Mixed Box and Text children ===
Added mixed children

=== Children with keys ===
Added keyed children

=== Add same child multiple times ===
Added same child twice (should work)

=== Add child to multiple parents ===
Added to multiple parents (reference semantics)

Done!
