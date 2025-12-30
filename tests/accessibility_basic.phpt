--TEST--
Accessibility functions
--EXTENSIONS--
tui
--FILE--
<?php

// Test function existence
var_dump(function_exists('tui_announce'));
var_dump(function_exists('tui_prefers_reduced_motion'));
var_dump(function_exists('tui_prefers_high_contrast'));
var_dump(function_exists('tui_get_accessibility_features'));
var_dump(function_exists('tui_aria_role_to_string'));
var_dump(function_exists('tui_aria_role_from_string'));

// Test ARIA role constants
var_dump(defined('TUI_ARIA_ROLE_BUTTON'));
var_dump(defined('TUI_ARIA_ROLE_NAVIGATION'));

// Test role conversion
var_dump(tui_aria_role_to_string(TUI_ARIA_ROLE_BUTTON) === 'button');
var_dump(tui_aria_role_to_string(TUI_ARIA_ROLE_NAVIGATION) === 'navigation');
var_dump(tui_aria_role_from_string('button') === TUI_ARIA_ROLE_BUTTON);
var_dump(tui_aria_role_from_string('navigation') === TUI_ARIA_ROLE_NAVIGATION);

// Test accessibility features returns array
$features = tui_get_accessibility_features();
var_dump(is_array($features));
var_dump(array_key_exists('reduced_motion', $features));
var_dump(array_key_exists('high_contrast', $features));
var_dump(array_key_exists('screen_reader', $features));

// Test preference functions return bool
var_dump(is_bool(tui_prefers_reduced_motion()));
var_dump(is_bool(tui_prefers_high_contrast()));

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
bool(true)
bool(true)
bool(true)
bool(true)
OK
