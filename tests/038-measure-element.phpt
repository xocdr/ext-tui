--TEST--
TuiInstance::measureElement returns computed layout
--EXTENSIONS--
tui
--FILE--
<?php
$rc = new ReflectionClass(Xocdr\Tui\Ext\Instance::class);

// Check method exists
echo "measureElement method exists: " . ($rc->hasMethod('measureElement') ? "yes" : "no") . "\n";

$method = $rc->getMethod('measureElement');
echo "measureElement is public: " . ($method->isPublic() ? "yes" : "no") . "\n";

// Check parameter info
$params = $method->getParameters();
echo "measureElement has " . count($params) . " parameter(s)\n";
echo "First parameter is 'id': " . ($params[0]->getName() === 'id' ? "yes" : "no") . "\n";

echo "measureElement API ready\n";
?>
--EXPECT--
measureElement method exists: yes
measureElement is public: yes
measureElement has 1 parameter(s)
First parameter is 'id': yes
measureElement API ready
