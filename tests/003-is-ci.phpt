--TEST--
tui_is_ci() returns boolean
--EXTENSIONS--
tui
--FILE--
<?php
$result = tui_is_ci();
var_dump(is_bool($result));
?>
--EXPECT--
bool(true)
