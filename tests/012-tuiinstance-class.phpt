--TEST--
TuiInstance class exists
--EXTENSIONS--
tui
--FILE--
<?php
var_dump(class_exists('TuiInstance'));
var_dump(method_exists('TuiInstance', 'rerender'));
var_dump(method_exists('TuiInstance', 'unmount'));
var_dump(method_exists('TuiInstance', 'waitUntilExit'));
var_dump(method_exists('TuiInstance', 'exit'));
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
