--TEST--
TuiBox::addChild() adds children
--EXTENSIONS--
tui
--FILE--
<?php
$box = new TuiBox();
$text = new TuiText("Hello");

$result = $box->addChild($text);

// Should return self for chaining
var_dump($result === $box);

// Should have 1 child
var_dump(count($box->children));
?>
--EXPECT--
bool(true)
int(1)
