--TEST--
Typed exceptions: InstanceDestroyedException
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\TuiException;
use Xocdr\Tui\InstanceDestroyedException;

// Test 1: Exception classes exist
echo "Test 1: Exception classes exist\n";
var_dump(class_exists(TuiException::class));
var_dump(class_exists(InstanceDestroyedException::class));

// Test 2: TuiException extends Exception
echo "\nTest 2: TuiException extends Exception\n";
$ref = new ReflectionClass(TuiException::class);
var_dump($ref->getParentClass()->getName());

// Test 3: InstanceDestroyedException extends TuiException
echo "\nTest 3: InstanceDestroyedException extends TuiException\n";
$ref = new ReflectionClass(InstanceDestroyedException::class);
var_dump($ref->getParentClass()->getName());

// Test 4: Can instantiate and throw TuiException
echo "\nTest 4: Can instantiate and throw TuiException\n";
try {
    throw new TuiException("Test TuiException");
} catch (TuiException $e) {
    echo "Caught TuiException: " . $e->getMessage() . "\n";
} catch (Exception $e) {
    echo "Caught wrong type\n";
}

// Test 5: Can instantiate and throw InstanceDestroyedException
echo "\nTest 5: Can instantiate and throw InstanceDestroyedException\n";
try {
    throw new InstanceDestroyedException("Instance was destroyed");
} catch (InstanceDestroyedException $e) {
    echo "Caught InstanceDestroyedException: " . $e->getMessage() . "\n";
} catch (TuiException $e) {
    echo "Caught TuiException (wrong specificity)\n";
} catch (Exception $e) {
    echo "Caught wrong type\n";
}

// Test 6: InstanceDestroyedException is catchable as TuiException
echo "\nTest 6: InstanceDestroyedException catchable as TuiException\n";
try {
    throw new InstanceDestroyedException("Test message");
} catch (TuiException $e) {
    echo "Caught as TuiException: " . get_class($e) . "\n";
}

// Test 7: InstanceDestroyedException is catchable as Exception
echo "\nTest 7: InstanceDestroyedException catchable as Exception\n";
try {
    throw new InstanceDestroyedException("Test message");
} catch (Exception $e) {
    echo "Caught as Exception: " . get_class($e) . "\n";
}

// Test 8: TuiException is catchable as Exception
echo "\nTest 8: TuiException catchable as Exception\n";
try {
    throw new TuiException("Test message");
} catch (Exception $e) {
    echo "Caught as Exception: " . get_class($e) . "\n";
}

echo "\nAll tests passed!\n";
?>
--EXPECT--
Test 1: Exception classes exist
bool(true)
bool(true)

Test 2: TuiException extends Exception
string(9) "Exception"

Test 3: InstanceDestroyedException extends TuiException
string(22) "Xocdr\Tui\TuiException"

Test 4: Can instantiate and throw TuiException
Caught TuiException: Test TuiException

Test 5: Can instantiate and throw InstanceDestroyedException
Caught InstanceDestroyedException: Instance was destroyed

Test 6: InstanceDestroyedException catchable as TuiException
Caught as TuiException: Xocdr\Tui\InstanceDestroyedException

Test 7: InstanceDestroyedException catchable as Exception
Caught as Exception: Xocdr\Tui\InstanceDestroyedException

Test 8: TuiException catchable as Exception
Caught as Exception: Xocdr\Tui\TuiException

All tests passed!
