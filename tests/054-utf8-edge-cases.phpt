--TEST--
UTF-8 edge cases and malformed sequences
--EXTENSIONS--
tui
--FILE--
<?php
echo "=== Valid UTF-8 sequences ===\n";

// Single byte ASCII
$result = tui_string_width("A");
echo "ASCII 'A': width = $result\n";

// 2-byte UTF-8 (Latin extended, Greek, etc.)
$result = tui_string_width("é");  // U+00E9
echo "2-byte 'é': width = $result\n";

// 3-byte UTF-8 (CJK, etc.)
$result = tui_string_width("中");  // U+4E2D - wide character
echo "3-byte '中' (wide): width = $result\n";

// 4-byte UTF-8 (emoji, rare CJK)
$result = tui_string_width("😀");  // U+1F600
echo "4-byte '😀' (emoji): width = $result\n";

echo "\n=== Combining characters ===\n";

// Base + combining mark
$result = tui_string_width("e\xCC\x81");  // e + combining acute
echo "e + combining acute: width = $result\n";

// Multiple combining marks
$result = tui_string_width("a\xCC\x81\xCC\x82");  // a + acute + circumflex
echo "a + 2 combining marks: width = $result\n";

echo "\n=== Zero-width characters ===\n";

// Zero-width space (U+200B) - may count as 1 depending on implementation
$result = tui_string_width("\xE2\x80\x8B");
echo "Zero-width space: width = $result\n";

// Zero-width joiner (U+200D)
$result = tui_string_width("\xE2\x80\x8D");
echo "Zero-width joiner: width = $result\n";

echo "\n=== Truncated sequences (should handle gracefully) ===\n";

// Truncated 2-byte sequence (only first byte)
$result = tui_string_width("\xC3");
echo "Truncated 2-byte: width = $result\n";

// Truncated 3-byte sequence (only 2 bytes)
$result = tui_string_width("\xE4\xB8");
echo "Truncated 3-byte: width = $result\n";

// Truncated 4-byte sequence (only 3 bytes)
$result = tui_string_width("\xF0\x9F\x98");
echo "Truncated 4-byte: width = $result\n";

echo "\n=== Invalid continuation bytes ===\n";

// Start byte followed by non-continuation
$result = tui_string_width("\xC3X");  // Should treat as separate
echo "Invalid continuation: width = $result\n";

// Bare continuation byte
$result = tui_string_width("\x80");
echo "Bare continuation: width = $result\n";

echo "\n=== Overlong encodings (invalid UTF-8) ===\n";

// Overlong encoding of '/' (U+002F)
// Valid: 0x2F, Invalid overlong: C0 AF
$result = tui_string_width("\xC0\xAF");
echo "Overlong '/': width = $result\n";

// Overlong encoding of NUL (U+0000)
// Invalid: C0 80
$result = tui_string_width("\xC0\x80");
echo "Overlong NUL: width = $result\n";

echo "\n=== Null bytes in strings ===\n";

// Embedded null - PHP string may be truncated at null
$result = tui_string_width("a\x00b");
echo "With embedded null: width = $result\n";

// Just null
$result = tui_string_width("\x00");
echo "Just null: width = $result\n";

echo "\n=== Surrogate pairs (invalid in UTF-8) ===\n";

// UTF-8 encoded surrogate (U+D800) - invalid
// ED A0 80
$result = tui_string_width("\xED\xA0\x80");
echo "Encoded surrogate: width = $result\n";

echo "\n=== Mixed valid and invalid ===\n";

$result = tui_string_width("Hello\xC3World");  // Invalid byte in middle
echo "Mixed: width = $result\n";

$result = tui_string_width("中\xFFText");  // Invalid 0xFF
echo "CJK + invalid: width = $result\n";

echo "\n=== Text operations on edge cases ===\n";

// Truncate with multibyte
$result = tui_truncate("中文字", 4);
echo "truncate CJK to 4: '$result'\n";

// Wrap with multibyte
$result = tui_wrap_text("中文 字符", 4);
echo "wrap CJK: " . json_encode($result) . "\n";

echo "\nAll UTF-8 edge case tests completed.\n";
?>
--EXPECT--
=== Valid UTF-8 sequences ===
ASCII 'A': width = 1
2-byte 'é': width = 1
3-byte '中' (wide): width = 2
4-byte '😀' (emoji): width = 2

=== Combining characters ===
e + combining acute: width = 1
a + 2 combining marks: width = 1

=== Zero-width characters ===
Zero-width space: width = 1
Zero-width joiner: width = 0

=== Truncated sequences (should handle gracefully) ===
Truncated 2-byte: width = 1
Truncated 3-byte: width = 2
Truncated 4-byte: width = 3

=== Invalid continuation bytes ===
Invalid continuation: width = 2
Bare continuation: width = 1

=== Overlong encodings (invalid UTF-8) ===
Overlong '/': width = 2
Overlong NUL: width = 2

=== Null bytes in strings ===
With embedded null: width = 1
Just null: width = 0

=== Surrogate pairs (invalid in UTF-8) ===
Encoded surrogate: width = 3

=== Mixed valid and invalid ===
Mixed: width = 11
CJK + invalid: width = 7

=== Text operations on edge cases ===
truncate CJK to 4: '...'
wrap CJK: ["\u4e2d\u6587","\u5b57\u7b26"]

All UTF-8 edge case tests completed.
