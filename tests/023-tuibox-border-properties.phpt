--TEST--
TuiBox has border style properties
--SKIPIF--
<?php if (!extension_loaded('tui')) die('skip tui extension not loaded'); ?>
--FILE--
<?php
$box = new TuiBox();
var_dump(property_exists($box, 'borderStyle'));
var_dump(property_exists($box, 'borderColor'));
var_dump($box->borderStyle);
var_dump($box->borderColor);

// Test setting border style
$bordered = new TuiBox(['borderStyle' => 'single', 'borderColor' => '#ff0000']);
var_dump($bordered->borderStyle);
var_dump($bordered->borderColor);
?>
--EXPECT--
bool(true)
bool(true)
NULL
NULL
string(6) "single"
string(7) "#ff0000"
