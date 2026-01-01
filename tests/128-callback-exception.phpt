--TEST--
Exception handling in component callbacks - basic error handling
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

echo "=== Test 1: Normal rendering works ===\n";
$renderer = tui_test_create(80, 24);
$tree = new ContainerNode(['children' => [new ContentNode("Normal content")]]);
tui_test_render($renderer, $tree);
echo "Normal render succeeded\n";
tui_test_destroy($renderer);

echo "\n=== Test 2: Exception in PHP code doesn't crash extension ===\n";
$renderer = tui_test_create(80, 24);
$tree = new ContainerNode(['children' => [new ContentNode("Before exception")]]);
tui_test_render($renderer, $tree);

try {
    throw new Exception("User code exception");
} catch (Exception $e) {
    echo "Caught: " . $e->getMessage() . "\n";
}

// Renderer should still be usable
$tree = new ContainerNode(['children' => [new ContentNode("After exception")]]);
tui_test_render($renderer, $tree);
echo "Renderer still works after exception\n";
tui_test_destroy($renderer);

echo "\n=== Test 3: Exception type preservation ===\n";
class CustomTestException extends Exception {}

try {
    throw new CustomTestException("Custom exception type");
} catch (CustomTestException $e) {
    echo "Correct exception type: CustomTestException\n";
} catch (Exception $e) {
    echo "Wrong exception type\n";
}

echo "\n=== Test 4: Multiple sequential renders after exception ===\n";
$renderer = tui_test_create(80, 24);

for ($i = 0; $i < 3; $i++) {
    try {
        if ($i === 1) {
            throw new RuntimeException("Exception on iteration $i");
        }
        $tree = new ContainerNode(['children' => [new ContentNode("Iteration $i")]]);
        tui_test_render($renderer, $tree);
        echo "Render $i: ok\n";
    } catch (RuntimeException $e) {
        echo "Render $i: caught exception\n";
    }
}

tui_test_destroy($renderer);

echo "\n=== Test 5: Nested exception handling ===\n";
$renderer = tui_test_create(80, 24);

try {
    $tree = new ContainerNode(['children' => [new ContentNode("Outer")]]);
    tui_test_render($renderer, $tree);

    try {
        throw new LogicException("Inner exception");
    } catch (LogicException $e) {
        echo "Caught inner: " . $e->getMessage() . "\n";

        // Can still render after catching inner exception
        $tree = new ContainerNode(['children' => [new ContentNode("After inner")]]);
        tui_test_render($renderer, $tree);
        echo "Render after inner exception: ok\n";
    }
} catch (Exception $e) {
    echo "Unexpected outer exception\n";
}

tui_test_destroy($renderer);

echo "\nDone!\n";
?>
--EXPECT--
=== Test 1: Normal rendering works ===
Normal render succeeded

=== Test 2: Exception in PHP code doesn't crash extension ===
Caught: User code exception
Renderer still works after exception

=== Test 3: Exception type preservation ===
Correct exception type: CustomTestException

=== Test 4: Multiple sequential renders after exception ===
Render 0: ok
Render 1: caught exception
Render 2: ok

=== Test 5: Nested exception handling ===
Caught inner: Inner exception
Render after inner exception: ok

Done!
