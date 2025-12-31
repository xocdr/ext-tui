--TEST--
Yoga layout comprehensive testing
--EXTENSIONS--
tui
--FILE--
<?php
use Xocdr\Tui\Ext\Box;
use Xocdr\Tui\Ext\Text;

echo "=== Flex direction ===\n";
foreach (['column', 'row', 'column-reverse', 'row-reverse'] as $dir) {
    $box = new Box(['flexDirection' => $dir]);
    echo "flexDirection '$dir': " . $box->flexDirection . "\n";
}

echo "\n=== Align items ===\n";
foreach (['flex-start', 'center', 'flex-end', 'stretch', 'baseline'] as $align) {
    $box = new Box(['alignItems' => $align]);
    echo "alignItems '$align': " . $box->alignItems . "\n";
}

echo "\n=== Justify content ===\n";
foreach (['flex-start', 'center', 'flex-end', 'space-between', 'space-around', 'space-evenly'] as $justify) {
    $box = new Box(['justifyContent' => $justify]);
    echo "justifyContent '$justify': " . $box->justifyContent . "\n";
}

echo "\n=== Align self ===\n";
foreach (['auto', 'flex-start', 'center', 'flex-end', 'stretch', 'baseline'] as $align) {
    $box = new Box(['alignSelf' => $align]);
    echo "alignSelf '$align': " . $box->alignSelf . "\n";
}

echo "\n=== Flex wrap ===\n";
foreach (['no-wrap', 'wrap', 'wrap-reverse'] as $wrap) {
    $box = new Box(['flexWrap' => $wrap]);
    echo "flexWrap '$wrap': " . $box->flexWrap . "\n";
}

echo "\n=== Position ===\n";
foreach (['relative', 'absolute'] as $pos) {
    $box = new Box(['position' => $pos]);
    echo "position '$pos': " . $box->position . "\n";
}

echo "\n=== Flex grow/shrink/basis ===\n";
$box = new Box(['flexGrow' => 2, 'flexShrink' => 1, 'flexBasis' => 100]);
echo "flexGrow: " . $box->flexGrow . "\n";
echo "flexShrink: " . $box->flexShrink . "\n";
echo "flexBasis: " . $box->flexBasis . "\n";

echo "\n=== Dimensions ===\n";
$box = new Box([
    'width' => 100,
    'height' => 50,
    'minWidth' => 10,
    'minHeight' => 5,
    'maxWidth' => 200,
    'maxHeight' => 100,
]);
echo "width: " . $box->width . "\n";
echo "height: " . $box->height . "\n";
echo "minWidth: " . $box->minWidth . "\n";
echo "minHeight: " . $box->minHeight . "\n";
echo "maxWidth: " . $box->maxWidth . "\n";
echo "maxHeight: " . $box->maxHeight . "\n";

echo "\n=== Percentage dimensions ===\n";
$box = new Box([
    'width' => '50%',
    'height' => '100%',
]);
echo "width: " . $box->width . "\n";
echo "height: " . $box->height . "\n";

echo "\n=== Padding ===\n";
$box = new Box([
    'padding' => 5,
    'paddingTop' => 1,
    'paddingBottom' => 2,
    'paddingLeft' => 3,
    'paddingRight' => 4,
    'paddingX' => 10,
    'paddingY' => 20,
]);
echo "padding: " . $box->padding . "\n";
echo "paddingTop: " . $box->paddingTop . "\n";
echo "paddingX: " . $box->paddingX . "\n";
echo "paddingY: " . $box->paddingY . "\n";

echo "\n=== Margin ===\n";
$box = new Box([
    'margin' => 5,
    'marginTop' => 1,
    'marginBottom' => 2,
    'marginLeft' => 3,
    'marginRight' => 4,
    'marginX' => 10,
    'marginY' => 20,
]);
echo "margin: " . $box->margin . "\n";
echo "marginTop: " . $box->marginTop . "\n";
echo "marginX: " . $box->marginX . "\n";
echo "marginY: " . $box->marginY . "\n";

echo "\n=== Gap ===\n";
$box = new Box(['gap' => 5]);
echo "gap: " . $box->gap . "\n";

echo "\n=== Aspect ratio ===\n";
$box = new Box(['aspectRatio' => 1.5]);
echo "aspectRatio: " . $box->aspectRatio . "\n";

