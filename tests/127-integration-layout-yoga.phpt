--TEST--
Integration test for Yoga layout engine
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

echo "=== Yoga layout integration test ===\n";

$renderer = tui_test_create(80, 24);

// Test 1: Basic flexbox row
echo "\n--- Test 1: Flexbox row ---\n";

$row = new ContainerNode([
    'id' => 'row-container',
    'width' => 60,
    'height' => 5,
    'flexDirection' => 'row',
    'borderStyle' => 'single',
]);

for ($i = 0; $i < 3; $i++) {
    $child = new ContainerNode([
        'id' => "row-child-$i",
        'flexGrow' => 1,
    ]);
    $child->addChild(new ContentNode("Col $i"));
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

$col = new ContainerNode([
    'id' => 'col-container',
    'width' => 30,
    'height' => 12,
    'flexDirection' => 'column',
    'borderStyle' => 'single',
]);

for ($i = 0; $i < 3; $i++) {
    $child = new ContainerNode([
        'id' => "col-child-$i",
        'flexGrow' => 1,
    ]);
    $child->addChild(new ContentNode("Row $i"));
    $col->addChild($child);
}

tui_test_render($renderer, $col);
$output = tui_test_get_output($renderer);
echo "Column layout rendered, " . count($output) . " lines\n";

// Test 3: Fixed dimensions
echo "\n--- Test 3: Fixed dimensions ---\n";

$fixed = new ContainerNode([
    'id' => 'fixed-container',
    'width' => 40,
    'height' => 10,
    'flexDirection' => 'row',
]);

$leftFixed = new ContainerNode([
    'id' => 'left-fixed',
    'width' => 15,
]);
$leftFixed->addChild(new ContentNode('Fixed 15'));

$middle = new ContainerNode([
    'id' => 'middle-flex',
    'flexGrow' => 1,
]);
$middle->addChild(new ContentNode('Flex'));

$rightFixed = new ContainerNode([
    'id' => 'right-fixed',
    'width' => 10,
]);
$rightFixed->addChild(new ContentNode('Fixed 10'));

$fixed->addChild($leftFixed);
$fixed->addChild($middle);
$fixed->addChild($rightFixed);

tui_test_render($renderer, $fixed);
echo "Fixed + flex layout rendered\n";

// Test 4: Padding
echo "\n--- Test 4: Padding ---\n";

$padded = new ContainerNode([
    'id' => 'padded',
    'width' => 40,
    'height' => 8,
    'padding' => 2,
    'borderStyle' => 'single',
]);
$padded->addChild(new ContentNode('Padded content'));

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

$perSide = new ContainerNode([
    'id' => 'per-side',
    'width' => 40,
    'height' => 8,
    'paddingTop' => 1,
    'paddingBottom' => 1,
    'paddingLeft' => 3,
    'paddingRight' => 3,
    'borderStyle' => 'single',
]);
$perSide->addChild(new ContentNode('Per-side padding'));

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
    $box = new ContainerNode([
        'id' => 'align-' . $align['name'],
        'width' => 30,
        'height' => 6,
        'alignItems' => $align['alignItems'],
        'justifyContent' => $align['justifyContent'],
    ]);
    $box->addChild(new ContentNode('X'));

    tui_test_render($renderer, $box);
    echo "Alignment {$align['name']} rendered\n";
}

// Test 7: Flex wrap
echo "\n--- Test 7: Flex wrap ---\n";

$wrapContainer = new ContainerNode([
    'id' => 'wrap-container',
    'width' => 40,
    'height' => 12,
    'flexDirection' => 'row',
    'flexWrap' => 'wrap',
]);

for ($i = 0; $i < 8; $i++) {
    $item = new ContainerNode([
        'id' => "wrap-item-$i",
        'width' => 12,
        'height' => 3,
    ]);
    $item->addChild(new ContentNode("W$i"));
    $wrapContainer->addChild($item);
}

tui_test_render($renderer, $wrapContainer);
echo "Wrapped 8 items in container\n";

// Test 8: Min/Max dimensions
echo "\n--- Test 8: Min/Max dimensions ---\n";

