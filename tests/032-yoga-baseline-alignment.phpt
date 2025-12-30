--TEST--
Yoga baseline alignment
--EXTENSIONS--
tui
--FILE--
<?php
// Test alignItems baseline
$box1 = new Xocdr\Tui\Ext\Box(['alignItems' => 'baseline']);
var_dump($box1->alignItems);

// Test alignSelf baseline
$box2 = new Xocdr\Tui\Ext\Box(['alignSelf' => 'baseline']);
var_dump($box2->alignSelf);

// Test other alignment values still work
$box3 = new Xocdr\Tui\Ext\Box(['alignItems' => 'center']);
var_dump($box3->alignItems);

$box4 = new Xocdr\Tui\Ext\Box(['alignItems' => 'stretch']);
var_dump($box4->alignItems);
?>
--EXPECT--
string(8) "baseline"
string(8) "baseline"
string(6) "center"
string(7) "stretch"
