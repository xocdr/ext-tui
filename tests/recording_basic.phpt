--TEST--
Screen recording basic functionality
--EXTENSIONS--
tui
--FILE--
<?php

// Test function existence
var_dump(function_exists('tui_record_create'));
var_dump(function_exists('tui_record_start'));
var_dump(function_exists('tui_record_capture'));
var_dump(function_exists('tui_record_stop'));
var_dump(function_exists('tui_record_export'));

// Create recording
$rec = tui_record_create(80, 24, "Test Recording");
var_dump(is_resource($rec));

// Test initial frame count
var_dump(tui_record_frame_count($rec) === 0);

// Start and capture
tui_record_start($rec);
tui_record_capture($rec, "Frame 1\n");
tui_record_capture($rec, "Frame 2\n");
tui_record_stop($rec);

// Verify frames
var_dump(tui_record_frame_count($rec) === 2);

// Test export contains expected header
$export = tui_record_export($rec);
var_dump(strpos($export, '"version": 2') !== false);
var_dump(strpos($export, '"width": 80') !== false);
var_dump(strpos($export, '"height": 24') !== false);
var_dump(strpos($export, 'Frame 1') !== false);

// Cleanup
tui_record_destroy($rec);

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
OK
