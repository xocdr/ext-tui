--TEST--
Xocdr\Tui\Ext\Text class exists with properties
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Text;

$text = new Text("Hello World");

var_dump($text instanceof Text);
var_dump($text->content);
var_dump($text->bold);
var_dump($text->italic);
?>
--EXPECT--
bool(true)
string(11) "Hello World"
bool(false)
bool(false)
