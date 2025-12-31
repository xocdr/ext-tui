--TEST--
TuiNode interface with getKey() and getId() methods
--EXTENSIONS--
tui
--FILE--
<?php
/**
 * Test the TuiNode interface and its implementations in Box and Text.
 */

use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;
use Xocdr\Tui\Ext\TuiNode;

echo "Test 1: TuiNode interface exists\n";
var_dump(interface_exists(TuiNode::class));

echo "\nTest 2: Box implements TuiNode\n";
$box = new Box();
var_dump($box instanceof TuiNode);

echo "\nTest 3: Text implements TuiNode\n";
$text = new Text("hello");
var_dump($text instanceof TuiNode);

echo "\nTest 4: Box getKey() returns null when not set\n";
$box = new Box();
var_dump($box->getKey());

echo "\nTest 5: Box getKey() returns value when set\n";
$box = new Box(['key' => 'my-box-key']);
var_dump($box->getKey());

echo "\nTest 6: Box getId() returns null when not set\n";
$box = new Box();
var_dump($box->getId());

echo "\nTest 7: Box getId() returns value when set\n";
$box = new Box(['id' => 'my-box-id']);
var_dump($box->getId());

echo "\nTest 8: Text getKey() returns null when not set\n";
$text = new Text("hello");
var_dump($text->getKey());

echo "\nTest 9: Text getKey() returns value when set\n";
$text = new Text("hello", ['key' => 'my-text-key']);
var_dump($text->getKey());

echo "\nTest 10: Text getId() returns null when not set\n";
$text = new Text("hello");
var_dump($text->getId());

echo "\nTest 11: Text getId() returns value when set\n";
$text = new Text("hello", ['id' => 'my-text-id']);
var_dump($text->getId());

echo "\nTest 12: Type hints work with TuiNode interface\n";
function processNode(TuiNode $node): string {
    return sprintf("key=%s, id=%s",
        $node->getKey() ?? 'null',
        $node->getId() ?? 'null');
}
echo processNode(new Box(['key' => 'box1', 'id' => 'box-id'])) . "\n";
echo processNode(new Text("text", ['key' => 'text1'])) . "\n";

echo "\nTuiNode interface test completed!\n";
?>
--EXPECT--
Test 1: TuiNode interface exists
bool(true)

Test 2: Box implements TuiNode
bool(true)

Test 3: Text implements TuiNode
bool(true)

Test 4: Box getKey() returns null when not set
NULL

Test 5: Box getKey() returns value when set
string(10) "my-box-key"

Test 6: Box getId() returns null when not set
NULL

Test 7: Box getId() returns value when set
string(9) "my-box-id"

Test 8: Text getKey() returns null when not set
NULL

Test 9: Text getKey() returns value when set
string(11) "my-text-key"

Test 10: Text getId() returns null when not set
NULL

Test 11: Text getId() returns value when set
string(10) "my-text-id"

Test 12: Type hints work with TuiNode interface
key=box1, id=box-id
key=text1, id=null

TuiNode interface test completed!
