--TEST--
Security: Fuzz testing mouse input parser with malformed/oversized sequences
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Test that mouse input parser handles malformed and oversized input safely.
 * The parser should return null/false for invalid input without crashing.
 */

echo "Test 1: Empty input\n";
$result = tui_parse_mouse("");
var_dump($result);

echo "\nTest 2: Truncated sequences\n";
// Partial SGR mouse sequences
$truncated = [
    "\x1b",           // Just ESC
    "\x1b[",          // ESC [
    "\x1b[<",         // ESC [ <
    "\x1b[<0",        // Missing semicolons
    "\x1b[<0;",       // Missing y and terminator
    "\x1b[<0;1",      // Missing semicolon
    "\x1b[<0;1;",     // Missing y and terminator
    "\x1b[<0;1;1",    // Missing terminator
];
foreach ($truncated as $i => $seq) {
    $result = tui_parse_mouse($seq);
    echo "Truncated $i: " . ($result === null ? "rejected (null)" : "ERROR: parsed") . "\n";
}

echo "\nTest 3: Invalid terminators\n";
$invalid_term = [
    "\x1b[<0;1;1X",   // Wrong terminator
    "\x1b[<0;1;1\x00", // Null terminator
    "\x1b[<0;1;1\n",  // Newline
];
foreach ($invalid_term as $i => $seq) {
    $result = tui_parse_mouse($seq);
    echo "Invalid term $i: " . ($result === null ? "rejected (null)" : "ERROR: parsed") . "\n";
}

echo "\nTest 4: Extremely large coordinate values (overflow test)\n";
// These should be rejected to prevent integer overflow
$overflow = [
    "\x1b[<0;99999999999999999;1M",  // Huge X
    "\x1b[<0;1;99999999999999999M",  // Huge Y
    "\x1b[<99999999999999999;1;1M",  // Huge button
    "\x1b[<2147483648;1;1M",         // INT_MAX + 1
];
foreach ($overflow as $i => $seq) {
    $result = tui_parse_mouse($seq);
    echo "Overflow $i: " . ($result === null ? "rejected (null)" : "parsed (coordinates may be clamped)") . "\n";
}

echo "\nTest 5: Negative-like values (minus signs in sequence)\n";
$negative = [
    "\x1b[<-1;1;1M",
    "\x1b[<0;-1;1M",
    "\x1b[<0;1;-1M",
];
foreach ($negative as $i => $seq) {
    $result = tui_parse_mouse($seq);
    echo "Negative $i: " . ($result === null ? "rejected (null)" : "ERROR: parsed") . "\n";
}

echo "\nTest 6: Non-numeric values in coordinates\n";
$nonnumeric = [
    "\x1b[<abc;1;1M",
    "\x1b[<0;abc;1M",
    "\x1b[<0;1;abcM",
    "\x1b[<0x10;1;1M",
];
foreach ($nonnumeric as $i => $seq) {
    $result = tui_parse_mouse($seq);
    echo "Non-numeric $i: " . ($result === null ? "rejected (null)" : "ERROR: parsed") . "\n";
}

echo "\nTest 7: Binary garbage\n";
$garbage = [
    str_repeat("\xff", 100),
    str_repeat("\x00", 100),
    random_bytes(50),
    "\x1b[<" . str_repeat("9", 1000) . ";1;1M",  // Very long number
];
foreach ($garbage as $i => $seq) {
    $result = tui_parse_mouse($seq);
    echo "Garbage $i: " . ($result === null ? "rejected (null)" : "parsed") . "\n";
}

echo "\nTest 8: Valid sequences (sanity check)\n";
$valid = [
    "\x1b[<0;10;20M",   // Left press
    "\x1b[<0;10;20m",   // Left release
    "\x1b[<1;10;20M",   // Middle press
    "\x1b[<2;10;20M",   // Right press
    "\x1b[<64;10;20M",  // Scroll up
    "\x1b[<35;10;20M",  // Motion with button
];
foreach ($valid as $i => $seq) {
    $result = tui_parse_mouse($seq);
    if ($result !== null) {
        echo "Valid $i: parsed (x={$result['x']}, y={$result['y']})\n";
    } else {
        echo "Valid $i: ERROR - should have parsed\n";
    }
}

echo "\nMouse input fuzz test completed safely!\n";
?>
--EXPECTF--
Test 1: Empty input
NULL

Test 2: Truncated sequences
Truncated 0: rejected (null)
Truncated 1: rejected (null)
Truncated 2: rejected (null)
Truncated 3: rejected (null)
Truncated 4: rejected (null)
Truncated 5: rejected (null)
Truncated 6: rejected (null)
Truncated 7: rejected (null)

Test 3: Invalid terminators
Invalid term 0: rejected (null)
Invalid term 1: rejected (null)
Invalid term 2: rejected (null)

Test 4: Extremely large coordinate values (overflow test)
Overflow 0: rejected (null)
Overflow 1: rejected (null)
Overflow 2: rejected (null)
Overflow 3: rejected (null)

Test 5: Negative-like values (minus signs in sequence)
Negative 0: rejected (null)
Negative 1: rejected (null)
Negative 2: rejected (null)

Test 6: Non-numeric values in coordinates
Non-numeric 0: rejected (null)
Non-numeric 1: rejected (null)
Non-numeric 2: rejected (null)
Non-numeric 3: rejected (null)

Test 7: Binary garbage
Garbage 0: rejected (null)
Garbage 1: rejected (null)
Garbage 2: rejected (null)
Garbage 3: rejected (null)

Test 8: Valid sequences (sanity check)
Valid 0: parsed (x=9, y=19)
Valid 1: parsed (x=9, y=19)
Valid 2: parsed (x=9, y=19)
Valid 3: parsed (x=9, y=19)
Valid 4: parsed (x=9, y=19)
Valid 5: parsed (x=9, y=19)

Mouse input fuzz test completed safely!
