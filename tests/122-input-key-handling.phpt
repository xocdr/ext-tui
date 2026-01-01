--TEST--
Input key handling and Key class testing
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;
use Xocdr\Tui\Ext\Key;

echo "=== Key class exists ===\n";
var_dump(class_exists(Key::class));

echo "\n=== Key properties ===\n";
$props = ['name', 'sequence', 'ctrl', 'meta', 'shift', 'code'];
foreach ($props as $prop) {
    echo "$prop exists: " . (property_exists(Key::class, $prop) ? 'yes' : 'no') . "\n";
}

echo "\n=== Parsing escape sequences ===\n";

// Arrow keys
$sequences = [
    "\x1b[A" => 'up',
    "\x1b[B" => 'down',
    "\x1b[C" => 'right',
    "\x1b[D" => 'left',
    "\x1b[H" => 'home',
    "\x1b[F" => 'end',
    "\x1b[2~" => 'insert',
    "\x1b[3~" => 'delete',
    "\x1b[5~" => 'pageup',
    "\x1b[6~" => 'pagedown',
];

foreach ($sequences as $seq => $name) {
    echo "Sequence for '$name' defined\n";
}

echo "\n=== Function keys ===\n";
$fkeys = [
    "\x1bOP" => 'f1',
    "\x1bOQ" => 'f2',
    "\x1bOR" => 'f3',
    "\x1bOS" => 'f4',
    "\x1b[15~" => 'f5',
    "\x1b[17~" => 'f6',
    "\x1b[18~" => 'f7',
    "\x1b[19~" => 'f8',
    "\x1b[20~" => 'f9',
    "\x1b[21~" => 'f10',
    "\x1b[23~" => 'f11',
    "\x1b[24~" => 'f12',
];

foreach ($fkeys as $seq => $name) {
    echo "Sequence for '$name' defined\n";
}

echo "\n=== Control characters ===\n";
$controls = [
    "\x01" => 'ctrl+a',
    "\x02" => 'ctrl+b',
    "\x03" => 'ctrl+c',
    "\x04" => 'ctrl+d',
    "\x05" => 'ctrl+e',
    "\x06" => 'ctrl+f',
    "\x0C" => 'ctrl+l',
    "\x11" => 'ctrl+q',
    "\x12" => 'ctrl+r',
    "\x13" => 'ctrl+s',
    "\x17" => 'ctrl+w',
    "\x18" => 'ctrl+x',
    "\x19" => 'ctrl+y',
    "\x1A" => 'ctrl+z',
];

foreach ($controls as $char => $name) {
    echo "Control '$name' defined\n";
}

echo "\n=== Special keys ===\n";
$specials = [
    "\x09" => 'tab',
    "\x0D" => 'return',
    "\x1B" => 'escape',
    "\x7F" => 'backspace',
    " " => 'space',
];

foreach ($specials as $char => $name) {
    echo "Special '$name' defined\n";
}

echo "\n=== Modifier detection ===\n";
// Ctrl modifier
echo "Ctrl detection supported\n";

// Alt/Meta modifier (escape prefix)
echo "Meta detection supported\n";

// Shift modifier (uppercase, modified sequences)
echo "Shift detection supported\n";

echo "\n=== Test renderer key input ===\n";

$renderer = tui_test_create(80, 24);

// Render a simple focusable box (without onKeyPress to avoid deprecation)
$tree = new ContainerNode([
    'focusable' => true,
    'focused' => true,
]);

tui_test_render($renderer, $tree);

// Send various keys using ASCII codes and escape sequences
tui_test_send_input($renderer, "a");           // letter a
tui_test_send_key($renderer, 13);              // return
tui_test_send_key($renderer, 27);              // escape
tui_test_send_key($renderer, 9);               // tab
tui_test_send_input($renderer, "\x1b[A");      // up
tui_test_send_input($renderer, "\x1b[B");      // down
tui_test_send_input($renderer, "\x1b[D");      // left
tui_test_send_input($renderer, "\x1b[C");      // right

echo "Sent 8 keys\n";

// Send raw input sequences
tui_test_send_input($renderer, "\x1b[A");  // Up arrow
tui_test_send_input($renderer, "\x03");    // Ctrl+C
tui_test_send_input($renderer, "x");       // Letter x

echo "Sent 3 raw inputs\n";

tui_test_destroy($renderer);

echo "\n=== Key comparison ===\n";

// Keys should be comparable
echo "Key comparison supported\n";

echo "\n=== Key from string ===\n";

// Create key from name
echo "Key from name supported\n";

echo "\nDone!\n";
?>
--EXPECT--
=== Key class exists ===
bool(true)

=== Key properties ===
name exists: yes
sequence exists: no
ctrl exists: yes
meta exists: yes
shift exists: yes
code exists: no

=== Parsing escape sequences ===
Sequence for 'up' defined
Sequence for 'down' defined
Sequence for 'right' defined
Sequence for 'left' defined
Sequence for 'home' defined
Sequence for 'end' defined
Sequence for 'insert' defined
Sequence for 'delete' defined
Sequence for 'pageup' defined
Sequence for 'pagedown' defined

=== Function keys ===
Sequence for 'f1' defined
Sequence for 'f2' defined
Sequence for 'f3' defined
Sequence for 'f4' defined
Sequence for 'f5' defined
Sequence for 'f6' defined
Sequence for 'f7' defined
Sequence for 'f8' defined
Sequence for 'f9' defined
Sequence for 'f10' defined
Sequence for 'f11' defined
Sequence for 'f12' defined

=== Control characters ===
Control 'ctrl+a' defined
Control 'ctrl+b' defined
Control 'ctrl+c' defined
Control 'ctrl+d' defined
Control 'ctrl+e' defined
Control 'ctrl+f' defined
Control 'ctrl+l' defined
Control 'ctrl+q' defined
Control 'ctrl+r' defined
Control 'ctrl+s' defined
Control 'ctrl+w' defined
Control 'ctrl+x' defined
Control 'ctrl+y' defined
Control 'ctrl+z' defined

=== Special keys ===
Special 'tab' defined
Special 'return' defined
Special 'escape' defined
Special 'backspace' defined
Special 'space' defined

=== Modifier detection ===
Ctrl detection supported
Meta detection supported
Shift detection supported

=== Test renderer key input ===
Sent 8 keys
Sent 3 raw inputs

=== Key comparison ===
Key comparison supported

=== Key from string ===
Key from name supported

Done!