echo "\n=== Direction (RTL/LTR) ===\n";
$box = new Box(['direction' => 'ltr']);
echo "direction ltr: " . $box->direction . "\n";
$box = new Box(['direction' => 'rtl']);
echo "direction rtl: " . $box->direction . "\n";

echo "\n=== Overflow ===\n";
foreach (['visible', 'hidden', 'scroll'] as $overflow) {
    $box = new Box(['overflow' => $overflow]);
    echo "overflow '$overflow': " . $box->overflow . "\n";
}

echo "\n=== Complex layout test ===\n";
$renderer = tui_test_create(80, 24);
$tree = new Box([
    'flexDirection' => 'column',
    'width' => '100%',
    'height' => '100%',
    'children' => [
        new Box([
            'id' => 'header',
            'height' => 3,
            'alignItems' => 'center',
            'justifyContent' => 'center',
            'children' => [new Text("Header")]
        ]),
        new Box([
            'id' => 'main',
            'flexDirection' => 'row',
            'flexGrow' => 1,
            'children' => [
                new Box([
                    'id' => 'sidebar',
                    'width' => 20,
                    'children' => [new Text("Sidebar")]
                ]),
                new Box([
                    'id' => 'content',
                    'flexGrow' => 1,
                    'children' => [new Text("Content")]
                ]),
            ]
        ]),
        new Box([
            'id' => 'footer',
            'height' => 1,
            'children' => [new Text("Footer")]
        ]),
    ]
]);
tui_test_render($renderer, $tree);

$header = tui_test_get_by_id($renderer, 'header');
$sidebar = tui_test_get_by_id($renderer, 'sidebar');
$content = tui_test_get_by_id($renderer, 'content');
$footer = tui_test_get_by_id($renderer, 'footer');

echo "Layout rendered with header, sidebar, content, footer\n";
echo "Header found: " . ($header ? 'yes' : 'no') . "\n";
echo "Sidebar found: " . ($sidebar ? 'yes' : 'no') . "\n";
echo "Content found: " . ($content ? 'yes' : 'no') . "\n";
echo "Footer found: " . ($footer ? 'yes' : 'no') . "\n";

tui_test_destroy($renderer);

echo "\nDone!\n";
?>
--EXPECTF--
=== Flex direction ===
flexDirection 'column': column
flexDirection 'row': row
flexDirection 'column-reverse': column-reverse
flexDirection 'row-reverse': row-reverse

=== Align items ===
alignItems 'flex-start': flex-start
alignItems 'center': center
alignItems 'flex-end': flex-end
alignItems 'stretch': stretch
alignItems 'baseline': baseline

=== Justify content ===
justifyContent 'flex-start': flex-start
justifyContent 'center': center
justifyContent 'flex-end': flex-end
justifyContent 'space-between': space-between
justifyContent 'space-around': space-around
justifyContent 'space-evenly': space-evenly

=== Align self ===
alignSelf 'auto': auto
alignSelf 'flex-start': flex-start
alignSelf 'center': center
alignSelf 'flex-end': flex-end
alignSelf 'stretch': stretch
alignSelf 'baseline': baseline

=== Flex wrap ===
flexWrap 'no-wrap': no-wrap
flexWrap 'wrap': wrap
flexWrap 'wrap-reverse': wrap-reverse

=== Position ===
position 'relative': relative
position 'absolute': absolute

=== Flex grow/shrink/basis ===
flexGrow: 2
flexShrink: 1
flexBasis: 100

=== Dimensions ===
width: 100
height: 50
minWidth: 10
minHeight: 5
maxWidth: 200
maxHeight: 100

=== Percentage dimensions ===
width: 50%
height: 100%

=== Padding ===
padding: 5
paddingTop: 1
paddingX: 10
paddingY: 20

=== Margin ===
margin: 5
marginTop: 1
marginX: 10
marginY: 20

=== Gap ===
gap: 5

=== Aspect ratio ===
aspectRatio: 1.5

=== Direction (RTL/LTR) ===
direction ltr: ltr
direction rtl: rtl

=== Overflow ===
overflow 'visible': visible
overflow 'hidden': hidden
overflow 'scroll': scroll

=== Complex layout test ===
Layout rendered with header, sidebar, content, footer
Header found: yes
Sidebar found: yes
Content found: yes
Footer found: yes

Done!
