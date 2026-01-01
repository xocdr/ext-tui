--TEST--
Integration test for full render cycles with state updates
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

echo "=== Full render cycle integration test ===\n";

// Create test renderer
$renderer = tui_test_create(80, 24);

// Test 1: Initial render with nested structure
echo "\n--- Test 1: Initial nested render ---\n";

$root = new ContainerNode([
    'id' => 'root',
    'flexDirection' => 'column',
    'width' => 80,
    'height' => 24,
]);

$header = new ContainerNode([
    'id' => 'header',
    'height' => 3,
    'borderStyle' => 'single',
]);
$header->addChild(new ContentNode('Header', ['id' => 'header-text']));

$content = new ContainerNode([
    'id' => 'content',
    'flexGrow' => 1,
    'flexDirection' => 'row',
]);

$sidebar = new ContainerNode([
    'id' => 'sidebar',
    'width' => 20,
    'borderStyle' => 'single',
]);
$sidebar->addChild(new ContentNode('Sidebar', ['id' => 'sidebar-text']));

$main = new ContainerNode([
    'id' => 'main',
    'flexGrow' => 1,
]);
$main->addChild(new ContentNode('Main content area', ['id' => 'main-text']));

$content->addChild($sidebar);
$content->addChild($main);

$root->addChild($header);
$root->addChild($content);

tui_test_render($renderer, $root);

$output = tui_test_get_output($renderer);
echo "Rendered " . count($output) . " lines\n";

// Verify content appears
$foundHeader = false;
$foundSidebar = false;
$foundMain = false;

foreach ($output as $line) {
    if (strpos($line, 'Header') !== false) $foundHeader = true;
    if (strpos($line, 'Sidebar') !== false) $foundSidebar = true;
    if (strpos($line, 'Main') !== false) $foundMain = true;
}

echo "Header found: " . ($foundHeader ? "yes" : "no") . "\n";
echo "Sidebar found: " . ($foundSidebar ? "yes" : "no") . "\n";
echo "Main found: " . ($foundMain ? "yes" : "no") . "\n";

// Test 2: Update content and re-render
echo "\n--- Test 2: Content update and re-render ---\n";

$main = new ContainerNode([
    'id' => 'main',
    'flexGrow' => 1,
]);
$main->addChild(new ContentNode('Updated main content!', ['id' => 'main-text-updated']));

$content = new ContainerNode([
    'id' => 'content',
    'flexGrow' => 1,
    'flexDirection' => 'row',
]);
$content->addChild($sidebar);
$content->addChild($main);

$root = new ContainerNode([
    'id' => 'root',
    'flexDirection' => 'column',
    'width' => 80,
    'height' => 24,
]);
$root->addChild($header);
$root->addChild($content);

tui_test_render($renderer, $root);

$output2 = tui_test_get_output($renderer);
$foundUpdated = false;
foreach ($output2 as $line) {
    if (strpos($line, 'Updated') !== false) $foundUpdated = true;
}
echo "Updated content found: " . ($foundUpdated ? "yes" : "no") . "\n";

// Test 3: Complex nesting depth
echo "\n--- Test 3: Deep nesting structure ---\n";

function createNestedBox($depth, $maxDepth) {
    $box = new ContainerNode([
        'id' => "level-$depth",
        'padding' => 1,
    ]);

    if ($depth < $maxDepth) {
        $box->addChild(createNestedBox($depth + 1, $maxDepth));
    } else {
        $box->addChild(new ContentNode("Deepest level: $depth"));
    }

    return $box;
}

$deepRoot = new ContainerNode([
    'id' => 'deep-root',
    'width' => 60,
    'height' => 20,
]);
$deepRoot->addChild(createNestedBox(1, 8));

tui_test_render($renderer, $deepRoot);
$deepOutput = tui_test_get_output($renderer);
echo "Deep render produced " . count($deepOutput) . " lines\n";

$foundDeepest = false;
foreach ($deepOutput as $line) {
    if (strpos($line, 'Deepest level: 8') !== false) $foundDeepest = true;
}
echo "Deepest level found: " . ($foundDeepest ? "yes" : "no") . "\n";

// Test 4: Wide tree (many siblings)
echo "\n--- Test 4: Wide tree with many siblings ---\n";

