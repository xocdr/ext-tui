--TEST--
Telemetry: INI setting for metrics
--EXTENSIONS--
tui
--INI--
tui.metrics_enabled=1
--FILE--
<?php
// With INI setting, metrics should be enabled from the start
var_dump(tui_metrics_enabled());

// Create some nodes to verify metrics are being collected
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

$renderer = tui_test_create(80, 24);
$box = new Box();
$box->children = [new Text("Test")];
tui_test_render($renderer, $box);

$m = tui_get_metrics();
// With metrics enabled via INI, we should see node counts
var_dump($m['node_count'] > 0);

tui_test_destroy($renderer);
echo "Done\n";
?>
--EXPECT--
bool(true)
bool(true)
Done
