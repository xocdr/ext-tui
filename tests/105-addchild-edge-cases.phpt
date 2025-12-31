--TEST--
Box::addChild() edge cases and error handling
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

echo "=== Add single child ===\n";
$box = new Box();
$box->addChild(new Text("Child"));
echo "Added 1 child\n";

echo "\n=== Add multiple children sequentially ===\n";
$box = new Box();
for ($i = 0; $i < 10; $i++) {
    $box->addChild(new Text("Child $i"));
}
echo "Added 10 children\n";

echo "\n=== Add nested boxes ===\n";
$box = new Box();
$inner = new Box();
$inner->addChild(new Text("Nested content"));
$box->addChild($inner);
echo "Added nested box\n";

echo "\n=== Add deeply nested structure ===\n";
$root = new Box();
$current = $root;
for ($i = 0; $i < 50; $i++) {
    $child = new Box();
    $current->addChild($child);
    $current = $child;
}
$current->addChild(new Text("Deep leaf"));
echo "Added 50 levels of nesting\n";

echo "\n=== Add many children (stress test) ===\n";
$box = new Box();
for ($i = 0; $i < 500; $i++) {
    $box->addChild(new Text("Child $i"));
}
echo "Added 500 children\n";

echo "\n=== Mixed Box and Text children ===\n";
$box = new Box();
$box->addChild(new Text("Text 1"));
$box->addChild(new Box(['children' => [new Text("Nested")]]));
$box->addChild(new Text("Text 2"));
$box->addChild(new Box());
echo "Added mixed children\n";

echo "\n=== Children with keys ===\n";
$box = new Box();
$box->addChild(new Text("First", ['key' => 'first']));
$box->addChild(new Text("Second", ['key' => 'second']));
$box->addChild(new Text("Third", ['key' => 'third']));
echo "Added keyed children\n";

echo "\n=== Add same child multiple times ===\n";
$box = new Box();
$child = new Text("Shared");
$box->addChild($child);
$box->addChild($child);  // Adding same instance again
echo "Added same child twice (should work)\n";

echo "\n=== Add child to multiple parents ===\n";
$child = new Text("Shared child");
$box1 = new Box();
$box2 = new Box();
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
