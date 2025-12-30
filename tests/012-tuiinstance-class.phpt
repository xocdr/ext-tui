--TEST--
Xocdr\Tui\Ext\Instance class exists
--EXTENSIONS--
tui
--FILE--
<?php
var_dump(class_exists('Xocdr\Tui\Ext\Instance'));
var_dump(method_exists('Xocdr\Tui\Ext\Instance', 'rerender'));
var_dump(method_exists('Xocdr\Tui\Ext\Instance', 'unmount'));
var_dump(method_exists('Xocdr\Tui\Ext\Instance', 'waitUntilExit'));
var_dump(method_exists('Xocdr\Tui\Ext\Instance', 'exit'));
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
