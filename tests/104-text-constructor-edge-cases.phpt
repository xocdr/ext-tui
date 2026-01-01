--TEST--
Text constructor edge cases and property validation
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContentNode;

echo "=== Empty content ===\n";
$text = new ContentNode("");
var_dump($text->content);

echo "\n=== NULL-like content ===\n";
$text = new ContentNode("0");
var_dump($text->content);

echo "\n=== Unicode content ===\n";
$text = new ContentNode("Hello 你好 مرحبا 🎉");
echo "Content set: " . (strlen($text->content) > 0 ? 'yes' : 'no') . "\n";

echo "\n=== Very long content ===\n";
$long = str_repeat("Lorem ipsum dolor sit amet. ", 1000);
$text = new ContentNode($long);
echo "Length: " . strlen($text->content) . "\n";

echo "\n=== All style properties ===\n";
$text = new ContentNode("Styled", [
    'bold' => true,
    'italic' => true,
    'underline' => true,
    'dim' => true,
    'inverse' => true,
    'strikethrough' => true,
]);
var_dump($text->bold);
var_dump($text->italic);
var_dump($text->underline);
var_dump($text->dim);
var_dump($text->inverse);
var_dump($text->strikethrough);

echo "\n=== Color formats ===\n";

// Hex colors
$text = new ContentNode("Red", ['color' => '#ff0000']);
echo "Hex color set\n";

// RGB array
$text = new ContentNode("Green", ['color' => [0, 255, 0]]);
echo "RGB array color set\n";

// Background color
$text = new ContentNode("Blue bg", ['backgroundColor' => '#0000ff']);
echo "Background color set\n";

echo "\n=== Wrap modes ===\n";
foreach (['none', 'word', 'char', 'word-char'] as $mode) {
    $text = new ContentNode("Wrapped text", ['wrap' => $mode]);
    echo "wrap '$mode': " . $text->wrap . "\n";
}

echo "\n=== Key and ID ===\n";
$text = new ContentNode("Keyed", [
    'key' => 'text-key',
    'id' => 'text-id',
]);
echo "key: " . $text->key . "\n";
echo "id: " . $text->id . "\n";

echo "\n=== Content only (no properties) ===\n";
$text = new ContentNode("Just content");
var_dump($text->content);

echo "\n=== Properties only (empty content) ===\n";
$text = new ContentNode("", ['bold' => true, 'color' => '#ff0000']);
var_dump($text->content);
var_dump($text->bold);

echo "\n=== Special characters ===\n";
$text = new ContentNode("<script>alert('xss')</script>");
echo "Special chars: " . (strlen($text->content) > 0 ? 'yes' : 'no') . "\n";

$text = new ContentNode("Line1\nLine2\tTabbed");
echo "Newlines and tabs: " . (strlen($text->content) > 0 ? 'yes' : 'no') . "\n";

echo "\n=== Binary content ===\n";
$text = new ContentNode("\x00\x01\x02\x03");
echo "Binary content length: " . strlen($text->content) . "\n";

echo "\n=== Numeric content ===\n";
$text = new ContentNode("12345");
var_dump($text->content);

echo "\n=== Boolean-like content ===\n";
$text = new ContentNode("true");
var_dump($text->content);
$text = new ContentNode("false");
var_dump($text->content);

echo "\nDone!\n";
?>
--EXPECT--
=== Empty content ===
string(0) ""

=== NULL-like content ===
string(1) "0"

=== Unicode content ===
Content set: yes

=== Very long content ===
Length: 28000

=== All style properties ===
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)

=== Color formats ===
Hex color set
RGB array color set
Background color set

=== Wrap modes ===
wrap 'none': none
wrap 'word': word
wrap 'char': char
wrap 'word-char': word-char

=== Key and ID ===
key: text-key
id: text-id

=== Content only (no properties) ===
string(12) "Just content"

=== Properties only (empty content) ===
string(0) ""
bool(true)

=== Special characters ===
Special chars: yes
Newlines and tabs: yes

=== Binary content ===
Binary content length: 0

=== Numeric content ===
string(5) "12345"

=== Boolean-like content ===
string(4) "true"
string(5) "false"

Done!
