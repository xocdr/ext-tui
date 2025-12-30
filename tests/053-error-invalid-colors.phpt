--TEST--
Error handling for invalid color values
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

echo "=== Valid named colors ===\n";

// Test valid named colors
$validColors = ['red', 'green', 'blue', 'yellow', 'cyan', 'magenta', 'white', 'black'];
foreach ($validColors as $color) {
    $text = new Text('test', ['color' => $color]);
    echo "$color: OK\n";
}

echo "\n=== Valid hex colors ===\n";

// Test valid hex colors
$hexColors = ['#fff', '#FFF', '#ffffff', '#FFFFFF', '#000', '#123456', '#abcdef', '#ABCDEF'];
foreach ($hexColors as $color) {
    $text = new Text('test', ['color' => $color]);
    echo "$color: OK\n";
}

echo "\n=== Valid RGB colors ===\n";

// Test valid RGB function colors
$rgbColors = ['rgb(0,0,0)', 'rgb(255,255,255)', 'rgb(128, 128, 128)'];
foreach ($rgbColors as $color) {
    $text = new Text('test', ['color' => $color]);
    echo "$color: OK\n";
}

echo "\n=== Invalid/unknown colors (should not crash) ===\n";

// Test invalid colors - should not crash, just be ignored or use default
$invalidColors = [
    'notacolor',
    '#xyz',
    '#12345',        // Wrong length
    'rgb(256,0,0)',  // Out of range
    'rgb(-1,0,0)',   // Negative
    '',              // Empty
    '   ',           // Whitespace
    '#',             // Just hash
    'rgb()',         // Empty rgb
];

foreach ($invalidColors as $color) {
    try {
        $text = new Text('test', ['color' => $color]);
        echo "'$color': accepted (may use default)\n";
    } catch (Throwable $e) {
        echo "'$color': exception - " . $e->getMessage() . "\n";
    }
}

echo "\n=== Border colors ===\n";

// Test border colors
$box = new Box(['borderStyle' => 'single', 'borderColor' => 'green']);
echo "borderColor green: OK\n";

$box = new Box([
    'borderStyle' => 'single',
    'borderTopColor' => 'red',
    'borderRightColor' => 'green',
    'borderBottomColor' => 'blue',
    'borderLeftColor' => 'yellow',
]);
echo "Individual border colors: OK\n";

echo "\nAll color tests completed.\n";
?>
--EXPECT--
=== Valid named colors ===
red: OK
green: OK
blue: OK
yellow: OK
cyan: OK
magenta: OK
white: OK
black: OK

=== Valid hex colors ===
#fff: OK
#FFF: OK
#ffffff: OK
#FFFFFF: OK
#000: OK
#123456: OK
#abcdef: OK
#ABCDEF: OK

=== Valid RGB colors ===
rgb(0,0,0): OK
rgb(255,255,255): OK
rgb(128, 128, 128): OK

=== Invalid/unknown colors (should not crash) ===
'notacolor': accepted (may use default)
'#xyz': accepted (may use default)
'#12345': accepted (may use default)
'rgb(256,0,0)': accepted (may use default)
'rgb(-1,0,0)': accepted (may use default)
'': accepted (may use default)
'   ': accepted (may use default)
'#': accepted (may use default)
'rgb()': accepted (may use default)

=== Border colors ===
borderColor green: OK
Individual border colors: OK

All color tests completed.
