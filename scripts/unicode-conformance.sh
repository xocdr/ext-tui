#!/bin/bash
#
# Unicode Conformance Testing Script
# Downloads Unicode test data and runs conformance tests
#
# Requires: curl, php with tui extension
#
# This script tests ext-tui's character width implementation against:
# - EastAsianWidth.txt (character width classifications)
# - emoji-test.txt (emoji sequences)
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
DATA_DIR="$PROJECT_DIR/.unicode-test-data"
UNICODE_VERSION="15.1.0"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=== ext-tui Unicode Conformance Test ==="
echo "Unicode Version: $UNICODE_VERSION"
echo ""

# Create data directory
mkdir -p "$DATA_DIR"

# Download EastAsianWidth.txt if needed
EAW_FILE="$DATA_DIR/EastAsianWidth.txt"
if [ ! -f "$EAW_FILE" ]; then
    echo "Downloading EastAsianWidth.txt..."
    curl -sS -o "$EAW_FILE" \
        "https://www.unicode.org/Public/$UNICODE_VERSION/ucd/EastAsianWidth.txt"
fi

# Download emoji-test.txt if needed
EMOJI_FILE="$DATA_DIR/emoji-test.txt"
if [ ! -f "$EMOJI_FILE" ]; then
    echo "Downloading emoji-test.txt..."
    curl -sS -o "$EMOJI_FILE" \
        "https://www.unicode.org/Public/emoji/15.1/emoji-test.txt"
fi

echo "Test data ready in $DATA_DIR"
echo ""

# Create test PHP script
TEST_SCRIPT=$(mktemp)
cat > "$TEST_SCRIPT" << 'PHPCODE'
<?php
/**
 * Unicode width conformance test runner
 * Tests tui_string_width() against Unicode EastAsianWidth data
 */

$dataDir = $argv[1] ?? '.unicode-test-data';

