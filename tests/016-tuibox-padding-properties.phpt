--TEST--
TuiBox has all padding and margin properties
--EXTENSIONS--
tui
--FILE--
<?php
$box = new TuiBox([
    'padding' => 1,
    'paddingTop' => 2,
    'paddingX' => 3,
    'margin' => 4,
    'marginLeft' => 5,
    'marginY' => 6
]);

var_dump($box->padding);
var_dump($box->paddingTop);
var_dump($box->paddingX);
var_dump($box->margin);
var_dump($box->marginLeft);
var_dump($box->marginY);
?>
--EXPECT--
int(1)
int(2)
int(3)
int(4)
int(5)
int(6)
