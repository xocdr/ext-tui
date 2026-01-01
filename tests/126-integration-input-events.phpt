--TEST--
Integration test for input handling and keyboard events
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;
use Xocdr\Tui\Ext\Key;

echo "=== Input events integration test ===\n";

$renderer = tui_test_create(80, 24);

// Test 1: Basic key input
echo "\n--- Test 1: Basic key input ---\n";

$root = new ContainerNode([
    'id' => 'input-test',
    'width' => 80,
    'height' => 24,
    'focusable' => true,
    'focused' => true,
]);
$root->addChild(new ContentNode('Press keys'));

tui_test_render($renderer, $root);

// Send various key codes
$keyCodes = [
    97 => 'a',
    98 => 'b',
    99 => 'c',
    13 => 'return',
    27 => 'escape',
    9 => 'tab',
    127 => 'backspace',
    32 => 'space',
];

foreach ($keyCodes as $code => $name) {
    tui_test_send_key($renderer, $code);
    tui_test_advance_frame($renderer);
    echo "Sent key: $name (code $code)\n";
}

// Test 2: Arrow keys (escape sequences)
echo "\n--- Test 2: Arrow keys ---\n";

$arrowSequences = [
    "\x1b[A" => 'up',
    "\x1b[B" => 'down',
    "\x1b[C" => 'right',
    "\x1b[D" => 'left',
];

foreach ($arrowSequences as $seq => $name) {
    tui_test_send_input($renderer, $seq);
    tui_test_advance_frame($renderer);
    echo "Sent arrow: $name\n";
}

// Test 3: Function keys
echo "\n--- Test 3: Function keys ---\n";

$functionKeys = [
    "\x1bOP" => 'F1',
    "\x1bOQ" => 'F2',
    "\x1bOR" => 'F3',
    "\x1bOS" => 'F4',
    "\x1b[15~" => 'F5',
    "\x1b[17~" => 'F6',
];

foreach ($functionKeys as $seq => $name) {
    tui_test_send_input($renderer, $seq);
    tui_test_advance_frame($renderer);
    echo "Sent function key: $name\n";
}

// Test 4: Control characters
echo "\n--- Test 4: Control characters ---\n";

$controlChars = [
    "\x01" => 'Ctrl+A',
    "\x03" => 'Ctrl+C',
    "\x04" => 'Ctrl+D',
    "\x18" => 'Ctrl+X',
    "\x19" => 'Ctrl+Y',
    "\x1A" => 'Ctrl+Z',
];

foreach ($controlChars as $char => $name) {
    tui_test_send_input($renderer, $char);
    tui_test_advance_frame($renderer);
    echo "Sent control: $name\n";
}

// Test 5: Navigation keys
echo "\n--- Test 5: Navigation keys ---\n";

$navKeys = [
    "\x1b[H" => 'Home',
    "\x1b[F" => 'End',
    "\x1b[2~" => 'Insert',
    "\x1b[3~" => 'Delete',
    "\x1b[5~" => 'PageUp',
    "\x1b[6~" => 'PageDown',
];

foreach ($navKeys as $seq => $name) {
    tui_test_send_input($renderer, $seq);
    tui_test_advance_frame($renderer);
    echo "Sent nav key: $name\n";
}

// Test 6: ContentNode input sequence
echo "\n--- Test 6: ContentNode input sequence ---\n";

$textInput = "Hello, World!";
for ($i = 0; $i < strlen($textInput); $i++) {
    $char = $textInput[$i];
    tui_test_send_input($renderer, $char);
}
tui_test_advance_frame($renderer);
echo "Sent text: \"$textInput\"\n";

// Test 7: Focus navigation
echo "\n--- Test 7: Focus navigation ---\n";

$focusRoot = new ContainerNode([
    'id' => 'focus-root',
    'width' => 80,
    'height' => 24,
    'flexDirection' => 'column',
]);

for ($i = 0; $i < 3; $i++) {
    $btn = new ContainerNode([
        'id' => "btn-$i",
        'focusable' => true,
        'focused' => ($i === 0),
        'height' => 3,
    ]);
    $btn->addChild(new ContentNode("Button $i"));
    $focusRoot->addChild($btn);
}

tui_test_render($renderer, $focusRoot);

// Navigate with Tab
for ($i = 0; $i < 5; $i++) {
    tui_test_send_key($renderer, 9); // Tab
    tui_test_advance_frame($renderer);
}
echo "Tab navigation (5 times) completed\n";

// Navigate with arrow keys
for ($i = 0; $i < 3; $i++) {
    tui_test_send_input($renderer, "\x1b[B"); // Down arrow
    tui_test_advance_frame($renderer);
}
echo "Arrow navigation (3 down) completed\n";

// Test 8: Modifier combinations
echo "\n--- Test 8: Modifier combinations ---\n";

// Shift+Tab (usually \x1b[Z)
tui_test_send_input($renderer, "\x1b[Z");
tui_test_advance_frame($renderer);
echo "Shift+Tab sent\n";

// Alt+letter (ESC followed by letter)
tui_test_send_input($renderer, "\x1ba");
tui_test_advance_frame($renderer);
echo "Alt+a sent\n";

// Test 9: Rapid key sequence
echo "\n--- Test 9: Rapid key sequence ---\n";

$rapidSequence = "The quick brown fox";
for ($i = 0; $i < strlen($rapidSequence); $i++) {
    tui_test_send_input($renderer, $rapidSequence[$i]);
}
tui_test_advance_frame($renderer);
echo "Rapid sequence of " . strlen($rapidSequence) . " characters sent\n";

// Test 10: Unicode input
echo "\n--- Test 10: Unicode input ---\n";

$unicodeChars = ['é', 'ñ', '中', '日', '🎉'];
foreach ($unicodeChars as $char) {
    tui_test_send_input($renderer, $char);
    tui_test_advance_frame($renderer);
}
echo "Unicode characters sent: " . implode(' ', $unicodeChars) . "\n";

tui_test_destroy($renderer);

echo "\n=== Input events integration tests passed ===\n";
?>
--EXPECT--
=== Input events integration test ===

--- Test 1: Basic key input ---
Sent key: a (code 97)
Sent key: b (code 98)
Sent key: c (code 99)
Sent key: return (code 13)
Sent key: escape (code 27)
Sent key: tab (code 9)
Sent key: backspace (code 127)
Sent key: space (code 32)

--- Test 2: Arrow keys ---
Sent arrow: up
Sent arrow: down
Sent arrow: right
Sent arrow: left

--- Test 3: Function keys ---
Sent function key: F1
Sent function key: F2
Sent function key: F3
Sent function key: F4
Sent function key: F5
Sent function key: F6

--- Test 4: Control characters ---
Sent control: Ctrl+A
Sent control: Ctrl+C
Sent control: Ctrl+D
Sent control: Ctrl+X
Sent control: Ctrl+Y
Sent control: Ctrl+Z

--- Test 5: Navigation keys ---
Sent nav key: Home
Sent nav key: End
Sent nav key: Insert
Sent nav key: Delete
Sent nav key: PageUp
Sent nav key: PageDown

--- Test 6: ContentNode input sequence ---
Sent text: "Hello, World!"

--- Test 7: Focus navigation ---
Tab navigation (5 times) completed
Arrow navigation (3 down) completed

--- Test 8: Modifier combinations ---
Shift+Tab sent
Alt+a sent

--- Test 9: Rapid key sequence ---
Rapid sequence of 19 characters sent

--- Test 10: Unicode input ---
Unicode characters sent: é ñ 中 日 🎉

=== Input events integration tests passed ===