// Parse EastAsianWidth.txt
function parseEastAsianWidth($file) {
    $widths = [];
    $lines = file($file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

    foreach ($lines as $line) {
        // Skip comments
        if (strpos($line, '#') === 0 || trim($line) === '') continue;

        // Format: XXXX;W or XXXX..YYYY;W (with optional spaces around semicolon)
        if (preg_match('/^([0-9A-F]+)(?:\.\.([0-9A-F]+))?\s*;\s*(\w+)/', $line, $m)) {
            $start = hexdec($m[1]);
            $end = isset($m[2]) && $m[2] !== '' ? hexdec($m[2]) : $start;
            $category = $m[3];

            for ($cp = $start; $cp <= $end; $cp++) {
                $widths[$cp] = $category;
            }
        }
    }
    return $widths;
}

// Get expected width from EAW category
function expectedWidth($category) {
    switch ($category) {
        case 'F':  // Fullwidth
        case 'W':  // Wide
            return 2;
        case 'H':  // Halfwidth
        case 'Na': // Narrow
        case 'N':  // Neutral (treat as narrow in Western context)
            return 1;
        case 'A':  // Ambiguous - typically 1 in Western context
            return 1;
        default:
            return 1;
    }
}

// Convert codepoint to UTF-8 string
function codepointToUtf8($cp) {
    if ($cp < 0x80) {
        return chr($cp);
    } elseif ($cp < 0x800) {
        return chr(0xC0 | ($cp >> 6)) . chr(0x80 | ($cp & 0x3F));
    } elseif ($cp < 0x10000) {
        return chr(0xE0 | ($cp >> 12)) .
               chr(0x80 | (($cp >> 6) & 0x3F)) .
               chr(0x80 | ($cp & 0x3F));
    } else {
        return chr(0xF0 | ($cp >> 18)) .
               chr(0x80 | (($cp >> 12) & 0x3F)) .
               chr(0x80 | (($cp >> 6) & 0x3F)) .
               chr(0x80 | ($cp & 0x3F));
    }
}

echo "=== East Asian Width Test ===\n";

$eawFile = "$dataDir/EastAsianWidth.txt";
if (!file_exists($eawFile)) {
    echo "ERROR: $eawFile not found\n";
    exit(1);
}

$widths = parseEastAsianWidth($eawFile);
echo "Loaded " . count($widths) . " codepoint width mappings\n";

// Sample test points (testing all would be too slow)
$testPoints = [
    // ASCII
    0x0041 => 'A',
    0x007A => 'z',

    // Latin Extended
    0x00E9 => 'é',
    0x00F1 => 'ñ',

    // CJK
    0x4E2D => '中',
    0x65E5 => '日',
    0x672C => '本',

    // Hangul
    0xD55C => '한',
    0xAE00 => '글',

    // Hiragana/Katakana
    0x3042 => 'あ',
    0x30A2 => 'ア',

    // Fullwidth
    0xFF21 => 'Ａ',
    0xFF10 => '０',

    // Halfwidth Katakana
    0xFF71 => 'ｱ',

    // CJK Extension B (4-byte UTF-8)
    0x20000 => '𠀀',
];

$passed = 0;
$failed = 0;
$skipped = 0;

foreach ($testPoints as $cp => $desc) {
    $char = codepointToUtf8($cp);
    $actual = tui_string_width($char);
    $category = $widths[$cp] ?? 'N';
    $expected = expectedWidth($category);

    // Special cases: our implementation may differ from strict EAW
    // - Emoji ranges in 0x2600-0x26FF are ambiguous but we use width 1
    // - Some symbols are ambiguous
    $isAmbiguous = ($category === 'A');

    if ($actual === $expected) {
        $passed++;
        echo "\033[32m✓\033[0m U+" . sprintf('%04X', $cp) . " ($desc) = $actual (EAW: $category)\n";
    } elseif ($isAmbiguous) {
        $skipped++;
        echo "\033[33m~\033[0m U+" . sprintf('%04X', $cp) . " ($desc) = $actual (EAW: $category = ambiguous, expected $expected)\n";
    } else {
        $failed++;
        echo "\033[31m✗\033[0m U+" . sprintf('%04X', $cp) . " ($desc) = $actual (expected $expected, EAW: $category)\n";
    }
}

echo "\n";
echo "Results: $passed passed, $failed failed, $skipped ambiguous\n";

// === Emoji Sequence Tests ===
echo "\n=== Emoji Sequence Test ===\n";

$emojiTests = [
    ['😀', 2, 'Grinning Face'],
    ['👋', 2, 'Waving Hand'],
    ['👋🏻', 2, 'Waving Hand: Light Skin'],
    ['👋🏿', 2, 'Waving Hand: Dark Skin'],
    ['🇺🇸', 2, 'Flag: US'],
    ['🇯🇵', 2, 'Flag: JP'],
    ['👨‍👩‍👧', 2, 'Family MWG (ZWJ)'],
    ['❤️', 2, 'Red Heart (with VS16)'],
];

$emojiPassed = 0;
$emojiFailed = 0;

foreach ($emojiTests as [$emoji, $expected, $name]) {
    $actual = tui_string_width($emoji);
    if ($actual === $expected) {
        $emojiPassed++;
        echo "\033[32m✓\033[0m $emoji ($name) = $actual\n";
    } else {
        $emojiFailed++;
        echo "\033[31m✗\033[0m $emoji ($name) = $actual (expected $expected)\n";
    }
}

echo "\n";
echo "Emoji Results: $emojiPassed passed, $emojiFailed failed\n";

// Overall result
$totalFailed = $failed + $emojiFailed;
if ($totalFailed > 0) {
    echo "\n\033[31mFAILED: $totalFailed test(s) failed\033[0m\n";
    exit(1);
} else {
    echo "\n\033[32mPASSED: All tests passed\033[0m\n";
    exit(0);
}
PHPCODE

# Run the test
echo "Running conformance tests..."
echo ""

php -n -d extension="$PROJECT_DIR/modules/tui.so" "$TEST_SCRIPT" "$DATA_DIR"
RESULT=$?

# Cleanup
rm -f "$TEST_SCRIPT"

exit $RESULT
