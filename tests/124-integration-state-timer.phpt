--TEST--
Integration test for state management and timers
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

echo "=== State and Timer integration test ===\n";

$renderer = tui_test_create(80, 24);

// Test 1: State updates trigger proper re-renders
echo "\n--- Test 1: State updates ---\n";

$counter = 0;
$renderCount = 0;

$root = new ContainerNode([
    'id' => 'root',
    'width' => 80,
    'height' => 24,
]);
$root->addChild(new ContentNode("Counter: $counter"));

// Initial render
tui_test_render($renderer, $root);
$renderCount++;
echo "Initial render, counter = $counter\n";

// Simulate state updates
for ($i = 1; $i <= 5; $i++) {
    $counter = $i;
    $root = new ContainerNode([
        'id' => 'root',
        'width' => 80,
        'height' => 24,
    ]);
    $root->addChild(new ContentNode("Counter: $counter"));
    tui_test_render($renderer, $root);
    $renderCount++;
}

echo "After 5 updates, counter = $counter, renders = $renderCount\n";

$output = tui_test_get_output($renderer);
$found = false;
foreach ($output as $line) {
    if (strpos($line, 'Counter: 5') !== false) $found = true;
}
echo "Final counter value found: " . ($found ? "yes" : "no") . "\n";

// Test 2: Multiple timers
echo "\n--- Test 2: Multiple timer simulation ---\n";

$timer1Fired = 0;
$timer2Fired = 0;
$timer3Fired = 0;

// We simulate timer behavior by tracking elapsed time
$elapsed = 0;
$intervals = [100, 250, 500]; // Timer intervals in ms
$lastFired = [0, 0, 0];

// Simulate 1 second of time passing in 10ms increments
for ($t = 0; $t <= 1000; $t += 10) {
    if ($t - $lastFired[0] >= $intervals[0]) {
        $timer1Fired++;
        $lastFired[0] = $t;
    }
    if ($t - $lastFired[1] >= $intervals[1]) {
        $timer2Fired++;
        $lastFired[1] = $t;
    }
    if ($t - $lastFired[2] >= $intervals[2]) {
        $timer3Fired++;
        $lastFired[2] = $t;
    }
}

echo "Timer 1 (100ms) fired: $timer1Fired times\n";
echo "Timer 2 (250ms) fired: $timer2Fired times\n";
echo "Timer 3 (500ms) fired: $timer3Fired times\n";

// Test 3: State-driven conditional rendering
echo "\n--- Test 3: Conditional rendering ---\n";

$showModal = false;

function renderApp($showModal) {
    $root = new ContainerNode([
        'id' => 'app',
        'width' => 80,
        'height' => 24,
        'flexDirection' => 'column',
    ]);

    $header = new ContainerNode(['id' => 'header', 'height' => 3]);
    $header->addChild(new ContentNode('My App'));
    $root->addChild($header);

    $content = new ContainerNode(['id' => 'content', 'flexGrow' => 1]);
    $content->addChild(new ContentNode('Main content here'));
    $root->addChild($content);

    if ($showModal) {
        $modal = new ContainerNode([
            'id' => 'modal',
            'borderStyle' => 'double',
            'width' => 40,
            'height' => 10,
        ]);
        $modal->addChild(new ContentNode('Modal dialog'));
        $root->addChild($modal);
    }

    return $root;
}

// Render without modal
tui_test_render($renderer, renderApp(false));
$output1 = tui_test_get_output($renderer);
$hasModal1 = false;
foreach ($output1 as $line) {
    if (strpos($line, 'Modal') !== false) $hasModal1 = true;
}
echo "Without modal - Modal visible: " . ($hasModal1 ? "yes" : "no") . "\n";

// Render with modal
tui_test_render($renderer, renderApp(true));
$output2 = tui_test_get_output($renderer);
$hasModal2 = false;
foreach ($output2 as $line) {
    if (strpos($line, 'Modal') !== false) $hasModal2 = true;
}
echo "With modal - Modal visible: " . ($hasModal2 ? "yes" : "no") . "\n";

// Test 4: List rendering with dynamic items
echo "\n--- Test 4: Dynamic list rendering ---\n";

function renderList($items) {
    $root = new ContainerNode([
        'id' => 'list-container',
        'width' => 40,
        'height' => 20,
        'flexDirection' => 'column',
    ]);

    foreach ($items as $i => $item) {
        $itemBox = new ContainerNode([
            'id' => "item-$i",
            'height' => 1,
        ]);
        $itemBox->addChild(new ContentNode("• $item"));
        $root->addChild($itemBox);
    }

    return $root;
}

$items = ['Apple', 'Banana', 'Cherry'];
tui_test_render($renderer, renderList($items));
$output3 = tui_test_get_output($renderer);

$foundItems = 0;
foreach ($output3 as $line) {
    foreach ($items as $item) {
        if (strpos($line, $item) !== false) $foundItems++;
    }
}
echo "Found $foundItems of " . count($items) . " items\n";

// Add more items
$items = ['Apple', 'Banana', 'Cherry', 'Date', 'Elderberry'];
tui_test_render($renderer, renderList($items));
$output4 = tui_test_get_output($renderer);

