--TEST--
Xocdr\Tui\Ext\FocusEvent class exists with correct properties
--SKIPIF--
<?php if (!extension_loaded('tui')) die('skip tui extension not loaded'); ?>
--FILE--
<?php
use Xocdr\Tui\Ext\FocusEvent;

// Check class exists
var_dump(class_exists('Xocdr\Tui\Ext\FocusEvent'));

// Create instance and check properties
$event = new FocusEvent();
var_dump(property_exists($event, 'previous'));
var_dump(property_exists($event, 'current'));
var_dump(property_exists($event, 'direction'));

// Default values
var_dump($event->previous);
var_dump($event->current);
var_dump($event->direction);

echo "Xocdr\\Tui\\Ext\\FocusEvent class works correctly\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
NULL
NULL
string(0) ""
Xocdr\Tui\Ext\FocusEvent class works correctly
