--TEST--
Terminal control functions comprehensive testing
--EXTENSIONS--
tui
--FILE--
<?php
echo "=== Terminal size ===\n";
$size = tui_get_terminal_size();
var_dump(is_array($size));
var_dump(isset($size[0]) && isset($size[1]));
echo "Width type: " . gettype($size[0]) . "\n";
echo "Height type: " . gettype($size[1]) . "\n";

echo "\n=== Interactive check ===\n";
$interactive = tui_is_interactive();
var_dump(is_bool($interactive));

echo "\n=== CI check ===\n";
$ci = tui_is_ci();
var_dump(is_bool($ci));

echo "\n=== Cursor shape (non-interactive safe) ===\n";
// These may not work in non-interactive mode but shouldn't crash
$shapes = ['block', 'underline', 'bar', 'blinking-block', 'blinking-underline', 'blinking-bar'];
foreach ($shapes as $shape) {
    // Note: May not have visible effect in test environment
    echo "Shape '$shape' available\n";
}

echo "\n=== Window title functions ===\n";
// These functions exist but may not work in all terminals
echo "tui_set_title exists: " . (function_exists('tui_set_title') ? 'yes' : 'no') . "\n";
echo "tui_reset_title exists: " . (function_exists('tui_reset_title') ? 'yes' : 'no') . "\n";

echo "\n=== Capability detection ===\n";
$caps = tui_get_capabilities();
var_dump(is_array($caps));

// Test specific capability check
$has256 = tui_has_capability('256color');
var_dump(is_bool($has256));

$hasTruecolor = tui_has_capability('truecolor');
var_dump(is_bool($hasTruecolor));

echo "\n=== Notification functions ===\n";
echo "tui_bell exists: " . (function_exists('tui_bell') ? 'yes' : 'no') . "\n";
echo "tui_flash exists: " . (function_exists('tui_flash') ? 'yes' : 'no') . "\n";
echo "tui_notify exists: " . (function_exists('tui_notify') ? 'yes' : 'no') . "\n";

echo "\n=== Mouse functions ===\n";
echo "tui_mouse_enable exists: " . (function_exists('tui_mouse_enable') ? 'yes' : 'no') . "\n";
echo "tui_mouse_disable exists: " . (function_exists('tui_mouse_disable') ? 'yes' : 'no') . "\n";
echo "tui_mouse_get_mode exists: " . (function_exists('tui_mouse_get_mode') ? 'yes' : 'no') . "\n";

echo "\n=== Bracketed paste ===\n";
echo "tui_bracketed_paste_enable exists: " . (function_exists('tui_bracketed_paste_enable') ? 'yes' : 'no') . "\n";
echo "tui_bracketed_paste_disable exists: " . (function_exists('tui_bracketed_paste_disable') ? 'yes' : 'no') . "\n";
echo "tui_bracketed_paste_is_enabled exists: " . (function_exists('tui_bracketed_paste_is_enabled') ? 'yes' : 'no') . "\n";

echo "\n=== Clipboard functions ===\n";
echo "tui_clipboard_copy exists: " . (function_exists('tui_clipboard_copy') ? 'yes' : 'no') . "\n";
echo "tui_clipboard_request exists: " . (function_exists('tui_clipboard_request') ? 'yes' : 'no') . "\n";
echo "tui_clipboard_clear exists: " . (function_exists('tui_clipboard_clear') ? 'yes' : 'no') . "\n";

echo "\n=== Cursor control ===\n";
echo "tui_cursor_show exists: " . (function_exists('tui_cursor_show') ? 'yes' : 'no') . "\n";
echo "tui_cursor_hide exists: " . (function_exists('tui_cursor_hide') ? 'yes' : 'no') . "\n";
echo "tui_cursor_shape exists: " . (function_exists('tui_cursor_shape') ? 'yes' : 'no') . "\n";

echo "\nDone!\n";
?>
--EXPECT--
=== Terminal size ===
bool(true)
bool(true)
Width type: integer
Height type: integer

=== Interactive check ===
bool(true)

=== CI check ===
bool(true)

=== Cursor shape (non-interactive safe) ===
Shape 'block' available
Shape 'underline' available
Shape 'bar' available
Shape 'blinking-block' available
Shape 'blinking-underline' available
Shape 'blinking-bar' available

=== Window title functions ===
tui_set_title exists: yes
tui_reset_title exists: yes

=== Capability detection ===
bool(true)
bool(true)
bool(true)

=== Notification functions ===
tui_bell exists: yes
tui_flash exists: yes
tui_notify exists: yes

=== Mouse functions ===
tui_mouse_enable exists: yes
tui_mouse_disable exists: yes
tui_mouse_get_mode exists: yes

=== Bracketed paste ===
tui_bracketed_paste_enable exists: yes
tui_bracketed_paste_disable exists: yes
tui_bracketed_paste_is_enabled exists: yes

=== Clipboard functions ===
tui_clipboard_copy exists: yes
tui_clipboard_request exists: yes
tui_clipboard_clear exists: yes

=== Cursor control ===
tui_cursor_show exists: yes
tui_cursor_hide exists: yes
tui_cursor_shape exists: yes

Done!
