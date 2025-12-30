--TEST--
Xocdr\Tui\Ext\Box has focusable and focused properties
--SKIPIF--
<?php if (!extension_loaded('tui')) die('skip tui extension not loaded'); ?>
--FILE--
<?php
use Xocdr\Tui\Ext\Box;

$box = new Box();
var_dump(property_exists($box, 'focusable'));
var_dump(property_exists($box, 'focused'));
var_dump($box->focusable);
var_dump($box->focused);

// Test setting focusable
$focusable = new Box(['focusable' => true]);
var_dump($focusable->focusable);

// Test setting focused
$focused = new Box(['focusable' => true, 'focused' => true]);
var_dump($focused->focused);
?>
--EXPECT--
bool(true)
bool(true)
bool(false)
bool(false)
bool(true)
bool(true)
