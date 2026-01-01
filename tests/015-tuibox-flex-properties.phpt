--TEST--
Xocdr\Tui\Ext\ContainerNode has all flex layout properties
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;

$box = new ContainerNode([
    'flexDirection' => 'row',
    'alignItems' => 'center',
    'justifyContent' => 'space-between',
    'flexGrow' => 1,
    'flexShrink' => 0,
    'width' => 100,
    'height' => 50
]);

var_dump($box->flexDirection);
var_dump($box->alignItems);
var_dump($box->justifyContent);
var_dump($box->flexGrow);
var_dump($box->flexShrink);
var_dump($box->width);
var_dump($box->height);
?>
--EXPECT--
string(3) "row"
string(6) "center"
string(13) "space-between"
int(1)
int(0)
int(100)
int(50)
