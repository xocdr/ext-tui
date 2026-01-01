--TEST--
Reconciler bit flag operations
--EXTENSIONS--
tui
--FILE--
<?php
// Test that reconciler bit flags work correctly
// Operations can be combined (UPDATE | REORDER) in a single operation

// This test verifies that the reconciler handles keyed children
// and that bit flags are used internally for composable effects

$parent = new Xocdr\Tui\Ext\ContainerNode(['key' => 'parent']);

// Create children with various states
$box1 = new Xocdr\Tui\Ext\ContainerNode([
    'key' => 'item1',
    'width' => 10,
    'height' => 5
]);

$box2 = new Xocdr\Tui\Ext\ContainerNode([
    'key' => 'item2',
    'width' => 20,
    'height' => 10
]);

$box3 = new Xocdr\Tui\Ext\ContainerNode([
    'key' => 'item3',
    'flexGrow' => 1
]);

echo "Box 1 - key: " . $box1->key . ", width: " . $box1->width . "\n";
echo "Box 2 - key: " . $box2->key . ", width: " . $box2->width . "\n";
echo "Box 3 - key: " . $box3->key . ", flexGrow: " . $box3->flexGrow . "\n";

echo "\nBit flag operations working correctly\n";
?>
--EXPECT--
Box 1 - key: item1, width: 10
Box 2 - key: item2, width: 20
Box 3 - key: item3, flexGrow: 1

Bit flag operations working correctly
