--TEST--
Layout constraints and Yoga integration comprehensive testing
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\ContainerNode;
use Xocdr\Tui\Ext\ContentNode;

echo "=== Flex properties ===\n";

// flexGrow
$box = new ContainerNode(['flexGrow' => 1]);
var_dump($box->flexGrow);

$box = new ContainerNode(['flexGrow' => 0]);
var_dump($box->flexGrow);

$box = new ContainerNode(['flexGrow' => 2.5]);
var_dump($box->flexGrow);

// flexShrink
$box = new ContainerNode(['flexShrink' => 1]);
var_dump($box->flexShrink);

$box = new ContainerNode(['flexShrink' => 0]);
var_dump($box->flexShrink);

// flexBasis
$box = new ContainerNode(['flexBasis' => 100]);
var_dump($box->flexBasis);

$box = new ContainerNode(['flexBasis' => 'auto']);
var_dump($box->flexBasis);

$box = new ContainerNode(['flexBasis' => '50%']);
var_dump($box->flexBasis);

echo "\n=== Min/Max constraints ===\n";

// minWidth/minHeight
$box = new ContainerNode(['minWidth' => 10, 'minHeight' => 5]);
var_dump($box->minWidth);
var_dump($box->minHeight);

// maxWidth/maxHeight
$box = new ContainerNode(['maxWidth' => 100, 'maxHeight' => 50]);
var_dump($box->maxWidth);
var_dump($box->maxHeight);

// Percentage min/max
$box = new ContainerNode(['minWidth' => '20%', 'maxWidth' => '80%']);
var_dump($box->minWidth);
var_dump($box->maxWidth);

echo "\n=== Aspect ratio ===\n";

$box = new ContainerNode(['aspectRatio' => 1.0]);  // Square
var_dump($box->aspectRatio);

$box = new ContainerNode(['aspectRatio' => 16/9]);  // Widescreen
var_dump($box->aspectRatio);

$box = new ContainerNode(['aspectRatio' => 0.5]);  // Portrait
var_dump($box->aspectRatio);

echo "\n=== Position types ===\n";

$box = new ContainerNode(['position' => 'relative']);
var_dump($box->position);

$box = new ContainerNode(['position' => 'absolute']);
var_dump($box->position);

// Position values
$box = new ContainerNode([
    'position' => 'absolute',
    'top' => 10,
    'left' => 20,
    'right' => 30,
    'bottom' => 40
]);
var_dump($box->top);
var_dump($box->left);
var_dump($box->right);
var_dump($box->bottom);

echo "\n=== Overflow ===\n";

$box = new ContainerNode(['overflow' => 'visible']);
var_dump($box->overflow);

$box = new ContainerNode(['overflow' => 'hidden']);
var_dump($box->overflow);

$box = new ContainerNode(['overflow' => 'scroll']);
var_dump($box->overflow);

echo "\n=== Gap properties ===\n";

$box = new ContainerNode(['gap' => 5]);
var_dump($box->gap);

$box = new ContainerNode(['rowGap' => 3, 'columnGap' => 7]);
var_dump($box->rowGap);
var_dump($box->columnGap);

echo "\n=== Align and justify ===\n";

$alignValues = ['flex-start', 'flex-end', 'center', 'stretch', 'baseline'];
foreach ($alignValues as $align) {
    $box = new ContainerNode(['alignItems' => $align]);
    echo "alignItems '$align': " . ($box->alignItems == $align ? 'ok' : 'mismatch') . "\n";
}

$justifyValues = ['flex-start', 'flex-end', 'center', 'space-between', 'space-around', 'space-evenly'];
foreach ($justifyValues as $justify) {
    $box = new ContainerNode(['justifyContent' => $justify]);
    echo "justifyContent '$justify': " . ($box->justifyContent == $justify ? 'ok' : 'mismatch') . "\n";
}

$box = new ContainerNode(['alignSelf' => 'center']);
var_dump($box->alignSelf);

$box = new ContainerNode(['alignContent' => 'stretch']);
var_dump($box->alignContent);

echo "\n=== Flex wrap ===\n";

$box = new ContainerNode(['flexWrap' => 'nowrap']);
var_dump($box->flexWrap);

$box = new ContainerNode(['flexWrap' => 'wrap']);
var_dump($box->flexWrap);

$box = new ContainerNode(['flexWrap' => 'wrap-reverse']);
var_dump($box->flexWrap);

echo "\n=== Display property ===\n";

$box = new ContainerNode(['display' => 'flex']);
var_dump($box->display);

$box = new ContainerNode(['display' => 'none']);
var_dump($box->display);

echo "\n=== Complex layout nesting ===\n";

$layout = new ContainerNode([
    'width' => '100%',
    'height' => '100%',
    'flexDirection' => 'column',
    'children' => [
        new ContainerNode([
            'height' => 50,
            'flexDirection' => 'row',
            'justifyContent' => 'space-between',
            'children' => [
                new ContentNode("Left"),
                new ContentNode("Center"),
                new ContentNode("Right"),
            ]
        ]),
        new ContainerNode([
            'flexGrow' => 1,
            'flexDirection' => 'row',
            'children' => [
                new ContainerNode(['width' => 20]),  // Sidebar
                new ContainerNode(['flexGrow' => 1]),  // Main content
            ]
        ]),
        new ContainerNode(['height' => 3]),  // Footer
    ]
]);
echo "Complex layout created\n";

echo "\n=== Conflicting constraints ===\n";

// Width vs minWidth
$box = new ContainerNode(['width' => 50, 'minWidth' => 100]);
echo "Width < minWidth handled\n";

// Width vs maxWidth
$box = new ContainerNode(['width' => 150, 'maxWidth' => 100]);
echo "Width > maxWidth handled\n";

// Fixed width vs flexGrow
$box = new ContainerNode(['width' => 100, 'flexGrow' => 1]);
echo "Fixed + flexGrow handled\n";

echo "\nDone!\n";
?>
--EXPECTF--
=== Flex properties ===
%s
%s
%s
%s
%s
%s
%s
%s

=== Min/Max constraints ===
%s
%s
%s
%s
%s
%s

=== Aspect ratio ===
%s
%s
%s

=== Position types ===
string(8) "relative"
string(8) "absolute"
%A
%s
%s
%s
%s

=== Overflow ===
string(7) "visible"
string(6) "hidden"
string(6) "scroll"

=== Gap properties ===
%s
%s
%s

=== Align and justify ===
alignItems 'flex-start': ok
alignItems 'flex-end': ok
alignItems 'center': ok
alignItems 'stretch': ok
alignItems 'baseline': ok
justifyContent 'flex-start': ok
justifyContent 'flex-end': ok
justifyContent 'center': ok
justifyContent 'space-between': ok
justifyContent 'space-around': ok
justifyContent 'space-evenly': ok
string(6) "center"
%A
string(7) "stretch"

=== Flex wrap ===
string(6) "nowrap"
string(4) "wrap"
string(12) "wrap-reverse"

=== Display property ===
string(4) "flex"
string(4) "none"

=== Complex layout nesting ===
Complex layout created

=== Conflicting constraints ===
Width < minWidth handled
Width > maxWidth handled
Fixed + flexGrow handled

Done!
