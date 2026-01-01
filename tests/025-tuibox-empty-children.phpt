--TEST--
Xocdr\Tui\Ext\ContainerNode handles empty children array correctly
--SKIPIF--
<?php if (!extension_loaded('tui')) die('skip tui extension not loaded'); ?>
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

// Create box with no children
$empty = new ContainerNode();
var_dump(is_array($empty->children));
var_dump(count($empty->children));

// Create box with explicit empty children
$explicit = new ContainerNode(['children' => []]);
var_dump(is_array($explicit->children));
var_dump(count($explicit->children));

// Add child to empty box
$box = new ContainerNode();
$box->addChild(new ContentNode('Hello'));
var_dump(count($box->children));

echo "Empty children handled correctly\n";
?>
--EXPECT--
bool(true)
int(0)
bool(true)
int(0)
int(1)
Empty children handled correctly
