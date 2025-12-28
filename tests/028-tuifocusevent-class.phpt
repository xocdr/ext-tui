--TEST--
TuiFocusEvent class exists with correct properties
--SKIPIF--
<?php if (!extension_loaded('tui')) die('skip tui extension not loaded'); ?>
--FILE--
<?php
// Check class exists
var_dump(class_exists('TuiFocusEvent'));

// Create instance and check properties
$event = new TuiFocusEvent();
var_dump(property_exists($event, 'previous'));
var_dump(property_exists($event, 'current'));
var_dump(property_exists($event, 'direction'));

// Default values
var_dump($event->previous);
var_dump($event->current);
var_dump($event->direction);

echo "TuiFocusEvent class works correctly\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
NULL
NULL
string(0) ""
TuiFocusEvent class works correctly
