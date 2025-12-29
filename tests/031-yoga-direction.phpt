--TEST--
Yoga direction (RTL/LTR) property
--EXTENSIONS--
tui
--FILE--
<?php
// Test direction property
$box1 = new Xocdr\Tui\Ext\Box(['direction' => 'rtl']);
var_dump($box1->direction);

$box2 = new Xocdr\Tui\Ext\Box(['direction' => 'ltr']);
var_dump($box2->direction);

// Test default (null = inherit)
$box3 = new Xocdr\Tui\Ext\Box([]);
var_dump($box3->direction);
?>
--EXPECT--
string(3) "rtl"
string(3) "ltr"
NULL
