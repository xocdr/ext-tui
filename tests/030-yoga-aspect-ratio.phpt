--TEST--
Yoga aspectRatio property
--EXTENSIONS--
tui
--FILE--
<?php
// Test aspectRatio property exists and can be set
$box = new Xocdr\Tui\Ext\ContainerNode(['aspectRatio' => 2.0]);
var_dump($box->aspectRatio);

$box2 = new Xocdr\Tui\Ext\ContainerNode(['aspectRatio' => 0.5]);
var_dump($box2->aspectRatio);

$box3 = new Xocdr\Tui\Ext\ContainerNode(['aspectRatio' => 16/9]);
var_dump(round($box3->aspectRatio, 4));

// Test default (null)
$box4 = new Xocdr\Tui\Ext\ContainerNode([]);
var_dump($box4->aspectRatio);
?>
--EXPECT--
float(2)
float(0.5)
float(1.7778)
NULL
