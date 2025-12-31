--TEST--
Test renderer comprehensive functionality
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

echo "=== Create test renderer ===\n";
$renderer = tui_test_create(80, 24);
var_dump(is_resource($renderer) || is_object($renderer));

echo "\n=== Render simple text ===\n";
$tree = new Text("Hello World");
tui_test_render($renderer, $tree);
echo "Rendered simple text\n";

echo "\n=== Render box with text ===\n";
$tree = new Box(['children' => [new Text("Inside box")]]);
tui_test_render($renderer, $tree);
echo "Rendered box with text\n";

echo "\n=== Render nested structure ===\n";
$tree = new Box([
    'flexDirection' => 'column',
    'children' => [
        new Box(['id' => 'header', 'children' => [new Text("Header")]]),
        new Box(['id' => 'content', 'flexGrow' => 1, 'children' => [new Text("Content")]]),
        new Box(['id' => 'footer', 'children' => [new Text("Footer")]]),
    ]
]);
tui_test_render($renderer, $tree);
echo "Rendered nested structure\n";

echo "\n=== Query by ID ===\n";
$header = tui_test_get_by_id($renderer, 'header');
echo "Found header: " . ($header ? 'yes' : 'no') . "\n";

$content = tui_test_get_by_id($renderer, 'content');
echo "Found content: " . ($content ? 'yes' : 'no') . "\n";

$nonexistent = tui_test_get_by_id($renderer, 'nonexistent');
echo "Found nonexistent: " . ($nonexistent ? 'yes' : 'no') . "\n";

echo "\n=== Query by text ===\n";
$headerText = tui_test_get_by_text($renderer, 'Header');
echo "Found 'Header': " . ($headerText ? 'yes' : 'no') . "\n";

$notFound = tui_test_get_by_text($renderer, 'NotInTree');
echo "Found 'NotInTree': " . ($notFound ? 'yes' : 'no') . "\n";

echo "\n=== Get output ===\n";
$output = tui_test_get_output($renderer);
echo "Output type: " . gettype($output) . "\n";
echo "Output not empty: " . (!empty($output) ? 'yes' : 'no') . "\n";

echo "\n=== Send input ===\n";
tui_test_send_input($renderer, "a");
echo "Sent 'a'\n";

tui_test_send_key($renderer, 13);  // enter
echo "Sent enter\n";

tui_test_send_key($renderer, 27);  // escape
echo "Sent escape\n";

// Arrow keys via escape sequences
tui_test_send_input($renderer, "\x1b[A");  // up
echo "Sent up arrow\n";

tui_test_send_input($renderer, "\x1b[B");  // down
echo "Sent down arrow\n";

tui_test_send_key($renderer, 9);  // tab
echo "Sent tab\n";

echo "\n=== Advance frame ===\n";
tui_test_advance_frame($renderer);
echo "Advanced 1 frame\n";

// Note: advance_frame only takes 1 argument
for ($i = 0; $i < 10; $i++) {
    tui_test_advance_frame($renderer);
}
echo "Advanced 10 frames\n";

echo "\n=== Re-render ===\n";
$newTree = new Box(['children' => [new Text("Updated content")]]);
tui_test_render($renderer, $newTree);
echo "Re-rendered with new tree\n";

echo "\n=== Different sizes ===\n";
$small = tui_test_create(20, 10);
tui_test_render($small, new Text("Small"));
echo "Created 20x10 renderer\n";
tui_test_destroy($small);

$large = tui_test_create(200, 50);
tui_test_render($large, new Text("Large"));
echo "Created 200x50 renderer\n";
tui_test_destroy($large);

echo "\n=== Destroy renderer ===\n";
tui_test_destroy($renderer);
echo "Destroyed renderer\n";

echo "\n=== Edge case: zero size ===\n";
try {
    $zero = tui_test_create(0, 0);
    if ($zero) {
        tui_test_destroy($zero);
    }
    echo "Handled zero size\n";
} catch (Exception $e) {
    echo "Zero size throws exception (expected)\n";
}

echo "\n=== Edge case: empty render ===\n";
$r = tui_test_create(80, 24);
tui_test_render($r, new Box());
echo "Rendered empty box\n";
tui_test_destroy($r);

echo "\nDone!\n";
?>
--EXPECTF--
=== Create test renderer ===
bool(true)

=== Render simple text ===
Rendered simple text

=== Render box with text ===
Rendered box with text

=== Render nested structure ===
Rendered nested structure

=== Query by ID ===
Found header: yes
Found content: yes
Found nonexistent: no

=== Query by text ===
Found 'Header': yes
Found 'NotInTree': no

=== Get output ===
Output type: array
Output not empty: yes

=== Send input ===
Sent 'a'
Sent enter
Sent escape
Sent up arrow
Sent down arrow
Sent tab

=== Advance frame ===
Advanced 1 frame
Advanced 10 frames

=== Re-render ===
Re-rendered with new tree

=== Different sizes ===
Created 20x10 renderer
Created 200x50 renderer

=== Destroy renderer ===
Destroyed renderer

=== Edge case: zero size ===
Zero size throws exception (expected)

=== Edge case: empty render ===
Rendered empty box

Done!
