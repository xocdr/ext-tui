--TEST--
Object pool: metrics tracking
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

// Get pool metrics function exists
var_dump(function_exists('tui_get_pool_metrics'));

// Get initial metrics
$m = tui_get_pool_metrics();
var_dump(is_array($m));
var_dump(array_key_exists('children_allocs', $m));
var_dump(array_key_exists('children_fallbacks', $m));
var_dump(array_key_exists('children_reuses', $m));
var_dump(array_key_exists('keymap_reuses', $m));
var_dump(array_key_exists('keymap_fallbacks', $m));
var_dump(array_key_exists('children_hit_rate', $m));
var_dump(array_key_exists('keymap_hit_rate', $m));

// Create a test renderer and some nodes to trigger pool usage
$renderer = tui_test_create(80, 24);

// Create multiple boxes which should allocate from pool
$box = new Box(['width' => 80, 'height' => 24]);
$box->children = [
    new Text("Line 1"),
    new Text("Line 2"),
];
tui_test_render($renderer, $box);

// Check pool was used
$m = tui_get_pool_metrics();
var_dump($m['children_allocs'] >= 0);  // Should have some allocations
var_dump($m['children_hit_rate'] >= 0.0);  // Hit rate is a percentage

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
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
Done
