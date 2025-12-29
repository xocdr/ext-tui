--TEST--
Testing framework: query functions (get_by_id, get_by_text)
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

$renderer = tui_test_create(80, 24);

// Create a simple UI with IDs
$box = new Box(['width' => 80, 'height' => 24]);
$box->id = "root";
$box->children = [
    (function() {
        $header = new Box(['height' => 3]);
        $header->id = "header";
        $header->children = [new Text("Welcome to TUI Test")];
        return $header;
    })(),
    (function() {
        $content = new Box(['flexGrow' => 1]);
        $content->id = "content";
        $content->children = [new Text("Main content here")];
        return $content;
    })(),
];

tui_test_render($renderer, $box);

// Test tui_test_get_by_id - found
$root = tui_test_get_by_id($renderer, "root");
var_dump($root !== null);
var_dump($root['id'] ?? null);
var_dump($root['type'] ?? null);

// Test tui_test_get_by_id - not found
$missing = tui_test_get_by_id($renderer, "nonexistent");
var_dump($missing === null);

// Test tui_test_get_by_text - found
$matches = tui_test_get_by_text($renderer, "Welcome");
var_dump(is_array($matches));
var_dump(count($matches));
if (count($matches) > 0) {
    var_dump(strpos($matches[0]['text'], "Welcome") !== false);
}

// Test tui_test_get_by_text - not found
$noMatches = tui_test_get_by_text($renderer, "XYZZY");
var_dump(count($noMatches));

tui_test_destroy($renderer);
echo "Done\n";
?>
--EXPECT--
bool(true)
string(4) "root"
string(3) "box"
bool(true)
bool(true)
int(1)
bool(true)
int(0)
Done
