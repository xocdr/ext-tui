--TEST--
Box constructor edge cases and property validation
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

echo "=== Empty constructor ===\n";
$box = new Box();
var_dump(get_class($box));

echo "\n=== Empty array ===\n";
$box = new Box([]);
var_dump(get_class($box));

echo "\n=== All numeric properties ===\n";
$box = new Box([
    'width' => 100,
    'height' => 50,
    'padding' => 5,
    'paddingTop' => 1,
    'paddingBottom' => 2,
    'paddingLeft' => 3,
    'paddingRight' => 4,
    'margin' => 10,
    'marginTop' => 1,
    'marginBottom' => 2,
    'marginLeft' => 3,
    'marginRight' => 4,
    'gap' => 2,
    'flexGrow' => 1,
    'flexShrink' => 0,
]);
echo "width: " . $box->width . "\n";
echo "height: " . $box->height . "\n";
echo "padding: " . $box->padding . "\n";
echo "flexGrow: " . $box->flexGrow . "\n";

echo "\n=== String properties ===\n";
$box = new Box([
    'key' => 'my-key',
    'id' => 'my-id',
    'flexDirection' => 'row',
    'alignItems' => 'center',
    'justifyContent' => 'space-between',
]);
echo "key: " . $box->key . "\n";
echo "id: " . $box->id . "\n";
echo "flexDirection: " . $box->flexDirection . "\n";
echo "alignItems: " . $box->alignItems . "\n";
echo "justifyContent: " . $box->justifyContent . "\n";

echo "\n=== Percentage values ===\n";
$box = new Box([
    'width' => '100%',
    'height' => '50%',
]);
echo "width: " . $box->width . "\n";
echo "height: " . $box->height . "\n";

echo "\n=== Color properties ===\n";
$box = new Box([
    'backgroundColor' => '#ff0000',
    'borderColor' => '#00ff00',
]);
echo "backgroundColor set: " . (isset($box->backgroundColor) ? 'yes' : 'no') . "\n";

echo "\n=== Boolean properties ===\n";
$box = new Box([
    'focusable' => true,
    'focused' => false,
]);
var_dump($box->focusable);
var_dump($box->focused);

echo "\n=== Border styles ===\n";
foreach (['none', 'single', 'double', 'round', 'bold', 'dashed'] as $style) {
    $box = new Box(['borderStyle' => $style]);
    echo "borderStyle '$style': " . $box->borderStyle . "\n";
}

echo "\n=== Children in constructor ===\n";
$box = new Box([
    'children' => [
        new Text("Child 1"),
        new Text("Child 2"),
        new Box(['children' => [new Text("Nested")]]),
    ]
]);
echo "Has children\n";

echo "\n=== Zero values ===\n";
$box = new Box([
    'width' => 0,
    'height' => 0,
    'padding' => 0,
    'margin' => 0,
    'flexGrow' => 0,
]);
echo "width: " . $box->width . "\n";
echo "flexGrow: " . $box->flexGrow . "\n";

echo "\n=== Negative values (should be handled) ===\n";
$box = new Box([
    'width' => -10,
    'height' => -20,
]);
// Should not crash, behavior may vary

echo "\n=== Float values ===\n";
$box = new Box([
    'width' => 50.5,
    'height' => 25.75,
    'flexGrow' => 1.5,
]);
echo "width: " . $box->width . "\n";

echo "\n=== Very large values ===\n";
$box = new Box([
    'width' => 999999,
    'height' => 999999,
]);
echo "width: " . $box->width . "\n";

echo "\n=== Unknown properties (should be ignored) ===\n";
$box = new Box([
    'unknownProperty' => 'value',
    'anotherUnknown' => 123,
]);
echo "Created box with unknown properties\n";

echo "\nDone!\n";
?>
--EXPECTF--
=== Empty constructor ===
string(17) "Xocdr\Tui\Ext\Box"

=== Empty array ===
string(17) "Xocdr\Tui\Ext\Box"

=== All numeric properties ===
width: 100
height: 50
padding: 5
flexGrow: 1

=== String properties ===
key: my-key
id: my-id
flexDirection: row
alignItems: center
justifyContent: space-between

=== Percentage values ===
width: 100%
height: 50%

=== Color properties ===
%A
backgroundColor set: yes

=== Boolean properties ===
bool(true)
bool(false)

=== Border styles ===
borderStyle 'none': none
borderStyle 'single': single
borderStyle 'double': double
borderStyle 'round': round
borderStyle 'bold': bold
borderStyle 'dashed': dashed

=== Children in constructor ===
Has children

=== Zero values ===
width: 0
flexGrow: 0

=== Negative values (should be handled) ===
%A
=== Float values ===
width: 50.5

=== Very large values ===
width: 999999

=== Unknown properties (should be ignored) ===
%A
Created box with unknown properties

Done!
