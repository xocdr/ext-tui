--TEST--
Color parsing edge cases
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContentNode;
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\Color;

echo "=== Hex colors (6-digit) ===\n";
$text = new ContentNode("Red", ['color' => '#ff0000']);
echo "Created with #ff0000\n";

$text = new ContentNode("Green", ['color' => '#00FF00']);  // uppercase
echo "Created with #00FF00\n";

$text = new ContentNode("Blue", ['color' => '#0000ff']);
echo "Created with #0000ff\n";

echo "\n=== Hex colors (3-digit shorthand) ===\n";
$text = new ContentNode("Red", ['color' => '#f00']);
echo "Created with #f00\n";

$text = new ContentNode("White", ['color' => '#fff']);
echo "Created with #fff\n";

echo "\n=== RGB array format ===\n";
$text = new ContentNode("Red", ['color' => [255, 0, 0]]);
echo "Created with [255, 0, 0]\n";

$text = new ContentNode("Green", ['color' => [0, 255, 0]]);
echo "Created with [0, 255, 0]\n";

$text = new ContentNode("Black", ['color' => [0, 0, 0]]);
echo "Created with [0, 0, 0]\n";

$text = new ContentNode("White", ['color' => [255, 255, 255]]);
echo "Created with [255, 255, 255]\n";

echo "\n=== Background colors ===\n";
$text = new ContentNode("BG Red", ['backgroundColor' => '#ff0000']);
echo "Background #ff0000\n";

$text = new ContentNode("BG RGB", ['backgroundColor' => [128, 128, 128]]);
echo "Background [128, 128, 128]\n";

echo "\n=== Box border colors ===\n";
$box = new ContainerNode(['borderStyle' => 'single', 'borderColor' => '#00ff00']);
echo "Border color #00ff00\n";

echo "\n=== Named colors via Color enum ===\n";
echo "Red value: " . Color::Red->value . "\n";
echo "Blue value: " . Color::Blue->value . "\n";
echo "Green value: " . Color::Green->value . "\n";
echo "White value: " . Color::White->value . "\n";
echo "Black value: " . Color::Black->value . "\n";

echo "\n=== Color::fromName() ===\n";
$red = Color::fromName('red');
echo "fromName('red'): " . ($red ? $red->value : 'null') . "\n";

$coral = Color::fromName('coral');
echo "fromName('coral'): " . ($coral ? $coral->value : 'null') . "\n";

$unknown = Color::fromName('notacolor');
echo "fromName('notacolor'): " . ($unknown ? $unknown->value : 'null') . "\n";

echo "\n=== Color::toRgb() ===\n";
$rgb = Color::Red->toRgb();
echo "Red RGB: r=" . $rgb['r'] . ", g=" . $rgb['g'] . ", b=" . $rgb['b'] . "\n";

$rgb = Color::Lime->toRgb();
echo "Lime RGB: r=" . $rgb['r'] . ", g=" . $rgb['g'] . ", b=" . $rgb['b'] . "\n";

echo "\n=== Color::toAnsi() and toAnsiBg() ===\n";
$ansi = Color::Red->toAnsi();
echo "Red ANSI (length): " . strlen($ansi) . "\n";

$ansiBg = Color::Blue->toAnsiBg();
echo "Blue ANSI BG (length): " . strlen($ansiBg) . "\n";

echo "\n=== Edge case: out of range RGB ===\n";
// Values outside 0-255 range
$text = new ContentNode("OutOfRange", ['color' => [300, -50, 1000]]);
echo "Created with out-of-range RGB\n";

echo "\n=== Edge case: hex without # ===\n";
$text = new ContentNode("NoHash", ['color' => 'ff0000']);
echo "Created with 'ff0000' (no #)\n";

echo "\n=== Edge case: invalid hex ===\n";
$text = new ContentNode("Invalid", ['color' => '#gggggg']);
echo "Created with invalid hex\n";

echo "\n=== Edge case: empty color ===\n";
$text = new ContentNode("Empty", ['color' => '']);
echo "Created with empty color\n";

echo "\nDone!\n";
?>
--EXPECTF--
=== Hex colors (6-digit) ===
Created with #ff0000
Created with #00FF00
Created with #0000ff

=== Hex colors (3-digit shorthand) ===
Created with #f00
Created with #fff

=== RGB array format ===
Created with [255, 0, 0]
Created with [0, 255, 0]
Created with [0, 0, 0]
Created with [255, 255, 255]

=== Background colors ===
Background #ff0000
Background [128, 128, 128]

=== Box border colors ===
Border color #00ff00

=== Named colors via Color enum ===
Red value: #%s
Blue value: #%s
Green value: #%s
White value: #%s
Black value: #%s

=== Color::fromName() ===
fromName('red'): #%s
fromName('coral'): #%s

Notice: %s::fromName(): Unknown color name 'notacolor' in %s on line %d
fromName('notacolor'): null

=== Color::toRgb() ===
Red RGB: r=%d, g=%d, b=%d
Lime RGB: r=%d, g=%d, b=%d

=== Color::toAnsi() and toAnsiBg() ===
Red ANSI (length): %d
Blue ANSI BG (length): %d

=== Edge case: out of range RGB ===
Created with out-of-range RGB

=== Edge case: hex without # ===
Created with 'ff0000' (no #)

=== Edge case: invalid hex ===
Created with invalid hex

=== Edge case: empty color ===
Created with empty color

Done!
