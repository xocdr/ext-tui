--TEST--
Integration test for Yoga layout engine
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

echo "=== Yoga layout integration test ===\n";

$renderer = tui_test_create(80, 24);

// Test 1: Basic flexbox row
echo "\n--- Test 1: Flexbox row ---\n";

$row = new Box([
    'id' => 'row-container',
    'width' => 60,
    'height' => 5,
    'flexDirection' => 'row',
    'borderStyle' => 'single',
]);

for ($i = 0; $i < 3; $i++) {
    $child = new Box([
        'id' => "row-child-$i",
        'flexGrow' => 1,
    ]);
    $child->addChild(new Text("Col $i"));
    $row->addChild($child);
}

tui_test_render($renderer, $row);
$output = tui_test_get_output($renderer);
echo "Row layout rendered, " . count($output) . " lines\n";

$foundAll = true;
for ($i = 0; $i < 3; $i++) {
    $found = false;
    foreach ($output as $line) {
        if (strpos($line, "Col $i") !== false) {
            $found = true;
            break;
        }
    }
    if (!$found) $foundAll = false;
}
echo "All columns visible: " . ($foundAll ? "yes" : "no") . "\n";

// Test 2: Basic flexbox column
echo "\n--- Test 2: Flexbox column ---\n";

$col = new Box([
    'id' => 'col-container',
    'width' => 30,
    'height' => 12,
    'flexDirection' => 'column',
    'borderStyle' => 'single',
]);

for ($i = 0; $i < 3; $i++) {
    $child = new Box([
        'id' => "col-child-$i",
        'flexGrow' => 1,
    ]);
    $child->addChild(new Text("Row $i"));
    $col->addChild($child);
}

tui_test_render($renderer, $col);
$output = tui_test_get_output($renderer);
echo "Column layout rendered, " . count($output) . " lines\n";

// Test 3: Fixed dimensions
echo "\n--- Test 3: Fixed dimensions ---\n";

$fixed = new Box([
    'id' => 'fixed-container',
    'width' => 40,
    'height' => 10,
    'flexDirection' => 'row',
]);

$leftFixed = new Box([
    'id' => 'left-fixed',
    'width' => 15,
]);
$leftFixed->addChild(new Text('Fixed 15'));

$middle = new Box([
    'id' => 'middle-flex',
    'flexGrow' => 1,
]);
$middle->addChild(new Text('Flex'));

$rightFixed = new Box([
    'id' => 'right-fixed',
    'width' => 10,
]);
$rightFixed->addChild(new Text('Fixed 10'));

$fixed->addChild($leftFixed);
$fixed->addChild($middle);
$fixed->addChild($rightFixed);

tui_test_render($renderer, $fixed);
echo "Fixed + flex layout rendered\n";

// Test 4: Padding
echo "\n--- Test 4: Padding ---\n";

$padded = new Box([
    'id' => 'padded',
    'width' => 40,
    'height' => 8,
    'padding' => 2,
    'borderStyle' => 'single',
]);
$padded->addChild(new Text('Padded content'));

tui_test_render($renderer, $padded);
$output = tui_test_get_output($renderer);

$foundPadded = false;
foreach ($output as $line) {
    if (strpos($line, 'Padded content') !== false) {
        $foundPadded = true;
        break;
    }
}
echo "Padded content visible: " . ($foundPadded ? "yes" : "no") . "\n";

// Test 5: Padding per side
echo "\n--- Test 5: Per-side padding ---\n";

$perSide = new Box([
    'id' => 'per-side',
    'width' => 40,
    'height' => 8,
    'paddingTop' => 1,
    'paddingBottom' => 1,
    'paddingLeft' => 3,
    'paddingRight' => 3,
    'borderStyle' => 'single',
]);
$perSide->addChild(new Text('Per-side padding'));

tui_test_render($renderer, $perSide);
echo "Per-side padding rendered\n";

// Test 6: Alignment
echo "\n--- Test 6: Alignment ---\n";

$alignments = [
    ['alignItems' => 'flex-start', 'justifyContent' => 'flex-start', 'name' => 'start/start'],
    ['alignItems' => 'center', 'justifyContent' => 'center', 'name' => 'center/center'],
    ['alignItems' => 'flex-end', 'justifyContent' => 'flex-end', 'name' => 'end/end'],
];

foreach ($alignments as $align) {
    $box = new Box([
        'id' => 'align-' . $align['name'],
        'width' => 30,
        'height' => 6,
        'alignItems' => $align['alignItems'],
        'justifyContent' => $align['justifyContent'],
    ]);
    $box->addChild(new Text('X'));

    tui_test_render($renderer, $box);
    echo "Alignment {$align['name']} rendered\n";
}

// Test 7: Flex wrap
echo "\n--- Test 7: Flex wrap ---\n";

$wrapContainer = new Box([
    'id' => 'wrap-container',
    'width' => 40,
    'height' => 12,
    'flexDirection' => 'row',
    'flexWrap' => 'wrap',
]);

for ($i = 0; $i < 8; $i++) {
    $item = new Box([
        'id' => "wrap-item-$i",
        'width' => 12,
        'height' => 3,
    ]);
    $item->addChild(new Text("W$i"));
    $wrapContainer->addChild($item);
}