$foundItems = 0;
foreach ($output4 as $line) {
    foreach ($items as $item) {
        if (strpos($line, $item) !== false) $foundItems++;
    }
}
echo "After adding items: Found $foundItems of " . count($items) . " items\n";

// Test 5: Keyed list reordering
echo "\n--- Test 5: Keyed list reordering ---\n";

function renderKeyedList($items) {
    $root = new ContainerNode([
        'id' => 'keyed-list',
        'width' => 40,
        'flexDirection' => 'column',
    ]);

    foreach ($items as $id => $label) {
        $itemBox = new ContainerNode([
            'key' => "item-$id",
            'id' => "box-$id",
            'height' => 1,
        ]);
        $itemBox->addChild(new ContentNode($label));
        $root->addChild($itemBox);
    }

    return $root;
}

$keyedItems = [1 => 'First', 2 => 'Second', 3 => 'Third'];
tui_test_render($renderer, renderKeyedList($keyedItems));
echo "Initial keyed list rendered\n";

// Reorder
$keyedItems = [3 => 'Third', 1 => 'First', 2 => 'Second'];
tui_test_render($renderer, renderKeyedList($keyedItems));
echo "Reordered keyed list rendered\n";

// Test 6: Complex state object
echo "\n--- Test 6: Complex state object ---\n";

$appState = [
    'user' => ['name' => 'John', 'role' => 'admin'],
    'theme' => 'dark',
    'notifications' => [
        ['id' => 1, 'text' => 'Welcome!'],
        ['id' => 2, 'text' => 'New message'],
    ],
    'settings' => [
        'autoSave' => true,
        'fontSize' => 14,
    ],
];

function renderFromState($state) {
    $root = new ContainerNode([
        'id' => 'stateful-app',
        'width' => 60,
        'height' => 20,
        'flexDirection' => 'column',
    ]);

    // User info
    $userInfo = new ContainerNode(['id' => 'user-info', 'height' => 2]);
    $userInfo->addChild(new ContentNode("User: {$state['user']['name']} ({$state['user']['role']})"));
    $root->addChild($userInfo);

    // Theme indicator
    $theme = new ContainerNode(['id' => 'theme', 'height' => 1]);
    $theme->addChild(new ContentNode("Theme: {$state['theme']}"));
    $root->addChild($theme);

    // Notifications
    $notifs = new ContainerNode(['id' => 'notifications', 'flexGrow' => 1, 'flexDirection' => 'column']);
    foreach ($state['notifications'] as $notif) {
        $notifBox = new ContainerNode(['id' => "notif-{$notif['id']}", 'height' => 1]);
        $notifBox->addChild(new ContentNode("📬 {$notif['text']}"));
        $notifs->addChild($notifBox);
    }
    $root->addChild($notifs);

    return $root;
}

tui_test_render($renderer, renderFromState($appState));
$stateOutput = tui_test_get_output($renderer);

$foundUser = false;
$foundTheme = false;
$foundNotifs = 0;

foreach ($stateOutput as $line) {
    if (strpos($line, 'John') !== false) $foundUser = true;
    if (strpos($line, 'dark') !== false) $foundTheme = true;
    if (strpos($line, 'Welcome') !== false) $foundNotifs++;
    if (strpos($line, 'New message') !== false) $foundNotifs++;
}

echo "User found: " . ($foundUser ? "yes" : "no") . "\n";
echo "Theme found: " . ($foundTheme ? "yes" : "no") . "\n";
echo "Notifications found: $foundNotifs\n";

// Update state
$appState['theme'] = 'light';
$appState['notifications'][] = ['id' => 3, 'text' => 'Update available'];

tui_test_render($renderer, renderFromState($appState));
$updatedOutput = tui_test_get_output($renderer);

$foundLight = false;
$foundUpdate = false;
foreach ($updatedOutput as $line) {
    if (strpos($line, 'light') !== false) $foundLight = true;
    if (strpos($line, 'Update available') !== false) $foundUpdate = true;
}

echo "Theme updated to light: " . ($foundLight ? "yes" : "no") . "\n";
echo "New notification found: " . ($foundUpdate ? "yes" : "no") . "\n";

tui_test_destroy($renderer);

echo "\n=== State/Timer integration tests passed ===\n";
?>
--EXPECT--
=== State and Timer integration test ===

--- Test 1: State updates ---
Initial render, counter = 0
After 5 updates, counter = 5, renders = 6
Final counter value found: yes

--- Test 2: Multiple timer simulation ---
Timer 1 (100ms) fired: 10 times
Timer 2 (250ms) fired: 4 times
Timer 3 (500ms) fired: 2 times

--- Test 3: Conditional rendering ---
Without modal - Modal visible: no
With modal - Modal visible: yes

--- Test 4: Dynamic list rendering ---
Found 3 of 3 items
After adding items: Found 5 of 5 items

--- Test 5: Keyed list reordering ---
Initial keyed list rendered
Reordered keyed list rendered

--- Test 6: Complex state object ---
User found: yes
Theme found: yes
Notifications found: 2
Theme updated to light: yes
New notification found: yes

=== State/Timer integration tests passed ===
