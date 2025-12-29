--TEST--
Xocdr\Tui\Ext\Text has all style properties
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Text;

$text = new Text("Test", [
    'bold' => true,
    'dim' => true,
    'italic' => true,
    'underline' => true,
    'inverse' => true,
    'strikethrough' => true
]);

var_dump($text->bold);
var_dump($text->dim);
var_dump($text->italic);
var_dump($text->underline);
var_dump($text->inverse);
var_dump($text->strikethrough);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
