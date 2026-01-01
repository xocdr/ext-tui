--TEST--
Xocdr\Tui\Ext\ContentNode constructor accepts content and properties
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContentNode;

$text = new ContentNode("Hello", [
    'bold' => true,
    'color' => '#ff0000'
]);

var_dump($text->content);
var_dump($text->bold);
var_dump($text->color);
?>
--EXPECT--
string(5) "Hello"
bool(true)
string(7) "#ff0000"
