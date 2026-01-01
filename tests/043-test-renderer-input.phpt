--TEST--
Testing framework: input simulation (send_input, send_key, advance_frame)
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

$renderer = tui_test_create(80, 24);

// Create a simple UI
$box = new ContainerNode(['width' => 80, 'height' => 24]);
$box->children = [new ContentNode("Press a key")];
tui_test_render($renderer, $box);

// Test tui_test_send_input - queue some text
tui_test_send_input($renderer, "hello");
echo "send_input: ok\n";

// Test tui_test_send_key - queue a special key (ENTER = 1)
tui_test_send_key($renderer, 1);  // TUI_KEY_ENTER
echo "send_key: ok\n";

// Test tui_test_advance_frame - process the queued input
tui_test_advance_frame($renderer);
echo "advance_frame: ok\n";

// Multiple advance frames should work
tui_test_advance_frame($renderer);
tui_test_advance_frame($renderer);
echo "multiple advance_frame: ok\n";

// Test various key codes
tui_test_send_key($renderer, 2);  // TAB
tui_test_send_key($renderer, 3);  // ESCAPE
tui_test_send_key($renderer, 4);  // BACKSPACE
tui_test_send_key($renderer, 5);  // UP
tui_test_send_key($renderer, 6);  // DOWN
tui_test_send_key($renderer, 7);  // RIGHT
tui_test_send_key($renderer, 8);  // LEFT
tui_test_advance_frame($renderer);
echo "various keys: ok\n";

tui_test_destroy($renderer);
echo "Done\n";
?>
--EXPECT--
send_input: ok
send_key: ok
advance_frame: ok
multiple advance_frame: ok
various keys: ok
Done
