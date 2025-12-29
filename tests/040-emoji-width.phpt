--TEST--
Emoji width calculation with ZWJ sequences and modifiers
--EXTENSIONS--
tui
--FILE--
<?php
// Basic emoji - width 2
echo "Simple emoji: " . tui_string_width("😀") . "\n";

// Emoji with skin tone modifier - should be width 2 (single character)
echo "With skin tone: " . tui_string_width("👋🏽") . "\n";

// ZWJ family sequence - multiple emoji joined by ZWJ render as one
// 👨‍👩‍👧 = man + ZWJ + woman + ZWJ + girl
// Should be width 2 (single composite character)
echo "ZWJ family: " . tui_string_width("👨‍👩‍👧") . "\n";

// Another ZWJ sequence: man technologist 👨‍💻 = man + ZWJ + laptop
echo "Man technologist: " . tui_string_width("👨‍💻") . "\n";

// Flag (two regional indicators combine to one flag)
// 🇺🇸 = U+1F1FA + U+1F1F8
echo "US Flag: " . tui_string_width("🇺🇸") . "\n";

// Multiple flags
echo "Two flags: " . tui_string_width("🇺🇸🇬🇧") . "\n";

// Mixed: emoji + text
echo "Emoji + text: " . tui_string_width("Hi 😀!") . "\n";

// Variation selector (VS16 makes emoji presentation)
// Heart ❤️ = ❤ + VS16
echo "Heart with VS16: " . tui_string_width("❤️") . "\n";

// Multiple simple emoji
echo "Three emoji: " . tui_string_width("😀😎🎉") . "\n";

echo "Done\n";
?>
--EXPECT--
Simple emoji: 2
With skin tone: 2
ZWJ family: 2
Man technologist: 2
US Flag: 2
Two flags: 4
Emoji + text: 6
Heart with VS16: 2
Three emoji: 6
Done
