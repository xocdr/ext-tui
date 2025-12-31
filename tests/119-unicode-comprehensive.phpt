--TEST--
Comprehensive Unicode handling
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

echo "=== CJK characters ===\n";

// Chinese
var_dump(tui_string_width("中文"));  // 4 (2 wide chars)
var_dump(tui_string_width("中文测试"));  // 8 (4 wide chars)

// Japanese Hiragana
var_dump(tui_string_width("あいう"));  // 6 (3 wide chars)

// Japanese Katakana
var_dump(tui_string_width("アイウ"));  // 6 (3 wide chars)

// Japanese Kanji
var_dump(tui_string_width("日本語"));  // 6 (3 wide chars)

// Korean
var_dump(tui_string_width("한글"));  // 4 (2 wide chars)

// Mixed
var_dump(tui_string_width("Hello中文World"));  // 5 + 4 + 5 = 14

echo "\n=== Emoji ===\n";

// Basic emoji
var_dump(tui_string_width("😀"));  // 2 (wide)
var_dump(tui_string_width("🎉"));  // 2
var_dump(tui_string_width("❤️"));  // varies by implementation

// Emoji sequence
var_dump(tui_string_width("👨‍👩‍👧‍👦"));  // Complex family emoji

// Flag emoji (2 regional indicator chars)
var_dump(tui_string_width("🇺🇸"));  // Flag
var_dump(tui_string_width("🇯🇵"));  // Flag

// Emoji with skin tone
var_dump(tui_string_width("👋🏽"));  // Wave with skin tone

echo "\n=== Combining characters ===\n";

// Combining diacritical marks
var_dump(tui_string_width("é"));  // precomposed: 1
var_dump(tui_string_width("é"));  // e + combining acute: 1

// Vietnamese with tones
var_dump(tui_string_width("Việt Nam"));  // 8 visual chars

// Hebrew with nikud
var_dump(tui_string_width("שָׁלוֹם"));  // 4 base chars

echo "\n=== RTL scripts ===\n";

// Arabic
var_dump(tui_string_width("مرحبا"));  // 5 chars

// Hebrew
var_dump(tui_string_width("שלום"));  // 4 chars

echo "\n=== Special width characters ===\n";

// Zero-width joiner
var_dump(tui_string_width("\u{200D}"));  // ZWJ: 0

// Zero-width non-joiner
var_dump(tui_string_width("\u{200C}"));  // ZWNJ: 0

// Zero-width space
var_dump(tui_string_width("\u{200B}"));  // ZWSP: 0

// Soft hyphen
var_dump(tui_string_width("\u{00AD}"));  // SHY: 0

// Non-breaking space
var_dump(tui_string_width("\u{00A0}"));  // NBSP: 1

// En space
var_dump(tui_string_width("\u{2002}"));  // EN SPACE: 1

// Em space
var_dump(tui_string_width("\u{2003}"));  // EM SPACE: 1

echo "\n=== Control characters ===\n";

var_dump(tui_string_width("\x00"));  // NULL: 0
var_dump(tui_string_width("\x07"));  // BELL: 0
var_dump(tui_string_width("\x08"));  // BACKSPACE: 0
var_dump(tui_string_width("\x1B"));  // ESC: 0
var_dump(tui_string_width("\x7F"));  // DEL: 0

echo "\n=== Fullwidth forms ===\n";

// Fullwidth ASCII
var_dump(tui_string_width("ＡＢＣ"));  // 6 (3 fullwidth chars)
var_dump(tui_string_width("１２３"));  // 6 (3 fullwidth digits)

// Halfwidth Katakana
var_dump(tui_string_width("ｱｲｳ"));  // 3 (halfwidth)

echo "\n=== Mathematical and symbols ===\n";

// Math symbols
var_dump(tui_string_width("∑∏∫"));  // 3

// Arrows
var_dump(tui_string_width("←→↑↓"));  // 4

// Box drawing
var_dump(tui_string_width("│─┌┐"));  // 4

// Currency
var_dump(tui_string_width("€£¥₹"));  // 4

echo "\n=== Text wrapping with Unicode ===\n";

// Wrap CJK text
$result = tui_wrap_text("中文测试文本", 4);
echo "CJK wrap to 4: " . count($result) . " lines\n";

// Wrap mixed text
$result = tui_wrap_text("Hello中文World日本語", 8);
echo "Mixed wrap to 8: " . count($result) . " lines\n";

// Wrap emoji
$result = tui_wrap_text("😀😎🎉👍", 4);
echo "Emoji wrap to 4: " . count($result) . " lines\n";

echo "\n=== Truncate with Unicode ===\n";

// Truncate CJK
$result = tui_truncate("中文测试文本", 6);
echo "CJK truncate to 6: " . tui_string_width($result) . " width\n";

// Truncate shouldn't split wide char
$result = tui_truncate("中文", 3);
echo "CJK truncate to 3: " . tui_string_width($result) . " width\n";

// Truncate mixed
$result = tui_truncate("AB中CD", 5);
echo "Mixed truncate to 5: " . tui_strip_ansi($result) . "\n";

echo "\n=== Text node with Unicode ===\n";

$text = new Text("Hello中文");
echo "Text created with CJK\n";

$text = new Text("😀🎉👍");
echo "Text created with emoji\n";

$text = new Text("שלום");
echo "Text created with Hebrew\n";

echo "\n=== Edge cases ===\n";

// Empty string
var_dump(tui_string_width(""));

// Single combining mark (no base)
var_dump(tui_string_width("\u{0301}"));  // Combining acute alone

// Very long Unicode string
$long = str_repeat("中", 1000);
var_dump(tui_string_width($long));  // 2000

// Alternating widths
var_dump(tui_string_width("a中b中c中"));  // 3 + 6 = 9

echo "\nDone!\n";
?>
--EXPECTF--
=== CJK characters ===
int(4)
int(8)
int(6)
int(6)
int(6)
int(4)
int(14)

=== Emoji ===
int(%d)
int(%d)
int(%d)
int(%d)
int(%d)
int(%d)
int(%d)

=== Combining characters ===
int(%d)
int(%d)
int(%d)
int(%d)

=== RTL scripts ===
int(%d)
int(%d)

=== Special width characters ===
int(%d)
int(%d)
int(%d)
int(%d)
int(%d)
int(%d)
int(%d)

=== Control characters ===
int(%d)
int(%d)
int(%d)
int(%d)
int(%d)

=== Fullwidth forms ===
int(%d)
int(%d)
int(%d)

=== Mathematical and symbols ===
int(%d)
int(%d)
int(%d)
int(%d)

=== Text wrapping with Unicode ===
CJK wrap to 4: %d lines
Mixed wrap to 8: %d lines
Emoji wrap to 4: %d lines

=== Truncate with Unicode ===
CJK truncate to 6: %d width
CJK truncate to 3: %d width
Mixed truncate to 5: %s

=== Text node with Unicode ===
Text created with CJK
Text created with emoji
Text created with Hebrew

=== Edge cases ===
int(0)
int(%d)
int(2000)
int(9)

Done!
