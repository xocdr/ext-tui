--TEST--
Xocdr\Tui\Ext\Text has wrap property
--SKIPIF--
<?php if (!extension_loaded('tui')) die('skip tui extension not loaded'); ?>
--FILE--
<?php
use Xocdr\Tui\Ext\Text;

$text = new Text();
var_dump(property_exists($text, 'wrap'));
var_dump($text->wrap);

// Test setting wrap mode
$wrapped = new Text('Hello', ['wrap' => 'word']);
var_dump($wrapped->wrap);

$charWrap = new Text('Hello', ['wrap' => 'char']);
var_dump($charWrap->wrap);
?>
--EXPECT--
bool(true)
NULL
string(4) "word"
string(4) "char"
