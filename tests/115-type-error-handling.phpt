--TEST--
Type error handling for all functions
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

echo "=== tui_string_width type errors ===\n";
try {
    tui_string_width(null);
} catch (TypeError $e) {
    echo "Caught: null\n";
}

try {
    tui_string_width([]);
} catch (TypeError $e) {
    echo "Caught: array\n";
}

try {
    tui_string_width(new stdClass());
} catch (TypeError $e) {
    echo "Caught: object\n";
}

echo "\n=== tui_wrap_text type errors ===\n";
try {
    tui_wrap_text(null, 10);
} catch (TypeError $e) {
    echo "Caught: null text\n";
}

try {
    tui_wrap_text("hello", "ten");
} catch (TypeError $e) {
    echo "Caught: string width\n";
}

// Note: negative width doesn't throw error, it's handled gracefully

echo "\n=== tui_truncate type errors ===\n";
try {
    tui_truncate(null, 10);
} catch (TypeError $e) {
    echo "Caught: null text\n";
}

try {
    tui_truncate("hello", "ten");
} catch (TypeError $e) {
    echo "Caught: string width\n";
}

echo "\n=== Box constructor type errors ===\n";
try {
    new ContainerNode("not an array");
} catch (TypeError $e) {
    echo "Caught: string props\n";
}

try {
    new ContainerNode(123);
} catch (TypeError $e) {
    echo "Caught: int props\n";
}

echo "\n=== Box property type coercion ===\n";
// These should work with coercion
$box = new ContainerNode(['width' => "100"]);  // string to int
echo "String width coerced\n";

$box = new ContainerNode(['width' => 50.5]);  // float to int
echo "Float width coerced\n";

echo "\n=== Text constructor type errors ===\n";
try {
    new ContentNode([]);  // array instead of string
} catch (TypeError $e) {
    echo "Caught: array content\n";
}

try {
    new ContentNode(new stdClass());
} catch (TypeError $e) {
    echo "Caught: object content\n";
}

echo "\n=== addChild type errors ===\n";
$box = new ContainerNode();
try {
    $box->addChild("not a node");
} catch (TypeError $e) {
    echo "Caught: string child\n";
}

try {
    $box->addChild(123);
} catch (TypeError $e) {
    echo "Caught: int child\n";
}

try {
    $box->addChild(null);
} catch (TypeError $e) {
    echo "Caught: null child\n";
}

echo "\n=== tui_buffer_create type errors ===\n";
try {
    $buffer = tui_buffer_create("80", 24);
    echo "String width coerced\n";
} catch (TypeError $e) {
    echo "Caught: string width\n";
}

try {
    $buffer = tui_buffer_create(80, "24");
    echo "String height coerced\n";
} catch (TypeError $e) {
    echo "Caught: string height\n";
}

echo "\n=== tui_history_create type errors ===\n";
try {
    $history = tui_history_create("100");
    echo "String capacity coerced\n";
} catch (TypeError $e) {
    echo "Caught: string capacity\n";
}

echo "\n=== tui_test_create type errors ===\n";
try {
    $renderer = tui_test_create("80", 24);
    echo "String width coerced\n";
} catch (TypeError $e) {
    echo "Caught: string width\n";
}

echo "\n=== Function argument count ===\n";
try {
    tui_string_width();  // missing required arg
} catch (ArgumentCountError $e) {
    echo "Caught: missing string_width arg\n";
}

try {
    tui_wrap_text("hello");  // missing width arg
} catch (ArgumentCountError $e) {
    echo "Caught: missing wrap_text width\n";
}

try {
    tui_truncate("hello");  // missing width arg
} catch (ArgumentCountError $e) {
    echo "Caught: missing truncate width\n";
}

try {
    tui_buffer_create();  // missing both args
} catch (ArgumentCountError $e) {
    echo "Caught: missing buffer_create args\n";
}

echo "\nDone!\n";
?>
--EXPECTF--
=== tui_string_width type errors ===
%A
Caught: array
Caught: object

=== tui_wrap_text type errors ===
%A
Caught: string width

=== tui_truncate type errors ===
%A
Caught: string width

=== Box constructor type errors ===
Caught: string props
Caught: int props

=== Box property type coercion ===
String width coerced
Float width coerced

=== Text constructor type errors ===
Caught: array content
Caught: object content

=== addChild type errors ===
Caught: string child
Caught: int child
Caught: null child

=== tui_buffer_create type errors ===
String width coerced
String height coerced

=== tui_history_create type errors ===
String capacity coerced

=== tui_test_create type errors ===
String width coerced

=== Function argument count ===
Caught: missing string_width arg
Caught: missing wrap_text width
Caught: missing truncate width
Caught: missing buffer_create args

Done!
