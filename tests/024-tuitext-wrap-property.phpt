--TEST--
Xocdr\Tui\Ext\ContentNode has wrap property
--SKIPIF--
<?php if (!extension_loaded('tui')) die('skip tui extension not loaded'); ?>
--FILE--
<?php
use Xocdr\Tui\Ext\ContentNode;

$text = new ContentNode();
var_dump(property_exists($text, 'wrap'));
var_dump($text->wrap);

// Test setting wrap mode
$wrapped = new ContentNode('Hello', ['wrap' => 'word']);
var_dump($wrapped->wrap);

$charWrap = new ContentNode('Hello', ['wrap' => 'char']);
var_dump($charWrap->wrap);
?>
--EXPECT--
bool(true)
NULL
string(4) "word"
string(4) "char"
