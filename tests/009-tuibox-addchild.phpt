--TEST--
Xocdr\Tui\Ext\ContainerNode::addChild() adds children
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

$box = new ContainerNode();
$text = new ContentNode("Hello");

$result = $box->addChild($text);

// Should return self for chaining
var_dump($result === $box);

// Should have 1 child
var_dump(count($box->children));
?>
--EXPECT--
bool(true)
int(1)
