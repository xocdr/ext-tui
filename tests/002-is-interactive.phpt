--TEST--
tui_is_interactive() returns boolean
--EXTENSIONS--
tui
--FILE--
<?php
$result = tui_is_interactive();
var_dump(is_bool($result));
?>
--EXPECT--
bool(true)
