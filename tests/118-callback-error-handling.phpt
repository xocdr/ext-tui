--TEST--
Callback and event handler error handling
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

echo "=== Valid callbacks ===\n";

// Named function callback
function myCallback() {
    echo "Named function called\n";
}

// Closure callback
$closure = function() {
    echo "Closure called\n";
};

// Static method callback
class CallbackTest {
    public static function staticMethod() {
        echo "Static method called\n";
    }

    public function instanceMethod() {
        echo "Instance method called\n";
    }
}

echo "Callbacks defined\n";

echo "\n=== Box with onClick ===\n";

$clicked = false;
$box = new ContainerNode([
    'onClick' => function() use (&$clicked) {
        $clicked = true;
        echo "Box clicked\n";
    }
]);
echo "Box with onClick created\n";

echo "\n=== Box with onKeyPress ===\n";

$keyPressed = null;
$box = new ContainerNode([
    'onKeyPress' => function($key) use (&$keyPressed) {
        $keyPressed = $key;
        echo "Key pressed\n";
    }
]);
echo "Box with onKeyPress created\n";

echo "\n=== Text with onInput ===\n";

$input = '';
$text = new ContentNode('', [
    'onInput' => function($char) use (&$input) {
        $input .= $char;
    }
]);
echo "Text with onInput created\n";

echo "\n=== Invalid callback types ===\n";

// String that's not a function name
try {
    $box = new ContainerNode(['onClick' => 'not_a_function']);
    echo "String callback accepted (may fail at call time)\n";
} catch (TypeError $e) {
    echo "Caught: invalid string callback\n";
}

// Integer as callback
try {
    $box = new ContainerNode(['onClick' => 123]);
    echo "Int callback accepted (may fail at call time)\n";
} catch (TypeError $e) {
    echo "Caught: invalid int callback\n";
}

// Array that's not a valid callable
try {
    $box = new ContainerNode(['onClick' => ['not', 'valid']]);
    echo "Invalid array callback accepted (may fail at call time)\n";
} catch (TypeError $e) {
    echo "Caught: invalid array callback\n";
}

echo "\n=== Multiple event handlers ===\n";

$events = [];
$box = new ContainerNode([
    'onClick' => function() use (&$events) { $events[] = 'click'; },
    'onKeyPress' => function($k) use (&$events) { $events[] = 'keypress'; },
    'onFocus' => function() use (&$events) { $events[] = 'focus'; },
    'onBlur' => function() use (&$events) { $events[] = 'blur'; },
]);
echo "Box with multiple handlers created\n";

echo "\n=== Nested callbacks ===\n";

$outer = function() {
    $inner = function() {
        echo "Inner called\n";
    };
    return $inner;
};

$box = new ContainerNode([
    'onClick' => $outer()
]);
echo "Nested callback set\n";

echo "\n=== Callbacks with use() references ===\n";

$counter = 0;
$box = new ContainerNode([
    'onClick' => function() use (&$counter) {
        $counter++;
    }
]);
echo "Reference callback created, counter = $counter\n";

echo "\n=== Arrow function callbacks ===\n";

$value = 'test';
$box = new ContainerNode([
    'onClick' => fn() => $value
]);
echo "Arrow function callback set\n";

echo "\n=== Class method callbacks ===\n";

$obj = new CallbackTest();
$box = new ContainerNode([
    'onClick' => [$obj, 'instanceMethod']
]);
echo "Instance method callback set\n";

$box = new ContainerNode([
    'onClick' => [CallbackTest::class, 'staticMethod']
]);
echo "Static method callback set\n";

$box = new ContainerNode([
    'onClick' => 'CallbackTest::staticMethod'
]);
echo "Static method string callback set\n";

echo "\n=== First-class callable syntax ===\n";

$box = new ContainerNode([
    'onClick' => myCallback(...)
]);
echo "First-class callable set\n";

$box = new ContainerNode([
    'onClick' => CallbackTest::staticMethod(...)
]);
echo "First-class static method set\n";

echo "\nDone!\n";
?>
--EXPECTF--
=== Valid callbacks ===
Callbacks defined

=== Box with onClick ===
%A
Box with onClick created

=== Box with onKeyPress ===
%A
Box with onKeyPress created

=== Text with onInput ===
%A
Text with onInput created

=== Invalid callback types ===
%A
String callback accepted (may fail at call time)
%A
Int callback accepted (may fail at call time)
%A
Invalid array callback accepted (may fail at call time)

=== Multiple event handlers ===
%A
Box with multiple handlers created

=== Nested callbacks ===
%A
Nested callback set

=== Callbacks with use() references ===
%A
Reference callback created, counter = 0

=== Arrow function callbacks ===
%A
Arrow function callback set

=== Class method callbacks ===
%A
Instance method callback set
%A
Static method callback set
%A
Static method string callback set

=== First-class callable syntax ===
%A
First-class callable set
%A
First-class static method set

Done!
