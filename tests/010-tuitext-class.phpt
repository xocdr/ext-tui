--TEST--
Xocdr\Tui\Ext\ContentNode class exists with properties
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContentNode;

$text = new ContentNode("Hello World");

var_dump($text instanceof ContentNode);
var_dump($text->content);
var_dump($text->bold);
var_dump($text->italic);
?>
--EXPECT--
bool(true)
string(11) "Hello World"
bool(false)
bool(false)
