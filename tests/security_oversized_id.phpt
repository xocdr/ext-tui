--TEST--
Security: Reject oversized node IDs
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\ValidationException;

// TUI_MAX_ID_LENGTH is 256 bytes
$maxLen = 256;

// Test at exactly max length - should work
$id = str_repeat('i', $maxLen);
$node = new ContainerNode(['id' => $id]);
var_dump($node->getId() === $id);
echo "Max length id accepted\n";

// Test just over max length - should throw exception
$oversized = str_repeat('i', $maxLen + 1);
try {
    $node = new ContainerNode(['id' => $oversized]);
    echo "ERROR: Should have thrown exception\n";
} catch (ValidationException $e) {
    echo "Exception caught: " . (strpos($e->getMessage(), 'maximum length') !== false ? 'correct message' : 'wrong message') . "\n";
}

// Test with setId method
$node2 = new ContainerNode();
try {
    $node2->setId($oversized);
    echo "ERROR: setId should have thrown exception\n";
} catch (ValidationException $e) {
    echo "setId throws exception\n";
}

echo "Done\n";
?>
--EXPECT--
bool(true)
Max length id accepted
Exception caught: correct message
setId throws exception
Done
