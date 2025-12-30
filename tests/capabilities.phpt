--TEST--
Terminal capability detection
--EXTENSIONS--
tui
--FILE--
<?php
// Test tui_get_capabilities returns an array with expected keys
$caps = tui_get_capabilities();

// Check top-level keys
var_dump(isset($caps['terminal']));
var_dump(isset($caps['name']));
var_dump(isset($caps['version']));
var_dump(isset($caps['color_depth']));
var_dump(isset($caps['capabilities']));

// Check capabilities sub-array
$capKeys = [
    'true_color', '256_color', 'mouse', 'mouse_sgr',
    'bracketed_paste', 'clipboard_osc52', 'hyperlinks_osc8',
    'sync_output', 'unicode', 'cursor_shape', 'title',
    'alternate_screen'
];

foreach ($capKeys as $key) {
    $exists = isset($caps['capabilities'][$key]);
    if (!$exists) {
        echo "Missing capability: $key\n";
    }
}
echo "All capability keys exist\n";

// Test tui_has_capability
var_dump(is_bool(tui_has_capability('cursor_shape')));
var_dump(is_bool(tui_has_capability('true_color')));
var_dump(is_bool(tui_has_capability('unknown_cap')));  // Unknown capability returns false
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
All capability keys exist
bool(true)
bool(true)
bool(true)
