--TEST--
Xocdr\Tui\Ext\Box handles empty children array correctly
--SKIPIF--
<?php if (!extension_loaded('tui')) die('skip tui extension not loaded'); ?>
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

// Create box with no children
$empty = new Box();
var_dump(is_array($empty->children));
var_dump(count($empty->children));

// Create box with explicit empty children
$explicit = new Box(['children' => []]);
var_dump(is_array($explicit->children));
var_dump(count($explicit->children));

// Add child to empty box
$box = new Box();
$box->addChild(new Text('Hello'));
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
