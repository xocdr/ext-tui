--TEST--
Security: Reject oversized text content
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContentNode;
use Xocdr\Tui\ValidationException;

// TUI_MAX_TEXT_LENGTH is 1MB (1048576 bytes)
$maxLen = 1048576;

// Test at exactly max length - should work
$text = str_repeat('a', $maxLen);
$node = new ContentNode($text);
var_dump($node instanceof ContentNode);
echo "Max length text accepted\n";
unset($node);
unset($text);

// Test just over max length - should throw exception
$oversized = str_repeat('b', $maxLen + 1);
try {
    $node = new ContentNode($oversized);
    echo "ERROR: Should have thrown exception\n";
} catch (ValidationException $e) {
    echo "Exception caught: " . (strpos($e->getMessage(), 'maximum length') !== false ? 'correct message' : 'wrong message') . "\n";
}
unset($oversized);

// Test with huge string - should definitely throw
$huge = str_repeat('c', $maxLen * 2);
try {
    $node = new ContentNode($huge);
    echo "ERROR: Should have thrown exception\n";
} catch (ValidationException $e) {
    echo "Huge text rejected with exception\n";
}

echo "Done\n";
?>
--EXPECT--
bool(true)
Max length text accepted
Exception caught: correct message
Huge text rejected with exception
Done
