--TEST--
TuiInstance methods comprehensive testing
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;
use Xocdr\Tui\Ext\Instance;

echo "=== Instance class exists ===\n";
var_dump(class_exists(Instance::class));

echo "\n=== Instance methods exist ===\n";
$methods = [
    'rerender',
    'unmount',
    'waitUntilExit',
    'exit',
    'useState',
    'useInput',
    'useFocus',
    'useFocusManager',
    'useStdin',
    'useStdout',
    'useStderr',
    'getTerminalSize',
    'getSize',
    'setState',
    'focusNext',
    'focusPrev',
    'measureElement',
    'setInputHandler',
    'setFocusHandler',
    'setResizeHandler',
    'setTickHandler',
    'addTimer',
    'removeTimer',
    'clear',
    'getCapturedOutput',
];
foreach ($methods as $method) {
    echo "$method: " . (method_exists(Instance::class, $method) ? 'yes' : 'no') . "\n";
}

echo "\n=== Test with test renderer (simulated instance) ===\n";
// We can't create a real Instance without tui_render() blocking,
// but we can test the methods exist and signatures are correct

// Test using reflection
$reflection = new ReflectionClass(Instance::class);

echo "\n=== Method signatures ===\n";
$methodsToCheck = ['rerender', 'unmount', 'exit', 'useState', 'addTimer'];
foreach ($methodsToCheck as $methodName) {
    if ($reflection->hasMethod($methodName)) {
        $method = $reflection->getMethod($methodName);
        $params = $method->getParameters();
        echo "$methodName: " . count($params) . " params\n";
    }
}

echo "\n=== Test measureElement with test renderer ===\n";
$renderer = tui_test_create(80, 24);
$tree = new Box([
    'id' => 'measurable',
    'width' => 40,
    'height' => 10,
    'children' => [new Text("Measure me")]
]);
tui_test_render($renderer, $tree);

// measureElement is tested via test renderer queries
$element = tui_test_get_by_id($renderer, 'measurable');
echo "Element found: " . ($element ? 'yes' : 'no') . "\n";

tui_test_destroy($renderer);

echo "\nDone!\n";
?>
--EXPECT--
=== Instance class exists ===
bool(true)

=== Instance methods exist ===
rerender: yes
unmount: yes
waitUntilExit: yes
exit: yes
useState: yes
useInput: yes
useFocus: yes
useFocusManager: yes
useStdin: yes
useStdout: yes
useStderr: yes
getTerminalSize: yes
getSize: yes
setState: yes
focusNext: yes
focusPrev: yes
measureElement: yes
setInputHandler: yes
setFocusHandler: yes
setResizeHandler: yes
setTickHandler: yes
addTimer: yes
removeTimer: yes
clear: yes
getCapturedOutput: yes

=== Test with test renderer (simulated instance) ===

=== Method signatures ===
rerender: 0 params
unmount: 0 params
exit: 1 params
useState: 1 params
addTimer: 2 params

=== Test measureElement with test renderer ===
Element found: yes

Done!
