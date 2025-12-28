--TEST--
TuiText constructor accepts content and properties
--EXTENSIONS--
tui
--FILE--
<?php
$text = new TuiText("Hello", [
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
