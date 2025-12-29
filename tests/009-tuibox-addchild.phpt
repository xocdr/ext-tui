--TEST--
Xocdr\Tui\Ext\Box::addChild() adds children
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

$box = new Box();
$text = new Text("Hello");

$result = $box->addChild($text);

// Should return self for chaining
var_dump($result === $box);

// Should have 1 child
var_dump(count($box->children));
?>
--EXPECT--
bool(true)
int(1)
