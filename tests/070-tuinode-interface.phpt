--TEST--
TuiNode interface with getKey() and getId() methods
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Test the TuiNode interface and its implementations in Box and Text.
 */

use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;
use Xocdr\Tui\Ext\TuiNode;

echo "Test 1: TuiNode interface exists\n";
var_dump(interface_exists(TuiNode::class));

echo "\nTest 2: ContainerNode implements TuiNode\n";
$box = new ContainerNode();
var_dump($box instanceof TuiNode);

echo "\nTest 3: ContentNode implements TuiNode\n";
$text = new ContentNode("hello");
var_dump($text instanceof TuiNode);

echo "\nTest 4: ContainerNode getKey() returns null when not set\n";
$box = new ContainerNode();
var_dump($box->getKey());

echo "\nTest 5: ContainerNode getKey() returns value when set\n";
$box = new ContainerNode(['key' => 'my-box-key']);
var_dump($box->getKey());

echo "\nTest 6: ContainerNode getId() returns null when not set\n";
$box = new ContainerNode();
var_dump($box->getId());

echo "\nTest 7: ContainerNode getId() returns value when set\n";
$box = new ContainerNode(['id' => 'my-box-id']);
var_dump($box->getId());

echo "\nTest 8: ContentNode getKey() returns null when not set\n";
$text = new ContentNode("hello");
var_dump($text->getKey());

echo "\nTest 9: ContentNode getKey() returns value when set\n";
$text = new ContentNode("hello", ['key' => 'my-text-key']);
var_dump($text->getKey());

echo "\nTest 10: ContentNode getId() returns null when not set\n";
$text = new ContentNode("hello");
var_dump($text->getId());

echo "\nTest 11: ContentNode getId() returns value when set\n";
$text = new ContentNode("hello", ['id' => 'my-text-id']);
var_dump($text->getId());

echo "\nTest 12: Type hints work with TuiNode interface\n";
function processNode(TuiNode $node): string {
    return sprintf("key=%s, id=%s",
        $node->getKey() ?? 'null',
        $node->getId() ?? 'null');
}
echo processNode(new ContainerNode(['key' => 'box1', 'id' => 'box-id'])) . "\n";
echo processNode(new ContentNode("text", ['key' => 'text1'])) . "\n";

echo "\nTuiNode interface test completed!\n";
?>
--EXPECT--
Test 1: TuiNode interface exists
bool(true)

Test 2: ContainerNode implements TuiNode
bool(true)

Test 3: ContentNode implements TuiNode
bool(true)

Test 4: ContainerNode getKey() returns null when not set
NULL

Test 5: ContainerNode getKey() returns value when set
string(10) "my-box-key"

Test 6: ContainerNode getId() returns null when not set
NULL

Test 7: ContainerNode getId() returns value when set
string(9) "my-box-id"

Test 8: ContentNode getKey() returns null when not set
NULL

Test 9: ContentNode getKey() returns value when set
string(11) "my-text-key"

Test 10: ContentNode getId() returns null when not set
NULL

Test 11: ContentNode getId() returns value when set
string(10) "my-text-id"

Test 12: Type hints work with TuiNode interface
key=box1, id=box-id
key=text1, id=null

TuiNode interface test completed!