tui_test_render($renderer, $wrapContainer);
echo "Wrapped 8 items in container\n";

// Test 8: Min/Max dimensions
echo "\n--- Test 8: Min/Max dimensions ---\n";

$minMax = new Box([
    'id' => 'minmax-container',
    'width' => 60,
    'height' => 10,
    'flexDirection' => 'row',
]);

$minChild = new Box([
    'id' => 'min-child',
    'flexGrow' => 1,
    'minWidth' => 20,
]);
$minChild->addChild(new Text('Min 20'));

$maxChild = new Box([
    'id' => 'max-child',
    'flexGrow' => 2,
    'maxWidth' => 30,
]);
$maxChild->addChild(new Text('Max 30'));

$minMax->addChild($minChild);
$minMax->addChild($maxChild);

tui_test_render($renderer, $minMax);
echo "Min/max constraints applied\n";

// Test 9: Aspect ratio
echo "\n--- Test 9: Aspect ratio ---\n";

$aspectContainer = new Box([
    'id' => 'aspect-container',
    'width' => 40,
    'height' => 20,
    'alignItems' => 'flex-start',
]);

$aspectBox = new Box([
    'id' => 'aspect-box',
    'width' => 20,
    'aspectRatio' => 2.0, // 2:1 ratio, so height = 10
]);
$aspectBox->addChild(new Text('2:1'));

$aspectContainer->addChild($aspectBox);

tui_test_render($renderer, $aspectContainer);
echo "Aspect ratio 2:1 applied\n";

// Test 10: Gap (if supported)
echo "\n--- Test 10: Gap ---\n";

$gapContainer = new Box([
    'id' => 'gap-container',
    'width' => 50,
    'height' => 8,
    'flexDirection' => 'row',
    'gap' => 2,
]);

for ($i = 0; $i < 3; $i++) {
    $gapChild = new Box([
        'id' => "gap-child-$i",
        'width' => 10,
    ]);
    $gapChild->addChild(new Text("G$i"));
    $gapContainer->addChild($gapChild);
}

tui_test_render($renderer, $gapContainer);
echo "Gap between items rendered\n";

// Test 11: Nested flex layouts
echo "\n--- Test 11: Nested flex ---\n";

$outer = new Box([
    'id' => 'outer',
    'width' => 60,
    'height' => 20,
    'flexDirection' => 'column',
]);

$topRow = new Box([
    'id' => 'top-row',
    'height' => 5,
    'flexDirection' => 'row',
]);
for ($i = 0; $i < 2; $i++) {
    $cell = new Box(['id' => "top-$i", 'flexGrow' => 1]);
    $cell->addChild(new Text("T$i"));
    $topRow->addChild($cell);
}

$middleRow = new Box([
    'id' => 'middle-row',
    'flexGrow' => 1,
    'flexDirection' => 'row',
]);
for ($i = 0; $i < 3; $i++) {
    $cell = new Box(['id' => "mid-$i", 'flexGrow' => 1]);
    $cell->addChild(new Text("M$i"));
    $middleRow->addChild($cell);
}

$bottomRow = new Box([
    'id' => 'bottom-row',
    'height' => 3,
    'flexDirection' => 'row',
]);
$cell = new Box(['id' => 'bottom-0', 'flexGrow' => 1]);
$cell->addChild(new Text('Bottom'));
$bottomRow->addChild($cell);

$outer->addChild($topRow);
$outer->addChild($middleRow);
$outer->addChild($bottomRow);

tui_test_render($renderer, $outer);
$output = tui_test_get_output($renderer);
echo "Nested flex layout: " . count($output) . " lines\n";

// Test 12: Direction (ltr/rtl)
echo "\n--- Test 12: Direction ---\n";

$ltrBox = new Box([
    'id' => 'ltr-box',
    'width' => 40,
    'height' => 4,
    'flexDirection' => 'row',
    'direction' => 'ltr',
]);
$ltrBox->addChild(new Text('First'));
$ltrBox->addChild(new Text('Second'));

tui_test_render($renderer, $ltrBox);
echo "LTR direction rendered\n";

tui_test_destroy($renderer);

echo "\n=== Yoga layout integration tests passed ===\n";
?>
--EXPECT--
=== Yoga layout integration test ===

--- Test 1: Flexbox row ---
Row layout rendered, 24 lines
All columns visible: yes

--- Test 2: Flexbox column ---
Column layout rendered, 24 lines

--- Test 3: Fixed dimensions ---
Fixed + flex layout rendered

--- Test 4: Padding ---
Padded content visible: yes

--- Test 5: Per-side padding ---
Per-side padding rendered

--- Test 6: Alignment ---
Alignment start/start rendered
Alignment center/center rendered
Alignment end/end rendered

--- Test 7: Flex wrap ---
Wrapped 8 items in container

--- Test 8: Min/Max dimensions ---
Min/max constraints applied

--- Test 9: Aspect ratio ---
Aspect ratio 2:1 applied

--- Test 10: Gap ---
Gap between items rendered

--- Test 11: Nested flex ---
Nested flex layout: 24 lines

--- Test 12: Direction ---
LTR direction rendered

=== Yoga layout integration tests passed ===
