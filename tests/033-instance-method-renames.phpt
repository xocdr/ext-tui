--TEST--
TuiInstance method renames with deprecated aliases
--EXTENSIONS--
tui
--FILE--
<?php
$rc = new ReflectionClass(Xocdr\Tui\Ext\Instance::class);

// Check new method names exist
$newMethods = ['state', 'onInput', 'focus', 'focusManager', 'stdin', 'stdout', 'stderr'];
foreach ($newMethods as $method) {
    echo "$method: " . ($rc->hasMethod($method) ? "exists" : "missing") . "\n";
}

echo "---\n";

// Check old method names are deprecated
$oldMethods = ['useState', 'useInput', 'useFocus', 'useFocusManager', 'useStdin', 'useStdout', 'useStderr'];
foreach ($oldMethods as $method) {
    $m = $rc->getMethod($method);
    echo "$method: " . ($m->isDeprecated() ? "deprecated" : "not deprecated") . "\n";
}
?>
--EXPECT--
state: exists
onInput: exists
focus: exists
focusManager: exists
stdin: exists
stdout: exists
stderr: exists
---
useState: deprecated
useInput: deprecated
useFocus: deprecated
useFocusManager: deprecated
useStdin: deprecated
useStdout: deprecated
useStderr: deprecated
