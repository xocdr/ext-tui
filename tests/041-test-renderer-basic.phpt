--TEST--
Testing framework: basic tui_test_* functions
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

// Test tui_test_create
$renderer = tui_test_create(40, 10);
var_dump(is_resource($renderer));

// Test tui_test_render with a simple box
$box = new Box(['width' => 20, 'height' => 3]);
$box->children = [new Text("Hello World")];

tui_test_render($renderer, $box);

// Test tui_test_get_output
$output = tui_test_get_output($renderer);
var_dump(is_array($output));
var_dump(count($output));  // Should be 10 (height of renderer)

// Test tui_test_to_string
$str = tui_test_to_string($renderer);
var_dump(is_string($str));
var_dump(strpos($str, "Hello World") !== false);

// Test tui_test_destroy
tui_test_destroy($renderer);
echo "Destroy complete\n";

// Test invalid dimensions
try {
    $bad = tui_test_create(0, 10);
    echo "Should have thrown\n";
} catch (Exception $e) {
    echo "Exception caught: invalid dimensions\n";
}

try {
    $bad = tui_test_create(2000, 10);
    echo "Should have thrown\n";
} catch (Exception $e) {
    echo "Exception caught: dimensions too large\n";
}

echo "Done\n";
?>
--EXPECT--
bool(true)
bool(true)
int(10)
bool(true)
bool(true)
Destroy complete
Exception caught: invalid dimensions
Exception caught: dimensions too large
Done
