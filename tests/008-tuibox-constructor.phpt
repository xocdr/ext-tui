--TEST--
Xocdr\Tui\Ext\Box constructor accepts properties
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;

$box = new Box([
    'flexDirection' => 'row',
    'padding' => 2,
    'gap' => 1
]);

var_dump($box->flexDirection);
var_dump($box->padding);
var_dump($box->gap);
?>
--EXPECT--
string(3) "row"
int(2)
int(1)
