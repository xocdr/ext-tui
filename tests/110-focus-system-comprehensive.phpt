--TEST--
Focus system comprehensive testing
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;
use Xocdr\Tui\Ext\Focus;
use Xocdr\Tui\Ext\FocusManager;
use Xocdr\Tui\Ext\FocusEvent;

echo "=== Focus classes exist ===\n";
var_dump(class_exists(Focus::class));
var_dump(class_exists(FocusManager::class));
var_dump(class_exists(FocusEvent::class));

echo "\n=== FocusEvent properties ===\n";
$event = new FocusEvent();
$props = ['previous', 'current', 'direction'];
foreach ($props as $prop) {
    echo "$prop exists: " . (property_exists($event, $prop) ? 'yes' : 'no') . "\n";
}

echo "\n=== Box focusable property ===\n";
$box = new Box(['focusable' => true]);
var_dump($box->focusable);

$box = new Box(['focusable' => false]);
var_dump($box->focusable);

echo "\n=== Box focused property ===\n";
$box = new Box(['focused' => true]);
var_dump($box->focused);

$box = new Box(['focused' => false]);
var_dump($box->focused);

echo "\n=== Multiple focusable elements ===\n";
$renderer = tui_test_create(80, 24);
$tree = new Box([
    'children' => [
        new Box(['id' => 'btn1', 'focusable' => true, 'children' => [new Text("Button 1")]]),
        new Box(['id' => 'btn2', 'focusable' => true, 'children' => [new Text("Button 2")]]),
        new Box(['id' => 'btn3', 'focusable' => true, 'children' => [new Text("Button 3")]]),
    ]
]);
tui_test_render($renderer, $tree);

$btn1 = tui_test_get_by_id($renderer, 'btn1');
$btn2 = tui_test_get_by_id($renderer, 'btn2');
$btn3 = tui_test_get_by_id($renderer, 'btn3');

echo "btn1 found: " . ($btn1 ? 'yes' : 'no') . "\n";
echo "btn2 found: " . ($btn2 ? 'yes' : 'no') . "\n";
echo "btn3 found: " . ($btn3 ? 'yes' : 'no') . "\n";

echo "\n=== Tab key navigation ===\n";
tui_test_send_key($renderer, 9);  // tab is ASCII 9
echo "Sent tab\n";

tui_test_send_key($renderer, 9);  // tab again
echo "Sent tab again\n";

echo "\n=== Shift+Tab navigation ===\n";
// Note: shift+tab simulation via escape sequence
tui_test_send_input($renderer, "\x1b[Z");  // Shift+Tab escape sequence
echo "Sent shift+tab\n";

tui_test_destroy($renderer);

echo "\n=== Non-focusable element ===\n";
$box = new Box(['focusable' => false]);
var_dump($box->focusable);

echo "\n=== Initial focused state ===\n";
$box = new Box(['focusable' => true, 'focused' => true]);
var_dump($box->focused);

echo "\n=== showCursor property ===\n";
$box = new Box(['focusable' => true, 'showCursor' => true]);
var_dump($box->showCursor);

$box = new Box(['focusable' => true, 'showCursor' => false]);
var_dump($box->showCursor);

echo "\nDone!\n";
?>
--EXPECT--
=== Focus classes exist ===
bool(true)
bool(true)
bool(true)

=== FocusEvent properties ===
previous exists: yes
current exists: yes
direction exists: yes

=== Box focusable property ===
bool(true)
bool(false)

=== Box focused property ===
bool(true)
bool(false)

=== Multiple focusable elements ===
btn1 found: yes
btn2 found: yes
btn3 found: yes

=== Tab key navigation ===
Sent tab
Sent tab again

=== Shift+Tab navigation ===
Sent shift+tab

=== Non-focusable element ===
bool(false)

=== Initial focused state ===
bool(true)

=== showCursor property ===
bool(true)
bool(false)

Done!
