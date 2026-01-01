--TEST--
Security: Fuzz testing keyboard input parser with malformed/oversized sequences
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Test that keyboard input parser handles malformed and oversized input safely.
 * The parser should handle any input without crashing.
 */

echo "Test 1: Empty and null-like input\n";
$result = tui_parse_key("");
var_dump($result === null || (is_array($result) && empty($result['key'])));

echo "\nTest 2: Single control characters\n";
for ($i = 0; $i < 32; $i++) {
    $result = tui_parse_key(chr($i));
    // Should parse without crashing - control chars are valid input
    if ($result === null) {
        echo "Ctrl char $i: null\n";
    }
}
echo "All control characters handled safely\n";

echo "\nTest 3: Truncated escape sequences\n";
$truncated = [
    "\x1b",           // Just ESC - valid escape key
    "\x1b[",          // Incomplete CSI
    "\x1b[1",         // Incomplete modifier
    "\x1b[1;",        // Incomplete modifier value
    "\x1bO",          // Incomplete SS3
];
foreach ($truncated as $i => $seq) {
    $result = tui_parse_key($seq);
    echo "Truncated $i: " . (is_array($result) ? "handled" : "null") . "\n";
}

echo "\nTest 4: Invalid escape sequences\n";
$invalid = [
    "\x1b[999999999~",    // Invalid function key number
    "\x1b[A",             // Valid arrow
    "\x1b[ZZZZZ",         // Invalid CSI
    "\x1bOZ",             // Invalid SS3
    "\x1b\x1b\x1b",       // Multiple escapes
];
foreach ($invalid as $i => $seq) {
    $result = tui_parse_key($seq);
    echo "Invalid $i: " . (is_array($result) ? "handled" : "null") . "\n";
}

echo "\nTest 5: Very long escape sequences\n";
$long = [
    "\x1b[" . str_repeat("1", 100) . "~",    // Long number
    "\x1b[" . str_repeat(";", 50) . "A",     // Many semicolons
    "\x1b[1;" . str_repeat("9", 100) . "A",  // Long modifier
];
foreach ($long as $i => $seq) {
    $result = tui_parse_key($seq);
    echo "Long $i: " . (is_array($result) ? "handled" : "null") . "\n";
}

echo "\nTest 6: Binary garbage\n";
$garbage = [
    str_repeat("\xff", 50),
    str_repeat("\x00", 50),
    "\x1b[<0;1;1M",  // Mouse sequence to keyboard parser
];
foreach ($garbage as $i => $seq) {
    $result = tui_parse_key($seq);
    echo "Garbage $i: " . (is_array($result) || $result === null ? "handled safely" : "ERROR") . "\n";
}

echo "\nTest 7: Valid special keys (sanity check)\n";
$valid = [
    "\x1b[A" => "upArrow",
    "\x1b[B" => "downArrow",
    "\x1b[C" => "rightArrow",
    "\x1b[D" => "leftArrow",
    "\x1b[H" => "home",
    "\x1b[F" => "end",
    "\x1b[3~" => "delete",
    "\x1b[5~" => "pageUp",
    "\x1b[6~" => "pageDown",
    "\x1bOP" => "F1",
    "\x1bOQ" => "F2",
];
foreach ($valid as $seq => $name) {
    $result = tui_parse_key($seq);
    if (is_array($result)) {
        echo "$name: parsed\n";
    } else {
        echo "$name: ERROR - should have parsed\n";
    }
}

echo "\nTest 8: UTF-8 multi-byte characters\n";
$utf8 = [
    "\xc3\xa9",       // é (2 bytes)
    "\xe4\xb8\xad",   // 中 (3 bytes)
    "\xf0\x9f\x98\x80", // emoji (4 bytes)
];
foreach ($utf8 as $i => $char) {
    $result = tui_parse_key($char);
    echo "UTF-8 $i: " . (is_array($result) ? "handled" : "null") . "\n";
}

echo "\nTest 9: Invalid UTF-8 sequences\n";
$invalid_utf8 = [
    "\xff\xfe",       // Invalid start bytes
    "\xc0\x80",       // Overlong encoding
    "\xed\xa0\x80",   // Surrogate half
];
foreach ($invalid_utf8 as $i => $seq) {
    $result = tui_parse_key($seq);
    echo "Invalid UTF-8 $i: " . (is_array($result) || $result === null ? "handled safely" : "ERROR") . "\n";
}

echo "\nKeyboard input fuzz test completed safely!\n";
?>
--EXPECTF--
Test 1: Empty and null-like input
bool(true)

Test 2: Single control characters
All control characters handled safely

Test 3: Truncated escape sequences
Truncated 0: handled
Truncated 1: handled
Truncated 2: handled
Truncated 3: handled
Truncated 4: handled

Test 4: Invalid escape sequences
Invalid 0: handled
Invalid 1: handled
Invalid 2: handled
Invalid 3: handled
Invalid 4: handled

Test 5: Very long escape sequences
Long 0: handled
Long 1: handled
Long 2: handled

Test 6: Binary garbage
Garbage 0: handled safely
Garbage 1: handled safely
Garbage 2: handled safely

Test 7: Valid special keys (sanity check)
upArrow: parsed
downArrow: parsed
rightArrow: parsed
leftArrow: parsed
home: parsed
end: parsed
delete: parsed
pageUp: parsed
pageDown: parsed
F1: parsed
F2: parsed

Test 8: UTF-8 multi-byte characters
UTF-8 0: handled
UTF-8 1: handled
UTF-8 2: handled

Test 9: Invalid UTF-8 sequences
Invalid UTF-8 0: handled safely
Invalid UTF-8 1: handled safely
Invalid UTF-8 2: handled safely

Keyboard input fuzz test completed safely!
