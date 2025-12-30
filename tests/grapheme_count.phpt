--TEST--
Grapheme cluster counting
--EXTENSIONS--
tui
--FILE--
<?php

// Test function exists
var_dump(function_exists('tui_grapheme_count'));

// ASCII
var_dump(tui_grapheme_count("hello") === 5);

// CJK characters (each is a separate grapheme)
var_dump(tui_grapheme_count("你好世界") === 4);

// Family emoji (ZWJ sequence - single grapheme)
$family = "\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7\xE2\x80\x8D\xF0\x9F\x91\xA6"; // 👨‍👩‍👧‍👦
var_dump(tui_grapheme_count($family) === 1);

// Flag emoji (regional indicator pair - single grapheme)
$flag = "\xF0\x9F\x87\xBA\xF0\x9F\x87\xB8"; // 🇺🇸
var_dump(tui_grapheme_count($flag) === 1);

// e with combining acute accent (single grapheme)
$eAccent = "e\xCC\x81"; // e + combining acute
var_dump(tui_grapheme_count($eAccent) === 1);

// Wave with skin tone modifier (single grapheme)
$wave = "\xF0\x9F\x91\x8B\xF0\x9F\x8F\xBD"; // 👋🏽
var_dump(tui_grapheme_count($wave) === 1);

// Empty string
var_dump(tui_grapheme_count("") === 0);

// Mixed content
var_dump(tui_grapheme_count("Hello 👨‍👩‍👧‍👦!") === 8);

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
OK
