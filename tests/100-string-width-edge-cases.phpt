--TEST--
tui_string_width() comprehensive edge cases
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Comprehensive edge case testing for tui_string_width()
 * Tests: empty strings, NULL bytes, control chars, combining marks,
 * zero-width chars, wide chars, mixed content
 */

echo "=== Empty and NULL ===\n";
var_dump(tui_string_width(""));
var_dump(tui_string_width("   "));  // 3 spaces

echo "\n=== ASCII ===\n";
var_dump(tui_string_width("a"));
var_dump(tui_string_width("abc"));
var_dump(tui_string_width("Hello, World!"));

echo "\n=== Control characters (zero width) ===\n";
var_dump(tui_string_width("\t"));      // tab
var_dump(tui_string_width("\n"));      // newline
var_dump(tui_string_width("\r"));      // carriage return
var_dump(tui_string_width("\x1b"));    // escape
var_dump(tui_string_width("\x00"));    // NULL byte

echo "\n=== ANSI escape sequences (should be zero width) ===\n";
var_dump(tui_string_width("\x1b[31m"));           // red color
var_dump(tui_string_width("\x1b[0m"));            // reset
var_dump(tui_string_width("\x1b[1;31;40m"));      // bold red on black
var_dump(tui_string_width("\x1b[31mHello\x1b[0m")); // "Hello" with color = 5

echo "\n=== Wide characters (CJK - width 2) ===\n";
var_dump(tui_string_width("中"));      // Chinese
var_dump(tui_string_width("日本"));    // Japanese (2 chars = 4)
var_dump(tui_string_width("한글"));    // Korean (2 chars = 4)
var_dump(tui_string_width("你好世界")); // 4 chars = 8

echo "\n=== Mixed ASCII and wide ===\n";
var_dump(tui_string_width("Hello中文")); // 5 + 4 = 9
var_dump(tui_string_width("a中b"));      // 1 + 2 + 1 = 4

echo "\n=== Combining characters (zero width) ===\n";
var_dump(tui_string_width("e\xcc\x81"));  // e + combining acute = 1
var_dump(tui_string_width("n\xcc\x83"));  // n + combining tilde = 1

echo "\n=== Zero-width characters ===\n";
var_dump(tui_string_width("\xe2\x80\x8b"));  // ZWSP
var_dump(tui_string_width("\xe2\x80\x8c"));  // ZWNJ
var_dump(tui_string_width("\xe2\x80\x8d"));  // ZWJ
var_dump(tui_string_width("\xef\xbb\xbf"));  // BOM

echo "\n=== Full-width punctuation ===\n";
var_dump(tui_string_width("！"));     // fullwidth exclamation = 2
var_dump(tui_string_width("？"));     // fullwidth question = 2

echo "\n=== Emoji (typically width 2) ===\n";
var_dump(tui_string_width("😀"));
var_dump(tui_string_width("👍"));
var_dump(tui_string_width("🎉"));

echo "\n=== Very long strings ===\n";
$long = str_repeat("a", 10000);
var_dump(tui_string_width($long));

$longWide = str_repeat("中", 5000);
var_dump(tui_string_width($longWide));

echo "\n=== Binary data ===\n";
var_dump(tui_string_width("\x80\x81\x82"));  // invalid UTF-8

echo "\nDone!\n";
?>
--EXPECT--
=== Empty and NULL ===
int(0)
int(3)

=== ASCII ===
int(1)
int(3)
int(13)

=== Control characters (zero width) ===
int(0)
int(0)
int(0)
int(0)
int(0)

=== ANSI escape sequences (should be zero width) ===
int(0)
int(0)
int(0)
int(5)

=== Wide characters (CJK - width 2) ===
int(2)
int(4)
int(4)
int(8)

=== Mixed ASCII and wide ===
int(9)
int(4)

=== Combining characters (zero width) ===
int(1)
int(1)

=== Zero-width characters ===
int(1)
int(1)
int(0)
int(1)

=== Full-width punctuation ===
int(2)
int(2)

=== Emoji (typically width 2) ===
int(2)
int(2)
int(2)

=== Very long strings ===
int(10000)
int(10000)

=== Binary data ===
int(3)

Done!
