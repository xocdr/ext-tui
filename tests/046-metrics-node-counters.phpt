--TEST--
Telemetry: node creation/destruction metrics
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

// Enable metrics
tui_metrics_enable();
tui_metrics_reset();

// Create a test renderer and some nodes
$renderer = tui_test_create(80, 24);

// Create a box with text children
$box = new Box(['width' => 80, 'height' => 24]);
$box->children = [
    new Text("Hello"),
    new Text("World"),
];
tui_test_render($renderer, $box);

// Check node metrics
$m = tui_get_node_metrics();
var_dump($m['node_count'] > 0);
var_dump($m['box_count'] >= 1);
var_dump($m['text_count'] >= 2);

// Get specific metrics functions
$nm = tui_get_node_metrics();
var_dump(isset($nm['node_count']));
var_dump(isset($nm['box_count']));
var_dump(isset($nm['text_count']));

tui_test_destroy($renderer);
echo "Done\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
Done
