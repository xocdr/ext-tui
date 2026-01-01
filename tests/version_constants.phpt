--TEST--
Version constants
--EXTENSIONS--
tui
--FILE--
<?php
// Test TUI_VERSION constant
var_dump(defined('TUI_VERSION'));
var_dump(is_string(TUI_VERSION));
var_dump(preg_match('/^\d+\.\d+\.\d+$/', TUI_VERSION) === 1);

// Test TUI_VERSION_ID constant
var_dump(defined('TUI_VERSION_ID'));
var_dump(is_int(TUI_VERSION_ID));
var_dump(TUI_VERSION_ID > 0);

// Verify version ID matches version string
$parts = explode('.', TUI_VERSION);
$expectedId = (int)$parts[0] * 10000 + (int)$parts[1] * 100 + (int)$parts[2];
var_dump(TUI_VERSION_ID === $expectedId);

echo "TUI_VERSION: " . TUI_VERSION . "\n";
echo "TUI_VERSION_ID: " . TUI_VERSION_ID . "\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
TUI_VERSION: 0.4.2
TUI_VERSION_ID: 402
