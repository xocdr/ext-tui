--TEST--
Unicode width conformance - East Asian Width property
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Tests character width calculations against expected East Asian Width values.
 * See: https://www.unicode.org/reports/tr11/
 *
 * Width categories:
 * - Narrow (Na): ASCII, Latin, Greek, etc. -> width 1
 * - Wide (W): CJK characters -> width 2
 * - Fullwidth (F): Fullwidth ASCII variants -> width 2
 * - Halfwidth (H): Halfwidth Katakana -> width 1
 * - Ambiguous (A): Characters with context-dependent width -> typically 1 in Western context
 */

// === Narrow characters (width 1) ===

// ASCII letters and digits
echo "ASCII 'A': " . tui_string_width("A") . "\n";
echo "ASCII '0': " . tui_string_width("0") . "\n";
echo "ASCII 'z': " . tui_string_width("z") . "\n";

// Latin Extended
echo "Latin 'é' (U+00E9): " . tui_string_width("é") . "\n";
echo "Latin 'ñ' (U+00F1): " . tui_string_width("ñ") . "\n";

// Greek
echo "Greek 'α' (U+03B1): " . tui_string_width("α") . "\n";
echo "Greek 'Ω' (U+03A9): " . tui_string_width("Ω") . "\n";

// Cyrillic
echo "Cyrillic 'Я' (U+042F): " . tui_string_width("Я") . "\n";

// === Wide characters (width 2) ===

// CJK Ideographs
echo "CJK '中' (U+4E2D): " . tui_string_width("中") . "\n";
echo "CJK '日' (U+65E5): " . tui_string_width("日") . "\n";
echo "CJK '本' (U+672C): " . tui_string_width("本") . "\n";

// Hangul Syllables
echo "Hangul '한' (U+D55C): " . tui_string_width("한") . "\n";
echo "Hangul '글' (U+AE00): " . tui_string_width("글") . "\n";

// Hiragana
echo "Hiragana 'あ' (U+3042): " . tui_string_width("あ") . "\n";

// Katakana
echo "Katakana 'ア' (U+30A2): " . tui_string_width("ア") . "\n";

// === Fullwidth characters (width 2) ===

// Fullwidth ASCII
echo "Fullwidth 'Ａ' (U+FF21): " . tui_string_width("Ａ") . "\n";
echo "Fullwidth '０' (U+FF10): " . tui_string_width("０") . "\n";

// === Halfwidth characters (width 1) ===

// Halfwidth Katakana
echo "Halfwidth 'ｱ' (U+FF71): " . tui_string_width("ｱ") . "\n";

// === Zero-width characters ===

// Combining marks
echo "Combining acute (U+0301): " . tui_string_width("\xCC\x81") . "\n";

// Zero-width joiner
echo "ZWJ (U+200D): " . tui_string_width("\xE2\x80\x8D") . "\n";

// === Control characters (width 0) ===
echo "Control NUL: " . tui_string_width("\x00") . "\n";
echo "Control TAB: " . tui_string_width("\x09") . "\n";
echo "Control DEL: " . tui_string_width("\x7F") . "\n";

// === String combinations ===
echo "Mixed 'Hello世界': " . tui_string_width("Hello世界") . "\n"; // 5 + 4 = 9
echo "Mixed 'ABCあいう': " . tui_string_width("ABCあいう") . "\n"; // 3 + 6 = 9

// === CJK Extension ranges ===
echo "CJK Ext-B '𠀀' (U+20000): " . tui_string_width("𠀀") . "\n"; // 4-byte UTF-8, width 2

echo "All tests completed.\n";
?>
--EXPECT--
ASCII 'A': 1
ASCII '0': 1
ASCII 'z': 1
Latin 'é' (U+00E9): 1
Latin 'ñ' (U+00F1): 1
Greek 'α' (U+03B1): 1
Greek 'Ω' (U+03A9): 1
Cyrillic 'Я' (U+042F): 1
CJK '中' (U+4E2D): 2
CJK '日' (U+65E5): 2
CJK '本' (U+672C): 2
Hangul '한' (U+D55C): 2
Hangul '글' (U+AE00): 2
Hiragana 'あ' (U+3042): 2
Katakana 'ア' (U+30A2): 2
Fullwidth 'Ａ' (U+FF21): 2
Fullwidth '０' (U+FF10): 2
Halfwidth 'ｱ' (U+FF71): 1
Combining acute (U+0301): 0
ZWJ (U+200D): 0
Control NUL: 0
Control TAB: 0
Control DEL: 0
Mixed 'Hello世界': 9
Mixed 'ABCあいう': 9
CJK Ext-B '𠀀' (U+20000): 2
All tests completed.
