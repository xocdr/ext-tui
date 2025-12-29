--TEST--
Xocdr\Tui\Ext\Key class exists with properties
--EXTENSIONS--
tui
--FILE--
<?php
var_dump(class_exists('Xocdr\Tui\Ext\Key'));

// Check properties exist via reflection
$rc = new ReflectionClass('Xocdr\Tui\Ext\Key');
$props = $rc->getProperties();
$propNames = array_map(fn($p) => $p->getName(), $props);

var_dump(in_array('key', $propNames));
var_dump(in_array('upArrow', $propNames));
var_dump(in_array('downArrow', $propNames));
var_dump(in_array('leftArrow', $propNames));
var_dump(in_array('rightArrow', $propNames));
var_dump(in_array('return', $propNames));
var_dump(in_array('escape', $propNames));
var_dump(in_array('ctrl', $propNames));
var_dump(in_array('meta', $propNames));
var_dump(in_array('shift', $propNames));
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
