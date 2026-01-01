--TEST--
Security: Reject oversized node keys
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\ValidationException;

// TUI_MAX_KEY_LENGTH is 256 bytes
$maxLen = 256;

// Test at exactly max length - should work
$key = str_repeat('k', $maxLen);
$node = new ContainerNode(['key' => $key]);
var_dump($node->getKey() === $key);
echo "Max length key accepted\n";

// Test just over max length - should throw exception
$oversized = str_repeat('k', $maxLen + 1);
try {
    $node = new ContainerNode(['key' => $oversized]);
    echo "ERROR: Should have thrown exception\n";
} catch (ValidationException $e) {
    echo "Exception caught: " . (strpos($e->getMessage(), 'maximum length') !== false ? 'correct message' : 'wrong message') . "\n";
}

// Test with setKey method
$node2 = new ContainerNode();
try {
    $node2->setKey($oversized);
    echo "ERROR: setKey should have thrown exception\n";
} catch (ValidationException $e) {
    echo "setKey throws exception\n";
}

echo "Done\n";
?>
--EXPECT--
bool(true)
Max length key accepted
Exception caught: correct message
setKey throws exception
Done
