--TEST--
tui_notify() sends notification and returns bool
--EXTENSIONS--
tui
--FILE--
<?php
// tui_notify writes directly to terminal (STDOUT_FILENO)
// We verify return values only

// tui_notify requires title
$result = tui_notify("Test");
echo "result1: " . ($result ? "true" : "false") . "\n";

// With body
$result = tui_notify("Title", "Body");
echo "result2: " . ($result ? "true" : "false") . "\n";

// With priority
$result = tui_notify("Title", "Body", TUI_NOTIFY_NORMAL);
echo "result3: " . ($result ? "true" : "false") . "\n";

$result = tui_notify("Title", null, TUI_NOTIFY_URGENT);
echo "result4: " . ($result ? "true" : "false") . "\n";

echo "OK\n";
?>
--EXPECTREGEX--
.*result1: true.*
.*result2: true.*
.*result3: true.*
.*result4: true.*
OK
