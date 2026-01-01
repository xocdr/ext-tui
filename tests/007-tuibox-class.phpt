--TEST--
Xocdr\Tui\Ext\ContainerNode class exists with properties
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;

$box = new ContainerNode();

// Check class exists
var_dump($box instanceof ContainerNode);

// Check default properties
var_dump($box->flexDirection);
var_dump($box->flexGrow);
var_dump($box->flexShrink);
var_dump(is_array($box->children));
?>
--EXPECT--
bool(true)
string(6) "column"
int(0)
int(1)
bool(true)
