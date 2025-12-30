--TEST--
Unicode emoji width handling
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Tests emoji width calculations.
 * Modern terminals typically display emoji as width 2.
 *
 * See: https://www.unicode.org/reports/tr51/
 */

// === Basic emoji (width 2 each) ===
echo "Smile 😀 (U+1F600): " . tui_string_width("😀") . "\n";
echo "Heart ❤ (U+2764): " . tui_string_width("❤") . "\n";
echo "Star ⭐ (U+2B50): " . tui_string_width("⭐") . "\n";
echo "Fire 🔥 (U+1F525): " . tui_string_width("🔥") . "\n";
echo "Rocket 🚀 (U+1F680): " . tui_string_width("🚀") . "\n";

// === Emoji with skin tone modifiers ===
// Base emoji + modifier should still be width 2 (combines into one)
echo "Waving hand 👋 (U+1F44B): " . tui_string_width("👋") . "\n";
echo "Waving hand light 👋🏻: " . tui_string_width("👋🏻") . "\n"; // base + modifier
echo "Waving hand dark 👋🏿: " . tui_string_width("👋🏿") . "\n";

// === ZWJ sequences ===
// Family: man + ZWJ + woman + ZWJ + girl should be width 2 (one combined glyph)
echo "Family 👨‍👩‍👧 (ZWJ): " . tui_string_width("👨‍👩‍👧") . "\n";

// Heart with gender: woman + ZWJ + heart + ZWJ + woman
echo "Couple 👩‍❤️‍👩 (ZWJ): " . tui_string_width("👩‍❤️‍👩") . "\n";

// Rainbow flag: flag + ZWJ + rainbow
echo "Rainbow flag 🏳️‍🌈: " . tui_string_width("🏳️‍🌈") . "\n";

// === Variation selectors ===
// VS16 (U+FE0F) forces emoji presentation
// VS15 (U+FE0E) forces text presentation
echo "Heart emoji ❤️ (with VS16): " . tui_string_width("❤️") . "\n";
echo "Heart text ❤︎ (with VS15): " . tui_string_width("❤︎") . "\n";

// === Flag sequences (Regional Indicator pairs) ===
// Two regional indicators combine into a flag (width 2)
echo "US flag 🇺🇸: " . tui_string_width("🇺🇸") . "\n";
echo "JP flag 🇯🇵: " . tui_string_width("🇯🇵") . "\n";
echo "FR flag 🇫🇷: " . tui_string_width("🇫🇷") . "\n";

// === Keycap sequences ===
// Digit + VS16 + keycap combining (U+20E3)
echo "Keycap 1️⃣: " . tui_string_width("1️⃣") . "\n";
echo "Keycap 2️⃣: " . tui_string_width("2️⃣") . "\n";

// === Multiple emoji in string ===
echo "Three emoji 🔥🚀⭐: " . tui_string_width("🔥🚀⭐") . "\n"; // 6
echo "Emoji with text Hello 👋: " . tui_string_width("Hello 👋") . "\n"; // 5 + 1 + 2 = 8

// === Edge cases ===
// Single regional indicator (incomplete flag)
$ri_u = "\xF0\x9F\x87\xBA"; // U+1F1FA (Regional Indicator U)
echo "Single regional indicator: " . tui_string_width($ri_u) . "\n"; // width 2

// Three regional indicators (one flag + one dangling)
$ri_s = "\xF0\x9F\x87\xB8"; // U+1F1F8 (Regional Indicator S)
$ri_a = "\xF0\x9F\x87\xA6"; // U+1F1E6 (Regional Indicator A)
echo "Three regional indicators (US + A): " . tui_string_width($ri_u . $ri_s . $ri_a) . "\n"; // 2 + 2 = 4

echo "All emoji tests completed.\n";
?>
--EXPECT--
Smile 😀 (U+1F600): 2
Heart ❤ (U+2764): 1
Star ⭐ (U+2B50): 1
Fire 🔥 (U+1F525): 2
Rocket 🚀 (U+1F680): 2
Waving hand 👋 (U+1F44B): 2
Waving hand light 👋🏻: 2
Waving hand dark 👋🏿: 2
Family 👨‍👩‍👧 (ZWJ): 2
Couple 👩‍❤️‍👩 (ZWJ): 2
Rainbow flag 🏳️‍🌈: 2
Heart emoji ❤️ (with VS16): 2
Heart text ❤︎ (with VS15): 1
US flag 🇺🇸: 2
JP flag 🇯🇵: 2
FR flag 🇫🇷: 2
Keycap 1️⃣: 2
Keycap 2️⃣: 2
Three emoji 🔥🚀⭐: 5
Emoji with text Hello 👋: 8
Single regional indicator: 2
Three regional indicators (US + A): 4
All emoji tests completed.
