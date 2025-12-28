--TEST--
TuiText class exists with properties
--EXTENSIONS--
tui
--FILE--
<?php
$text = new TuiText("Hello World");

var_dump($text instanceof TuiText);
var_dump($text->content);
var_dump($text->bold);
var_dump($text->italic);
?>
--EXPECT--
bool(true)
string(11) "Hello World"
bool(false)
bool(false)
