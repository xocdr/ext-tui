--TEST--
Security: Fuzz testing text utility functions with edge cases
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Test that text utility functions handle malformed and edge case input safely.
 */

echo "Test 1: tui_string_width with edge cases\n";
$cases = [
    "" => 0,                          // Empty string
    "\x00" => 0,                      // Null byte
    "\t" => 0,                        // Tab (usually 0 or 1 width)
    "\n" => 0,                        // Newline
    "\r" => 0,                        // Carriage return
    "\x1b[31m" => 0,                  // ANSI escape (should be 0 width)
    str_repeat("a", 10000) => 10000,  // Long ASCII
];
foreach ($cases as $input => $expected) {
    $width = tui_string_width($input);
    $ok = ($expected === "any" || $width >= 0);
    echo "Width test: " . ($ok ? "OK" : "FAIL") . " (got $width)\n";
}

echo "\nTest 2: tui_string_width with invalid UTF-8\n";
$invalid_utf8 = [
    "\xff\xfe",           // Invalid start bytes
    "\xc0\x80",           // Overlong encoding
    "\xed\xa0\x80",       // Surrogate half
    "\xf8\x80\x80\x80",   // 5-byte sequence (invalid)
    "hello\xffworld",     // Invalid byte in middle
];
foreach ($invalid_utf8 as $i => $seq) {
    $width = tui_string_width($seq);
    echo "Invalid UTF-8 $i: width=$width (handled safely)\n";
}

echo "\nTest 3: tui_wrap_text with edge cases\n";
$wrap_cases = [
    ["", 10],                        // Empty
    ["hello", 0],                    // Zero width
    ["hello", -1],                   // Negative width
    ["hello", 1],                    // Very narrow
    [str_repeat("a", 10000), 80],    // Very long text
    ["word word word", 5],           // Words at boundary
];
foreach ($wrap_cases as $i => [$text, $width]) {
    $result = tui_wrap_text($text, $width);
    echo "Wrap $i: " . (is_array($result) ? "OK (lines=" . count($result) . ")" : gettype($result)) . "\n";
}

echo "\nTest 4: tui_truncate with edge cases\n";
$truncate_cases = [
    ["", 10, "..."],                    // Empty
    ["hello", 0, "..."],                // Zero width
    ["hello", -1, "..."],               // Negative width
    ["hello", 3, "..."],                // Width smaller than ellipsis
    ["hello", 5, ""],                   // Empty ellipsis
    [str_repeat("a", 10000), 10, "..."], // Very long text
    ["hello", 10, str_repeat(".", 100)], // Very long ellipsis
];
foreach ($truncate_cases as $i => [$text, $width, $ellipsis]) {
    $result = tui_truncate($text, $width, $ellipsis);
    echo "Truncate $i: " . (is_string($result) ? "OK (len=" . strlen($result) . ")" : gettype($result)) . "\n";
}

echo "\nTest 5: Wide characters (CJK)\n";
$cjk = "你好世界";  // 4 characters, 8 display width
$width = tui_string_width($cjk);
echo "CJK width: $width (expected 8)\n";

$wrapped = tui_wrap_text($cjk, 4);
echo "CJK wrap at 4: " . count($wrapped) . " lines\n";

$truncated = tui_truncate($cjk, 5, "...");
echo "CJK truncate at 5: '$truncated'\n";

echo "\nTest 6: Mixed content\n";
$mixed = "Hello 你好 World 世界";
$width = tui_string_width($mixed);
echo "Mixed width: $width\n";

echo "\nTest 7: Emoji handling\n";
$emoji = "Hello 😀 World 🎉";
$width = tui_string_width($emoji);
echo "Emoji width: $width (varies by terminal)\n";

echo "\nTest 8: Binary content\n";
$binary = random_bytes(100);
$width = tui_string_width($binary);
echo "Binary width: " . ($width >= 0 ? "OK (width=$width)" : "ERROR") . "\n";

$wrapped = tui_wrap_text($binary, 10);
echo "Binary wrap: " . (is_array($wrapped) ? "OK" : "ERROR") . "\n";

echo "\nText functions fuzz test completed safely!\n";
?>
--EXPECTF--
Test 1: tui_string_width with edge cases
Width test: OK (got 0)
Width test: OK (got %d)
Width test: OK (got %d)
Width test: OK (got %d)
Width test: OK (got %d)
Width test: OK (got %d)
Width test: OK (got 10000)

Test 2: tui_string_width with invalid UTF-8
Invalid UTF-8 0: width=%d (handled safely)
Invalid UTF-8 1: width=%d (handled safely)
Invalid UTF-8 2: width=%d (handled safely)
Invalid UTF-8 3: width=%d (handled safely)
Invalid UTF-8 4: width=%d (handled safely)

Test 3: tui_wrap_text with edge cases
Wrap 0: OK (lines=%d)
Wrap 1: OK (lines=%d)
Wrap 2: OK (lines=%d)
Wrap 3: OK (lines=%d)
Wrap 4: OK (lines=%d)
Wrap 5: OK (lines=%d)

Test 4: tui_truncate with edge cases
Truncate 0: OK (len=%d)
Truncate 1: OK (len=%d)
Truncate 2: OK (len=%d)
Truncate 3: OK (len=%d)
Truncate 4: OK (len=%d)
Truncate 5: OK (len=%d)
Truncate 6: OK (len=%d)

Test 5: Wide characters (CJK)
CJK width: 8 (expected 8)
CJK wrap at 4: %d lines
CJK truncate at 5: '%s'

Test 6: Mixed content
Mixed width: %d

Test 7: Emoji handling
Emoji width: %d (varies by terminal)

Test 8: Binary content
Binary width: OK (width=%d)
Binary wrap: OK

Text functions fuzz test completed safely!