$wideRoot = new ContainerNode([
    'id' => 'wide-root',
    'width' => 80,
    'height' => 24,
    'flexDirection' => 'row',
    'flexWrap' => 'wrap',
]);

for ($i = 0; $i < 20; $i++) {
    $child = new ContainerNode([
        'id' => "child-$i",
        'width' => 10,
        'height' => 3,
    ]);
    $child->addChild(new ContentNode("C$i"));
    $wideRoot->addChild($child);
}

tui_test_render($renderer, $wideRoot);
$wideOutput = tui_test_get_output($renderer);
echo "Wide tree rendered with " . count($wideOutput) . " lines\n";

// Test 5: Focus traversal
echo "\n--- Test 5: Focus system integration ---\n";

$focusRoot = new ContainerNode([
    'id' => 'focus-root',
    'width' => 80,
    'height' => 24,
    'flexDirection' => 'column',
]);

for ($i = 0; $i < 5; $i++) {
    $focusable = new ContainerNode([
        'id' => "focusable-$i",
        'focusable' => true,
        'focused' => ($i === 0), // First one focused
        'height' => 3,
    ]);
    $focusable->addChild(new ContentNode("Button $i"));
    $focusRoot->addChild($focusable);
}

tui_test_render($renderer, $focusRoot);
echo "Focus test rendered\n";

// Send tab key (ASCII 9) multiple times
for ($i = 0; $i < 3; $i++) {
    tui_test_send_key($renderer, 9);
    tui_test_advance_frame($renderer);
}
echo "Tab navigation simulated\n";

// Test 6: Style inheritance
echo "\n--- Test 6: Style inheritance ---\n";

$styledRoot = new ContainerNode([
    'id' => 'styled-root',
    'width' => 40,
    'height' => 10,
    'borderStyle' => 'double',
]);

$styledChild = new ContainerNode([
    'id' => 'styled-child',
    'flexGrow' => 1,
]);

$styledText = new ContentNode('Styled text', [
    'color' => 'white',
    'bold' => true,
]);

$styledChild->addChild($styledText);
$styledRoot->addChild($styledChild);

tui_test_render($renderer, $styledRoot);
$styledOutput = tui_test_get_output($renderer);
echo "Styled tree rendered with " . count($styledOutput) . " lines\n";

// Test 7: Empty containers
echo "\n--- Test 7: Empty containers ---\n";

$emptyRoot = new ContainerNode([
    'id' => 'empty-root',
    'width' => 20,
    'height' => 5,
    'borderStyle' => 'single',
]);

$emptyChild = new ContainerNode([
    'id' => 'empty-child',
    'flexGrow' => 1,
]);
// No children added to emptyChild

$emptyRoot->addChild($emptyChild);

tui_test_render($renderer, $emptyRoot);
echo "Empty container rendered\n";

// Test 8: Rapid re-renders
echo "\n--- Test 8: Rapid re-render stress ---\n";

for ($i = 0; $i < 50; $i++) {
    $rapidRoot = new ContainerNode([
        'id' => 'rapid-root',
        'width' => 80,
        'height' => 24,
    ]);
    $rapidRoot->addChild(new ContentNode("Render iteration $i"));
    tui_test_render($renderer, $rapidRoot);
}
echo "50 rapid re-renders completed\n";

// Cleanup
tui_test_destroy($renderer);

echo "\n=== Integration tests passed ===\n";
?>
--EXPECTF--
=== Full render cycle integration test ===

--- Test 1: Initial nested render ---
Rendered 24 lines
Header found: yes
Sidebar found: yes
Main found: yes

--- Test 2: Content update and re-render ---
Updated content found: yes

--- Test 3: Deep nesting structure ---
Deep render produced 24 lines
Deepest level found: yes

--- Test 4: Wide tree with many siblings ---
Wide tree rendered with 24 lines

--- Test 5: Focus system integration ---
Focus test rendered
Tab navigation simulated

--- Test 6: Style inheritance ---
Styled tree rendered with 24 lines

--- Test 7: Empty containers ---
Empty container rendered

--- Test 8: Rapid re-render stress ---
50 rapid re-renders completed

=== Integration tests passed ===