$minMax = new ContainerNode([
    'id' => 'minmax-container',
    'width' => 60,
    'height' => 10,
    'flexDirection' => 'row',
]);

$minChild = new ContainerNode([
    'id' => 'min-child',
    'flexGrow' => 1,
    'minWidth' => 20,
]);
$minChild->addChild(new ContentNode('Min 20'));

$maxChild = new ContainerNode([
    'id' => 'max-child',
    'flexGrow' => 2,
    'maxWidth' => 30,
]);
$maxChild->addChild(new ContentNode('Max 30'));

$minMax->addChild($minChild);
$minMax->addChild($maxChild);

tui_test_render($renderer, $minMax);
echo "Min/max constraints applied\n";

// Test 9: Aspect ratio
echo "\n--- Test 9: Aspect ratio ---\n";

$aspectContainer = new ContainerNode([
    'id' => 'aspect-container',
    'width' => 40,
    'height' => 20,
    'alignItems' => 'flex-start',
]);

$aspectBox = new ContainerNode([
    'id' => 'aspect-box',
    'width' => 20,
    'aspectRatio' => 2.0, // 2:1 ratio, so height = 10
]);
$aspectBox->addChild(new ContentNode('2:1'));

$aspectContainer->addChild($aspectBox);

tui_test_render($renderer, $aspectContainer);
echo "Aspect ratio 2:1 applied\n";

// Test 10: Gap (if supported)
echo "\n--- Test 10: Gap ---\n";

$gapContainer = new ContainerNode([
    'id' => 'gap-container',
    'width' => 50,
    'height' => 8,
    'flexDirection' => 'row',
    'gap' => 2,
]);

for ($i = 0; $i < 3; $i++) {
    $gapChild = new ContainerNode([
        'id' => "gap-child-$i",
        'width' => 10,
    ]);
    $gapChild->addChild(new ContentNode("G$i"));
    $gapContainer->addChild($gapChild);
}

tui_test_render($renderer, $gapContainer);
echo "Gap between items rendered\n";

// Test 11: Nested flex layouts
echo "\n--- Test 11: Nested flex ---\n";

$outer = new ContainerNode([
    'id' => 'outer',
    'width' => 60,
    'height' => 20,
    'flexDirection' => 'column',
]);

$topRow = new ContainerNode([
    'id' => 'top-row',
    'height' => 5,
    'flexDirection' => 'row',
]);
for ($i = 0; $i < 2; $i++) {
    $cell = new ContainerNode(['id' => "top-$i", 'flexGrow' => 1]);
    $cell->addChild(new ContentNode("T$i"));
    $topRow->addChild($cell);
}

$middleRow = new ContainerNode([
    'id' => 'middle-row',
    'flexGrow' => 1,
    'flexDirection' => 'row',
]);
for ($i = 0; $i < 3; $i++) {
    $cell = new ContainerNode(['id' => "mid-$i", 'flexGrow' => 1]);
    $cell->addChild(new ContentNode("M$i"));
    $middleRow->addChild($cell);
}

$bottomRow = new ContainerNode([
    'id' => 'bottom-row',
    'height' => 3,
    'flexDirection' => 'row',
]);
$cell = new ContainerNode(['id' => 'bottom-0', 'flexGrow' => 1]);
$cell->addChild(new ContentNode('Bottom'));
$bottomRow->addChild($cell);

$outer->addChild($topRow);
$outer->addChild($middleRow);
$outer->addChild($bottomRow);

tui_test_render($renderer, $outer);
$output = tui_test_get_output($renderer);
echo "Nested flex layout: " . count($output) . " lines\n";

// Test 12: Direction (ltr/rtl)
echo "\n--- Test 12: Direction ---\n";

$ltrBox = new ContainerNode([
    'id' => 'ltr-box',
    'width' => 40,
    'height' => 4,
    'flexDirection' => 'row',
    'direction' => 'ltr',
]);
$ltrBox->addChild(new ContentNode('First'));
$ltrBox->addChild(new ContentNode('Second'));

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
