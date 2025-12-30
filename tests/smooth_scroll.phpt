--TEST--
Smooth scrolling animation functions
--EXTENSIONS--
tui
--FILE--
<?php
// Create scroll animation
$anim = tui_scroll_create();

// Initial state
var_dump(tui_scroll_is_animating($anim));  // false - not animating yet
$pos = tui_scroll_get_position($anim);
var_dump($pos['x'] === 0.0);  // starts at 0
var_dump($pos['y'] === 0.0);

// Set target - should start animating
tui_scroll_set_target($anim, 100.0, 50.0);
var_dump(tui_scroll_is_animating($anim));  // true

// Update animation (simulate 60fps for 1 second = 60 frames)
for ($i = 0; $i < 60; $i++) {
    tui_scroll_update($anim, 1.0/60.0);
}

// Should have converged close to target
$pos = tui_scroll_get_position($anim);
var_dump(abs($pos['x'] - 100.0) < 1.0);  // close to 100
var_dump(abs($pos['y'] - 50.0) < 1.0);   // close to 50

// Test snap
tui_scroll_set_target($anim, 200.0, 100.0);
tui_scroll_snap($anim);  // Immediately jump to target
$pos = tui_scroll_get_position($anim);
var_dump($pos['x'] === 200.0);
var_dump($pos['y'] === 100.0);
var_dump(tui_scroll_is_animating($anim));  // false after snap

// Test scroll_by (incremental)
tui_scroll_by($anim, 10.0, 5.0);  // Target becomes 210, 105
var_dump(tui_scroll_is_animating($anim));  // true

// Update until done
for ($i = 0; $i < 120; $i++) {
    if (!tui_scroll_update($anim, 1.0/60.0)) break;
}
var_dump(tui_scroll_is_animating($anim));  // false - converged

$pos = tui_scroll_get_position($anim);
var_dump(abs($pos['x'] - 210.0) < 0.1);
var_dump(abs($pos['y'] - 105.0) < 0.1);

// Test spring parameters
tui_scroll_set_spring($anim, 400.0, 1.0);  // Stiffer spring
tui_scroll_set_target($anim, 300.0, 150.0);

// Stiffer spring should converge faster
for ($i = 0; $i < 30; $i++) {
    tui_scroll_update($anim, 1.0/60.0);
}
$pos = tui_scroll_get_position($anim);
var_dump(abs($pos['x'] - 300.0) < 5.0);  // Should be close

// Clean up
tui_scroll_destroy($anim);
echo "Smooth scroll tests passed\n";
?>
--EXPECT--
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(true)
bool(false)
bool(true)
bool(true)
bool(true)
Smooth scroll tests passed
