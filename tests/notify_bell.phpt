--TEST--
tui_bell() executes without error
--EXTENSIONS--
tui
--FILE--
<?php
// tui_bell writes BEL character directly to terminal (STDOUT_FILENO)
// We can't capture it with PHP output buffering
// Just verify it runs without error
tui_bell();
echo "OK\n";
?>
--EXPECTREGEX--
.*OK
