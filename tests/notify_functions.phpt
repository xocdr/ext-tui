--TEST--
Notification functions exist and have correct signatures
--EXTENSIONS--
tui
--FILE--
<?php
// Test function existence
var_dump(function_exists('tui_bell'));
var_dump(function_exists('tui_flash'));
var_dump(function_exists('tui_notify'));

// Test constants
var_dump(defined('TUI_NOTIFY_NORMAL'));
var_dump(defined('TUI_NOTIFY_URGENT'));
var_dump(TUI_NOTIFY_NORMAL === 0);
var_dump(TUI_NOTIFY_URGENT === 1);

// Test reflection for tui_bell
$rf = new ReflectionFunction('tui_bell');
var_dump($rf->getNumberOfParameters() === 0);
var_dump($rf->getReturnType()->getName() === 'void');

// Test reflection for tui_flash
$rf = new ReflectionFunction('tui_flash');
var_dump($rf->getNumberOfParameters() === 0);
var_dump($rf->getReturnType()->getName() === 'void');

// Test reflection for tui_notify
$rf = new ReflectionFunction('tui_notify');
var_dump($rf->getNumberOfRequiredParameters() === 1);
var_dump($rf->getNumberOfParameters() === 3);
var_dump($rf->getReturnType()->getName() === 'bool');

echo "OK\n";
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
bool(true)
bool(true)
bool(true)
OK
