/*
  +----------------------------------------------------------------------+
  | ext-tui: Terminal UI extension for PHP                              |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"

ZEND_DECLARE_MODULE_GLOBALS(tui)

/* Resource type IDs (global, declared extern in tui_internal.h) */
int le_tui_canvas;
int le_tui_table;
int le_tui_sprite;
int le_tui_buffer;
int le_tui_test_renderer;
int le_tui_history;
int le_tui_virtual_list;
int le_tui_scroll_animation;
int le_tui_image;

/* Resource destructors */
static void tui_canvas_dtor(zend_resource *res)
{
    tui_canvas *canvas = (tui_canvas *)res->ptr;
    if (canvas) {
        tui_canvas_free(canvas);
    }
}

static void tui_table_dtor(zend_resource *res)
{
    tui_table *table = (tui_table *)res->ptr;
    if (table) {
        tui_table_free(table);
    }
}

static void tui_sprite_dtor(zend_resource *res)
{
    tui_sprite *sprite = (tui_sprite *)res->ptr;
    if (sprite) {
        tui_sprite_free(sprite);
    }
}

static void tui_buffer_dtor(zend_resource *res)
{
    tui_buffer *buffer = (tui_buffer *)res->ptr;
    if (buffer) {
        tui_buffer_destroy(buffer);
    }
}

static void tui_test_renderer_dtor(zend_resource *res)
{
    tui_test_renderer *renderer = (tui_test_renderer *)res->ptr;
    if (renderer) {
        tui_test_renderer_destroy(renderer);
    }
}

static void tui_history_dtor(zend_resource *res)
{
    tui_input_history *history = (tui_input_history *)res->ptr;
    if (history) {
        tui_history_destroy(history);
    }
}

/* Get shared Yoga configuration - used by node.c */
YGConfigRef tui_get_yoga_config(void)
{
    return TUI_G(yoga_config);
}

/* Class entries */
zend_class_entry *tui_box_ce;
zend_class_entry *tui_text_ce;
zend_class_entry *tui_instance_ce;
zend_class_entry *tui_key_ce;
zend_class_entry *tui_focus_event_ce;
zend_class_entry *tui_focus_ce;
zend_class_entry *tui_focus_manager_ce;
zend_class_entry *tui_stdin_context_ce;
zend_class_entry *tui_stdout_context_ce;
zend_class_entry *tui_stderr_context_ce;
zend_class_entry *tui_newline_ce;
zend_class_entry *tui_spacer_ce;
zend_class_entry *tui_transform_ce;
zend_class_entry *tui_static_ce;
zend_class_entry *tui_color_ce;

/* Object handlers (global, declared extern in tui_internal.h) */
zend_object_handlers tui_instance_handlers;
zend_object_handlers tui_focus_handlers;
zend_object_handlers tui_focus_manager_handlers;

/* Note: Object structures (tui_instance_object, tui_focus_object, etc.)
 * and their helper macros are defined in tui_internal.h */

zend_object *tui_instance_create_object(zend_class_entry *ce)
{
    tui_instance_object *intern = zend_object_alloc(sizeof(tui_instance_object), ce);

    intern->app = NULL;

    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);

    intern->std.handlers = &tui_instance_handlers;

    return &intern->std;
}

static void tui_instance_free_object(zend_object *obj)
{
    tui_instance_object *intern = tui_instance_from_obj(obj);

    /* Clean up the app if it still exists */
    if (intern->app) {
        tui_app_stop(intern->app);
        tui_app_destroy(intern->app);
        intern->app = NULL;
    }

    zend_object_std_dtor(&intern->std);
}

/* ------------------------------------------------------------------
 * TuiFocus object creation/free (structure in tui_internal.h)
 * ------------------------------------------------------------------ */
zend_object *tui_focus_create_object(zend_class_entry *ce)
{
    tui_focus_object *intern = zend_object_alloc(sizeof(tui_focus_object), ce);

    intern->app = NULL;

    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);

    intern->std.handlers = &tui_focus_handlers;

    return &intern->std;
}

static void tui_focus_free_object(zend_object *obj)
{
    tui_focus_object *intern = tui_focus_from_obj(obj);
    intern->app = NULL;
    zend_object_std_dtor(&intern->std);
}

/* ------------------------------------------------------------------
 * TuiFocusManager object creation/free (structure in tui_internal.h)
 * ------------------------------------------------------------------ */
zend_object *tui_focus_manager_create_object(zend_class_entry *ce)
{
    tui_focus_manager_object *intern = zend_object_alloc(sizeof(tui_focus_manager_object), ce);

    intern->app = NULL;

    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);

    intern->std.handlers = &tui_focus_manager_handlers;

    return &intern->std;
}

static void tui_focus_manager_free_object(zend_object *obj)
{
    tui_focus_manager_object *intern = tui_focus_manager_from_obj(obj);
    intern->app = NULL;
    zend_object_std_dtor(&intern->std);
}

/* ------------------------------------------------------------------
 * Named Colors lookup table (594 vibrancy palette colors)
 * Generated from vibrancy palette: 18 families × 11 shades × 3 vibrancies
 * ------------------------------------------------------------------ */
static const struct {
    const char *name;
    uint8_t r, g, b;
} named_colors[] = {
    {"snow", 247, 247, 247},
    {"white", 247, 247, 247},
    {"bright white", 247, 247, 247},
    {"bone", 240, 240, 240},
    {"eggshell", 240, 240, 240},
    {"pearl", 240, 240, 240},
    {"platinum", 219, 219, 219},
    {"silver", 219, 219, 219},
    {"chrome", 219, 219, 219},
    {"fog", 189, 189, 189},
    {"ash", 189, 189, 189},
    {"aluminum", 189, 189, 189},
    {"pewter", 153, 153, 153},
    {"gray", 153, 153, 153},
    {"nickel", 153, 153, 153},
    {"stone", 122, 122, 122},
    {"iron", 122, 122, 122},
    {"titanium", 122, 122, 122},
    {"boulder", 97, 97, 97},
    {"concrete", 97, 97, 97},
    {"graphite", 97, 97, 97},
    {"dim gray", 71, 71, 71},
    {"charcoal", 71, 71, 71},
    {"gunmetal", 71, 71, 71},
    {"coal", 46, 46, 46},
    {"onyx", 46, 46, 46},
    {"jet", 46, 46, 46},
    {"obsidian", 26, 26, 26},
    {"soot", 26, 26, 26},
    {"raven", 26, 26, 26},
    {"tar", 13, 13, 13},
    {"pitch", 13, 13, 13},
    {"black", 13, 13, 13},
    {"snow rose", 249, 246, 246},
    {"rose white", 249, 245, 245},
    {"pink mist", 250, 245, 245},
    {"pale blush", 242, 237, 237},
    {"rose fog", 243, 236, 236},
    {"petal pink", 245, 234, 234},
    {"dusty rose", 228, 211, 211},
    {"salmon pink", 232, 206, 206},
    {"coral pink", 237, 201, 201},
    {"old rose", 205, 172, 172},
    {"salmon", 219, 159, 159},
    {"coral", 235, 142, 142},
    {"indian red", 183, 123, 123},
    {"watermelon", 209, 97, 97},
    {"tomato", 240, 66, 66},
    {"brick", 165, 80, 80},
    {"crimson", 202, 43, 43},
    {"red", 245, 0, 0},
    {"redwood", 131, 63, 63},
    {"firebrick", 160, 34, 34},
    {"scarlet", 194, 0, 0},
    {"barn red", 93, 50, 50},
    {"carmine", 111, 32, 32},
    {"ruby", 132, 11, 11},
    {"wine red", 60, 32, 32},
    {"blood red", 71, 21, 21},
    {"cherry red", 85, 7, 7},
    {"dark wine", 31, 20, 20},
    {"oxblood", 35, 16, 16},
    {"garnet", 41, 10, 10},
    {"black red", 15, 10, 10},
    {"bordeaux", 18, 8, 8},
    {"black cherry", 20, 5, 5},
    {"seashell", 249, 247, 246},
    {"linen", 249, 247, 245},
    {"white peach", 250, 247, 245},
    {"bisque", 242, 240, 237},
    {"antique white", 243, 240, 236},
    {"papaya whip", 245, 240, 234},
    {"navajo", 228, 219, 211},
    {"peach", 232, 219, 206},
    {"apricot", 237, 219, 201},
    {"fawn", 205, 189, 172},
    {"sandy brown", 219, 189, 159},
    {"cantaloupe", 235, 189, 142},
    {"camel", 183, 153, 123},
    {"mango", 209, 153, 97},
    {"tangerine", 240, 153, 66},
    {"clay", 165, 122, 80},
    {"carrot", 202, 122, 43},
    {"orange", 245, 122, 0},
    {"copper", 131, 97, 63},
    {"burnt orange", 160, 97, 34},
    {"pumpkin", 194, 97, 0},
    {"rust", 93, 71, 50},
    {"sienna", 111, 71, 32},
    {"persimmon", 132, 71, 11},
    {"umber", 60, 46, 32},
    {"cinnamon", 71, 46, 21},
    {"ginger", 85, 46, 7},
    {"sepia", 31, 26, 20},
    {"mahogany", 35, 26, 16},
    {"dark rust", 41, 26, 10},
    {"brown", 15, 13, 10},
    {"cocoa", 18, 13, 8},
    {"espresso", 20, 13, 5},
    {"cream", 249, 248, 246},
    {"cornsilk", 249, 248, 245},
    {"light cream", 250, 249, 245},
    {"champagne", 242, 241, 237},
    {"vanilla", 243, 242, 236},
    {"butter", 245, 242, 234},
    {"flax", 228, 224, 211},
    {"straw", 232, 226, 206},
    {"blonde", 237, 228, 201},
    {"khaki", 204, 196, 173},
    {"maize", 217, 203, 160},
    {"golden yellow", 233, 211, 145},
    {"sand", 182, 167, 124},
    {"honey mustard", 207, 180, 99},
    {"gold", 235, 194, 71},
    {"tan", 163, 143, 82},
    {"honey", 198, 160, 47},
    {"amber", 239, 181, 6},
    {"caramel", 129, 113, 65},
    {"ochre", 157, 127, 37},
    {"goldenrod", 189, 143, 5},
    {"bronze", 92, 81, 51},
    {"brass", 109, 90, 34},
    {"old gold", 129, 100, 14},
    {"walnut", 59, 52, 33},
    {"toffee", 70, 58, 22},
    {"brown sugar", 83, 64, 9},
    {"amber umber", 31, 28, 20},
    {"dark sepia", 35, 30, 16},
    {"molasses", 40, 33, 11},
    {"bistre", 15, 14, 10},
    {"amber cocoa", 17, 15, 8},
    {"dark chocolate", 20, 16, 5},
    {"ivory", 249, 248, 246},
    {"light ivory", 249, 249, 245},
    {"light yellow", 250, 250, 245},
    {"pale cream", 242, 242, 237},
    {"chiffon", 243, 243, 236},
    {"buttermilk", 245, 244, 234},
    {"pale flax", 228, 226, 211},
    {"pale blonde", 232, 230, 206},
    {"banana", 237, 234, 201},
    {"pale khaki", 204, 202, 173},
    {"pale maize", 217, 213, 160},
    {"lemon", 233, 225, 145},
    {"pale sand", 182, 177, 124},
    {"sunshine", 207, 198, 99},
    {"canary", 235, 222, 71},
    {"buff", 163, 156, 82},
    {"golden", 198, 185, 47},
    {"yellow", 239, 219, 6},
    {"olive", 129, 124, 65},
    {"mustard", 157, 147, 37},
    {"bright gold", 189, 174, 5},
    {"drab", 92, 88, 51},
    {"olive bronze", 109, 103, 34},
    {"dark gold", 129, 119, 14},
    {"army green", 59, 57, 33},
    {"yellow olive", 70, 66, 22},
    {"dark ochre", 83, 77, 9},
    {"olive drab", 31, 30, 20},
    {"yellow umber", 35, 33, 16},
    {"dark brown", 40, 38, 11},
    {"dark bistre", 15, 15, 10},
    {"coffee", 17, 17, 8},
    {"black brown", 20, 19, 5},
    {"honeydew", 248, 249, 246},
    {"mint cream", 248, 249, 245},
    {"snow pea", 248, 250, 245},
    {"pale green", 240, 242, 237},
    {"tea green", 240, 243, 236},
    {"key lime", 241, 245, 234},
    {"lime celadon", 221, 228, 211},
    {"pistachio", 221, 232, 206},
    {"sprout", 222, 237, 201},
    {"sage green", 191, 203, 175},
    {"pear", 193, 214, 163},
    {"celery", 195, 228, 149},
    {"moss green", 157, 179, 127},
    {"kiwi", 161, 201, 105},
    {"chartreuse", 165, 227, 79},
    {"olive green", 128, 159, 86},
    {"apple green", 134, 190, 55},
    {"lime", 140, 226, 18},
    {"fern green", 102, 126, 68},
    {"grass green", 106, 150, 43},
    {"lawn green", 111, 179, 15},
    {"army olive", 74, 89, 53},
    {"avocado", 77, 105, 38},
    {"pickle", 80, 123, 20},
    {"hunter green", 48, 58, 34},
    {"forest lime", 49, 67, 24},
    {"jungle green", 51, 79, 13},
    {"pine green", 26, 30, 21},
    {"evergreen", 27, 34, 17},
    {"basil", 28, 39, 12},
    {"swamp", 13, 15, 10},
    {"midnight moss", 13, 17, 9},
    {"dark jungle", 14, 19, 6},
    {"pale honeydew", 246, 249, 247},
    {"mint", 245, 249, 246},
    {"tea", 245, 250, 246},
    {"pale mint", 237, 242, 238},
    {"seafoam", 236, 243, 237},
    {"green celadon", 234, 245, 236},
    {"sage", 211, 228, 214},
    {"light pistachio", 206, 232, 211},
    {"cucumber", 201, 237, 207},
    {"artichoke", 177, 201, 181},
    {"fern", 166, 211, 174},
    {"grass", 154, 224, 165},
    {"moss", 130, 176, 138},
    {"kelly", 111, 195, 125},
    {"clover", 88, 218, 110},
    {"green olive", 90, 155, 101},
    {"forest", 63, 182, 83},
    {"green", 31, 214, 61},
    {"hunter", 71, 122, 80},
    {"shamrock", 50, 144, 65},
    {"green emerald", 24, 170, 48},
    {"dark army", 55, 87, 61},
    {"pine", 42, 101, 52},
    {"jungle", 26, 117, 41},
    {"brunswick", 36, 56, 39},
    {"bottle green", 27, 65, 33},
    {"racing green", 17, 75, 26},
    {"green evergreen", 21, 30, 23},
    {"ivy", 18, 33, 21},
    {"malachite", 14, 37, 18},
    {"black forest", 11, 15, 11},
    {"midnight green", 9, 16, 10},
    {"deep forest", 7, 18, 9},
    {"aqua mist", 246, 249, 248},
    {"emerald seafoam", 245, 249, 248},
    {"aquamarine", 245, 250, 248},
    {"pale aqua", 237, 242, 240},
    {"light mint", 236, 243, 241},
    {"peppermint", 234, 245, 241},
    {"celadon", 211, 228, 222},
    {"spearmint", 206, 232, 224},
    {"foam", 201, 237, 225},
    {"dusty sage", 177, 201, 193},
    {"caribbean", 166, 211, 196},
    {"spring green", 154, 224, 200},
    {"patina", 130, 176, 161},
    {"jade", 111, 195, 167},
    {"bright malachite", 88, 218, 175},
    {"eucalyptus", 90, 155, 133},
    {"sea green", 63, 182, 142},
    {"emerald", 31, 214, 153},
    {"viridian", 71, 122, 105},
    {"amazon", 50, 144, 113},
    {"bright shamrock", 24, 170, 121},
    {"dark pine", 55, 87, 77},
    {"bottle", 42, 101, 81},
    {"racing", 26, 117, 87},
    {"dark brunswick", 36, 56, 49},
    {"myrtle", 27, 65, 52},
    {"deep jungle", 17, 75, 56},
    {"emerald evergreen", 21, 30, 27},
    {"deep sea", 18, 33, 28},
    {"deep emerald", 14, 37, 29},
    {"abyss green", 11, 15, 13},
    {"midnight emerald", 9, 16, 14},
    {"black emerald", 7, 18, 15},
    {"pale cyan", 246, 249, 248},
    {"light foam", 245, 249, 249},
    {"ice blue", 245, 250, 250},
    {"mist", 237, 242, 242},
    {"glacier", 236, 243, 243},
    {"bubble", 234, 245, 245},
    {"opal", 211, 228, 227},
    {"teal seafoam", 206, 232, 231},
    {"spray", 201, 237, 236},
    {"dusty teal", 177, 200, 199},
    {"turquoise", 168, 210, 208},
    {"bright aquamarine", 156, 221, 219},
    {"cadet blue", 132, 174, 173},
    {"verdigris", 114, 192, 190},
    {"bright cyan", 92, 214, 210},
    {"slate teal", 92, 152, 150},
    {"ming", 67, 178, 174},
    {"teal", 37, 208, 202},
    {"steel teal", 73, 121, 119},
    {"lagoon", 53, 141, 138},
    {"persian green", 29, 165, 160},
    {"storm teal", 57, 86, 85},
    {"peacock", 44, 99, 97},
    {"deep teal", 29, 114, 111},
    {"petrol", 36, 55, 55},
    {"marine teal", 28, 64, 62},
    {"ocean teal", 19, 73, 71},
    {"dark spruce", 22, 29, 29},
    {"spruce", 19, 32, 32},
    {"abyss teal", 15, 36, 35},
    {"midnight teal", 11, 15, 14},
    {"inkwell", 9, 16, 16},
    {"black teal", 7, 18, 18},
    {"bubbles", 246, 248, 249},
    {"frost", 245, 249, 249},
    {"ice", 245, 249, 250},
    {"pale mist", 237, 241, 242},
    {"crystal", 236, 242, 243},
    {"diamond", 234, 243, 245},
    {"powder blue", 211, 225, 228},
    {"splash", 206, 228, 232},
    {"sky blue", 201, 231, 237},
    {"light cyan", 174, 198, 203},
    {"robin egg", 162, 207, 216},
    {"electric turquoise", 147, 217, 230},
    {"cadet", 126, 171, 180},
    {"pacific", 102, 187, 204},
    {"electric blue", 75, 205, 231},
    {"steel cyan", 84, 148, 161},
    {"bright lagoon", 51, 170, 194},
    {"cyan", 12, 196, 233},
    {"slate cyan", 66, 117, 127},
    {"bondi", 40, 135, 154},
    {"cerulean", 10, 155, 184},
    {"storm cyan", 52, 84, 91},
    {"ocean", 36, 95, 107},
    {"deep cyan", 17, 108, 126},
    {"petrol blue", 34, 54, 58},
    {"marine", 23, 61, 69},
    {"bright peacock", 11, 69, 81},
    {"midnight cyan", 21, 29, 30},
    {"abyss cyan", 17, 31, 34},
    {"deep ocean", 12, 35, 39},
    {"void cyan", 10, 14, 15},
    {"ink cyan", 8, 16, 17},
    {"black cyan", 6, 17, 20},
    {"alice blue", 246, 248, 249},
    {"whisper", 245, 248, 249},
    {"cloud", 245, 248, 250},
    {"sky pale", 237, 240, 242},
    {"powder", 236, 241, 243},
    {"pale ice", 234, 241, 245},
    {"dusty sky", 211, 222, 228},
    {"baby blue", 206, 224, 232},
    {"breeze", 201, 225, 237},
    {"faded sky", 175, 193, 203},
    {"maya blue", 163, 197, 214},
    {"sky", 149, 202, 228},
    {"slate blue", 127, 162, 179},
    {"carolina", 105, 169, 201},
    {"picton", 79, 178, 227},
    {"steel blue", 86, 135, 159},
    {"azure", 55, 145, 190},
    {"capri", 18, 157, 226},
    {"shadow blue", 68, 107, 126},
    {"sky cerulean", 43, 115, 150},
    {"dodger", 15, 124, 179},
    {"storm blue", 53, 77, 89},
    {"lapis", 38, 83, 105},
    {"denim", 20, 89, 123},
    {"cobalt", 34, 50, 58},
    {"ocean blue", 24, 53, 67},
    {"marine blue", 13, 57, 79},
    {"midnight blue", 21, 27, 30},
    {"navy", 17, 28, 34},
    {"ink blue", 12, 30, 39},
    {"void blue", 10, 14, 15},
    {"abyss blue", 9, 14, 17},
    {"black sky", 6, 15, 19},
    {"pale alice", 246, 247, 249},
    {"ghost white", 245, 247, 249},
    {"blue ice", 245, 246, 250},
    {"pale blue", 237, 239, 242},
    {"light powder", 236, 238, 243},
    {"baby", 234, 238, 245},
    {"periwinkle", 211, 216, 228},
    {"pale sky", 206, 215, 232},
    {"lavender blue", 201, 213, 237},
    {"dusty blue", 172, 183, 205},
    {"cornflower", 159, 179, 219},
    {"bright maya", 142, 173, 235},
    {"steel", 123, 143, 183},
    {"bright azure", 97, 134, 209},
    {"bright dodger", 66, 124, 240},
    {"faded denim", 80, 108, 165},
    {"royal", 43, 96, 202},
    {"blue", 0, 82, 245},
    {"dark cobalt", 63, 86, 131},
    {"blue cerulean", 34, 76, 160},
    {"sapphire", 0, 65, 194},
    {"dark lapis", 50, 64, 93},
    {"egyptian", 32, 58, 111},
    {"ultramarine", 11, 51, 132},
    {"prussian", 32, 41, 60},
    {"dark navy", 21, 37, 71},
    {"bright marine", 7, 33, 85},
    {"oxford", 20, 24, 31},
    {"midnight", 16, 22, 35},
    {"ink", 10, 20, 41},
    {"void", 10, 12, 15},
    {"abyss", 8, 11, 18},
    {"black blue", 5, 10, 20},
    {"lavender", 246, 246, 249},
    {"pale periwinkle", 246, 245, 249},
    {"thistle", 245, 245, 250},
    {"pale lilac", 238, 237, 242},
    {"wisteria", 237, 236, 243},
    {"light iris", 235, 234, 245},
    {"mauve", 212, 211, 228},
    {"heather", 209, 206, 232},
    {"lilac", 204, 201, 237},
    {"dusty violet", 180, 178, 199},
    {"bluebell", 172, 169, 208},
    {"light orchid", 164, 159, 219},
    {"slate violet", 137, 133, 173},
    {"iris", 122, 116, 190},
    {"han blue", 106, 97, 209},
    {"grape", 99, 95, 150},
    {"majorelle", 79, 71, 174},
    {"indigo", 56, 43, 202},
    {"plum", 79, 75, 119},
    {"bright violet", 63, 56, 138},
    {"electric indigo", 44, 34, 160},
    {"byzantium", 60, 58, 85},
    {"indigo purple", 50, 46, 97},
    {"bright purple", 39, 32, 111},
    {"eggplant", 39, 37, 55},
    {"blackcurrant", 32, 29, 62},
    {"deep violet", 25, 21, 71},
    {"raisin", 23, 22, 29},
    {"midnight violet", 20, 19, 32},
    {"abyss violet", 17, 16, 35},
    {"void violet", 11, 11, 14},
    {"black indigo", 10, 10, 16},
    {"deep space", 9, 8, 18},
    {"pale lavender", 247, 246, 249},
    {"light mauve", 247, 245, 249},
    {"pale thistle", 247, 245, 250},
    {"dusty lilac", 240, 237, 242},
    {"light wisteria", 240, 236, 243},
    {"bright lilac", 240, 234, 245},
    {"faded heather", 219, 211, 228},
    {"orchid pink", 219, 206, 232},
    {"light periwinkle", 219, 201, 237},
    {"dusty orchid", 189, 176, 202},
    {"amethyst", 189, 165, 213},
    {"heliotrope", 189, 152, 226},
    {"dark plum", 153, 129, 177},
    {"bright grape", 153, 108, 198},
    {"bright orchid", 153, 84, 222},
    {"dark byzantium", 122, 88, 157},
    {"violet purple", 122, 59, 186},
    {"violet", 122, 24, 220},
    {"dark eggplant", 97, 70, 124},
    {"fuchsia violet", 97, 47, 147},
    {"electric violet", 97, 19, 174},
    {"dark raisin", 71, 54, 88},
    {"tyrian", 71, 40, 103},
    {"neon violet", 71, 23, 120},
    {"blackberry", 46, 35, 57},
    {"mulberry", 46, 26, 66},
    {"deep purple", 46, 15, 77},
    {"midnight purple", 25, 21, 30},
    {"wine violet", 25, 18, 33},
    {"abyss purple", 25, 13, 38},
    {"void purple", 13, 11, 15},
    {"black violet", 13, 9, 17},
    {"deep void", 13, 7, 19},
    {"faded thistle", 248, 246, 249},
    {"purple lavender", 249, 245, 249},
    {"light lilac", 249, 245, 250},
    {"pale plum", 241, 237, 242},
    {"purple mauve", 242, 236, 243},
    {"pale wisteria", 243, 234, 245},
    {"dusty heather", 225, 211, 228},
    {"purple orchid", 228, 206, 232},
    {"bright plum", 231, 201, 237},
    {"dusty purple", 197, 177, 201},
    {"light amethyst", 204, 166, 211},
    {"bright fuchsia", 212, 154, 224},
    {"faded grape", 168, 130, 176},
    {"royal purple", 181, 111, 195},
    {"purple flower", 196, 88, 218},
    {"muted byzantium", 144, 90, 155},
    {"purple iris", 162, 63, 182},
    {"purple", 184, 31, 214},
    {"muted eggplant", 114, 71, 122},
    {"purple violet", 128, 50, 144},
    {"magenta", 145, 24, 170},
    {"dark purple raisin", 82, 55, 87},
    {"purple tyrian", 91, 42, 101},
    {"electric purple", 102, 26, 117},
    {"purple blackberry", 53, 36, 56},
    {"purple mulberry", 59, 27, 65},
    {"bright deep purple", 65, 17, 75},
    {"dark purple midnight", 28, 21, 30},
    {"purple wine", 30, 18, 33},
    {"purple abyss", 33, 14, 37},
    {"purple void", 14, 11, 15},
    {"black purple", 15, 9, 16},
    {"purple deep space", 17, 7, 18},
    {"blush", 249, 246, 248},
    {"fuchsia pink", 249, 245, 248},
    {"cotton candy", 250, 245, 249},
    {"pale pink", 242, 237, 241},
    {"carnation", 243, 236, 242},
    {"fuchsia petal", 245, 234, 242},
    {"dusty pink", 228, 211, 224},
    {"tulip pink", 232, 206, 226},
    {"peony", 237, 201, 228},
    {"fuchsia mauve", 203, 174, 196},
    {"fuchsia orchid", 216, 162, 202},
    {"hot pink", 230, 147, 210},
    {"fuchsia rose", 180, 126, 167},
    {"cerise", 204, 102, 178},
    {"bright magenta", 231, 75, 192},
    {"raspberry", 161, 84, 142},
    {"hot fuchsia", 194, 51, 158},
    {"fuchsia", 233, 12, 177},
    {"amaranth", 127, 66, 112},
    {"barbie pink", 154, 40, 125},
    {"electric fuchsia", 184, 10, 141},
    {"fuchsia mulberry", 91, 52, 81},
    {"berry pink", 107, 36, 89},
    {"neon fuchsia", 126, 17, 99},
    {"fuchsia plum", 58, 34, 52},
    {"fuchsia wine", 69, 23, 57},
    {"deep fuchsia", 81, 11, 63},
    {"maroon pink", 30, 21, 28},
    {"burgundy pink", 34, 17, 30},
    {"fuchsia raisin", 39, 12, 32},
    {"midnight fuchsia", 15, 10, 14},
    {"fuchsia void", 17, 8, 15},
    {"black fuchsia", 20, 6, 16},
    {"pink snow", 249, 246, 247},
    {"pink blush", 249, 245, 247},
    {"lace pink", 250, 245, 246},
    {"faded pink", 242, 237, 239},
    {"pink powder", 243, 236, 238},
    {"piglet", 245, 234, 238},
    {"muted pink", 228, 211, 216},
    {"cotton pink", 232, 206, 215},
    {"ballet pink", 237, 201, 213},
    {"antique pink", 202, 176, 184},
    {"cherry blossom", 213, 165, 181},
    {"pink rose", 226, 152, 176},
    {"dusty rose pink", 177, 129, 145},
    {"pink carnation", 198, 108, 138},
    {"flamingo", 222, 84, 130},
    {"mauve pink", 157, 88, 111},
    {"bubblegum", 186, 59, 101},
    {"pink", 220, 24, 90},
    {"faded dusty rose", 124, 70, 88},
    {"pink tulip", 147, 47, 80},
    {"punch pink", 174, 19, 71},
    {"rouge", 88, 54, 66},
    {"watermelon pink", 103, 40, 61},
    {"pink coral", 120, 23, 55},
    {"berry", 57, 35, 42},
    {"raspberry pink", 66, 26, 39},
    {"cerise pink", 77, 15, 35},
    {"pink wine", 30, 21, 24},
    {"maroon", 33, 18, 23},
    {"burgundy", 38, 13, 21},
    {"midnight pink", 15, 11, 12},
    {"pink void", 17, 9, 11},
    {"black pink", 19, 7, 11},
    {"rosewater", 249, 246, 247},
    {"rose blush", 249, 245, 246},
    {"petal", 250, 245, 246},
    {"pale rose", 242, 237, 238},
    {"rose powder", 243, 236, 237},
    {"rose ballet", 245, 234, 236},
    {"faded rose", 228, 211, 214},
    {"rose carnation", 232, 206, 211},
    {"rose tulip", 237, 201, 207},
    {"rose antique", 203, 175, 179},
    {"rose peony", 214, 163, 172},
    {"salmon rose", 228, 149, 162},
    {"rose mauve", 179, 127, 136},
    {"coral rose", 201, 105, 121},
    {"punch", 227, 79, 104},
    {"faded rouge", 159, 86, 98},
    {"rose cerise", 190, 55, 77},
    {"rose", 226, 18, 53},
    {"rose raspberry", 126, 68, 78},
    {"rose ruby", 150, 43, 61},
    {"cherry", 179, 15, 42},
    {"rose berry", 89, 53, 59},
    {"wine", 105, 38, 49},
    {"rose crimson", 123, 20, 37},
    {"claret", 58, 34, 38},
    {"burgundy rose", 67, 24, 32},
    {"rose oxblood", 79, 13, 24},
    {"rose maroon", 30, 21, 22},
    {"dark bordeaux", 34, 17, 20},
    {"rose garnet", 39, 12, 17},
    {"midnight rose", 15, 10, 11},
    {"rose void", 17, 9, 10},
    {"black rose", 19, 6, 8},
    /* Sentinel */
    {NULL, 0, 0, 0}
};

/* Total named colors: 594 */

static int lookup_named_color(const char *name, tui_color *color)
{
    for (int i = 0; named_colors[i].name != NULL; i++) {
        if (strcasecmp(name, named_colors[i].name) == 0) {
            color->r = named_colors[i].r;
            color->g = named_colors[i].g;
            color->b = named_colors[i].b;
            color->is_set = 1;
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------
 * Color enum helper: Convert CSS name to PascalCase (e.g., "darkblue" -> "DarkBlue")
 * ------------------------------------------------------------------ */
static void css_name_to_pascal_case(const char *name, char *out, size_t out_size)
{
    if (!name || !out || out_size == 0) return;

    /* Known word components in CSS color names */
    static const char *words[] = {
        /* Modifiers */
        "dark", "light", "medium", "pale", "deep", "dim",
        /* Color words */
        "red", "green", "blue", "yellow", "orange", "pink", "purple", "violet",
        "cyan", "gray", "grey", "brown", "white", "black", "salmon", "coral",
        "gold", "golden", "rod", "turquoise", "aqua", "marine", "sea", "sky",
        "slate", "steel", "powder", "alice", "cadet", "dodger", "royal",
        "corn", "flower", "midnight", "navy", "indigo", "orchid", "plum",
        "thistle", "lavender", "magenta", "fuchsia", "crimson", "fire", "brick",
        "indian", "hot", "misty", "rose", "rosy", "sandy", "sienna", "saddle",
        "peru", "chocolate", "tan", "wheat", "burly", "wood", "bisque",
        "blanched", "almond", "navajo", "papaya", "whip", "peach", "puff",
        "lemon", "chiffon", "moccasin", "ivory", "beige", "linen", "old", "lace",
        "floral", "ghost", "honey", "dew", "mint", "cream", "azure", "snow",
        "shell", "smoke", "spring", "lawn", "lime", "chart", "reuse", "forest",
        "olive", "drab", "khaki", "rebecca",
        NULL
    };

    size_t j = 0;
    size_t i = 0;
    size_t name_len = strlen(name);
    size_t remaining = out_size - 1;  /* Reserve space for null terminator */

    while (name[i] && remaining > 0) {
        /* Try to match a known word at current position */
        int matched = 0;
        for (int w = 0; words[w] != NULL; w++) {
            size_t wlen = strlen(words[w]);
            if (i + wlen <= name_len && strncmp(name + i, words[w], wlen) == 0) {
                /* Check if entire word fits */
                if (wlen > remaining) {
                    /* Word doesn't fit - stop here */
                    goto done;
                }
                /* Found a word - capitalize first letter */
                out[j++] = (name[i] >= 'a' && name[i] <= 'z')
                           ? name[i] - 'a' + 'A' : name[i];
                remaining--;
                /* Copy rest of word */
                for (size_t k = 1; k < wlen; k++) {
                    out[j++] = name[i + k];
                    remaining--;
                }
                i += wlen;
                matched = 1;
                break;
            }
        }
        if (!matched) {
            /* Unknown character - just copy (capitalize if first) */
            if (i == 0 && name[i] >= 'a' && name[i] <= 'z') {
                out[j++] = name[i] - 'a' + 'A';
            } else {
                out[j++] = name[i];
            }
            remaining--;
            i++;
        }
    }
done:
    out[j] = '\0';
}

/* ------------------------------------------------------------------
 * Color enum helper: Get named_colors index from enum case (by hex value)
 * ------------------------------------------------------------------ */
static int get_color_index_from_hex(const char *hex)
{
    if (!hex || hex[0] != '#' || strlen(hex) != 7) return -1;

    unsigned int r, g, b;
    if (sscanf(hex, "#%02x%02x%02x", &r, &g, &b) != 3) return -1;

    for (int i = 0; named_colors[i].name != NULL; i++) {
        if (named_colors[i].r == r && named_colors[i].g == g && named_colors[i].b == b) {
            return i;
        }
    }
    return -1;
}

/* ------------------------------------------------------------------
 * Color enum methods
 * ------------------------------------------------------------------ */

/* Color::toRgb(): array{r: int, g: int, b: int} */
PHP_METHOD(Color, toRgb)
{
    ZEND_PARSE_PARAMETERS_NONE();

    zval *value = zend_enum_fetch_case_value(Z_OBJ_P(ZEND_THIS));
    if (!value || Z_TYPE_P(value) != IS_STRING) {
        RETURN_NULL();
    }

    int idx = get_color_index_from_hex(Z_STRVAL_P(value));
    if (idx < 0) {
        RETURN_NULL();
    }

    array_init(return_value);
    add_assoc_long(return_value, "r", named_colors[idx].r);
    add_assoc_long(return_value, "g", named_colors[idx].g);
    add_assoc_long(return_value, "b", named_colors[idx].b);
}

/* Color::toAnsi(): string - ANSI escape sequence for foreground */
PHP_METHOD(Color, toAnsi)
{
    ZEND_PARSE_PARAMETERS_NONE();

    zval *value = zend_enum_fetch_case_value(Z_OBJ_P(ZEND_THIS));
    if (!value || Z_TYPE_P(value) != IS_STRING) {
        RETURN_EMPTY_STRING();
    }

    int idx = get_color_index_from_hex(Z_STRVAL_P(value));
    if (idx < 0) {
        RETURN_EMPTY_STRING();
    }

    char ansi[32];
    int len = snprintf(ansi, sizeof(ansi), "\033[38;2;%d;%d;%dm",
             named_colors[idx].r, named_colors[idx].g, named_colors[idx].b);
    if (len < 0 || (size_t)len >= sizeof(ansi)) {
        RETURN_EMPTY_STRING();
    }
    RETURN_STRING(ansi);
}

/* Color::toAnsiBg(): string - ANSI escape sequence for background */
PHP_METHOD(Color, toAnsiBg)
{
    ZEND_PARSE_PARAMETERS_NONE();

    zval *value = zend_enum_fetch_case_value(Z_OBJ_P(ZEND_THIS));
    if (!value || Z_TYPE_P(value) != IS_STRING) {
        RETURN_EMPTY_STRING();
    }

    int idx = get_color_index_from_hex(Z_STRVAL_P(value));
    if (idx < 0) {
        RETURN_EMPTY_STRING();
    }

    char ansi[32];
    int len = snprintf(ansi, sizeof(ansi), "\033[48;2;%d;%d;%dm",
             named_colors[idx].r, named_colors[idx].g, named_colors[idx].b);
    if (len < 0 || (size_t)len >= sizeof(ansi)) {
        RETURN_EMPTY_STRING();
    }
    RETURN_STRING(ansi);
}

/* Color::fromName(string $name): ?Color - Get color case by CSS name */
PHP_METHOD(Color, fromName)
{
    zend_string *name;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    /* Lookup by CSS name (case-insensitive) */
    for (int i = 0; named_colors[i].name != NULL; i++) {
        if (strcasecmp(ZSTR_VAL(name), named_colors[i].name) == 0) {
            /* Build hex value */
            char hex[8];
            int len = snprintf(hex, sizeof(hex), "#%02x%02x%02x",
                     named_colors[i].r, named_colors[i].g, named_colors[i].b);
            if (len < 0 || (size_t)len >= sizeof(hex)) {
                break;  /* Encoding error - return NULL */
            }

            /* Get enum case by value */
            zend_string *hex_str = zend_string_init(hex, 7, 0);
            zend_object *case_obj = NULL;
            zend_result result = zend_enum_get_case_by_value(&case_obj, tui_color_ce, 0, hex_str, true);
            zend_string_release(hex_str);

            if (result == SUCCESS && case_obj) {
                RETURN_OBJ_COPY(case_obj);
            }
            break;
        }
    }

    RETURN_NULL();
}

/* Color enum arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_color_torgb, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_color_toansi, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_color_toansibg, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_color_fromname, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry tui_color_methods[] = {
    PHP_ME(Color, toRgb, arginfo_color_torgb, ZEND_ACC_PUBLIC)
    PHP_ME(Color, toAnsi, arginfo_color_toansi, ZEND_ACC_PUBLIC)
    PHP_ME(Color, toAnsiBg, arginfo_color_toansibg, ZEND_ACC_PUBLIC)
    PHP_ME(Color, fromName, arginfo_color_fromname, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

/* ------------------------------------------------------------------
 * Helper: Parse RGB color from string (#RRGGBB, named) or array [r, g, b]
 * ------------------------------------------------------------------ */
static int parse_color(zval *value, tui_color *color)
{
    if (Z_TYPE_P(value) == IS_STRING) {
        const char *str = Z_STRVAL_P(value);
        size_t len = Z_STRLEN_P(value);

        /* Try hex format first: #RRGGBB */
        if (str[0] == '#' && len == 7) {
            /* Validate all characters are hex digits before parsing */
            int valid = 1;
            for (int i = 1; i < 7; i++) {
                if (!((str[i] >= '0' && str[i] <= '9') ||
                      (str[i] >= 'a' && str[i] <= 'f') ||
                      (str[i] >= 'A' && str[i] <= 'F'))) {
                    valid = 0;
                    break;
                }
            }
            if (valid) {
                unsigned int r, g, b;
                if (sscanf(str, "#%02x%02x%02x", &r, &g, &b) == 3) {
                    color->r = (uint8_t)r;
                    color->g = (uint8_t)g;
                    color->b = (uint8_t)b;
                    color->is_set = 1;
                    return 1;
                }
            }
        }

        /* Try named color lookup */
        if (lookup_named_color(str, color)) {
            return 1;
        }
    } else if (Z_TYPE_P(value) == IS_ARRAY) {
        HashTable *ht = Z_ARRVAL_P(value);
        zval *r = zend_hash_index_find(ht, 0);
        zval *g = zend_hash_index_find(ht, 1);
        zval *b = zend_hash_index_find(ht, 2);
        if (r && g && b) {
            /* Clamp values to 0-255 range */
            zend_long rv = zval_get_long(r);
            zend_long gv = zval_get_long(g);
            zend_long bv = zval_get_long(b);
            color->r = (uint8_t)(rv < 0 ? 0 : (rv > 255 ? 255 : rv));
            color->g = (uint8_t)(gv < 0 ? 0 : (gv > 255 ? 255 : gv));
            color->b = (uint8_t)(bv < 0 ? 0 : (bv > 255 ? 255 : bv));
            color->is_set = 1;
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------
 * PHP-to-C Node Tree Conversion
 * ------------------------------------------------------------------ */
/* Convert PHP object tree to C node tree (exposed for split modules) */
tui_node* php_to_tui_node(zval *obj, int depth)
{
    if (!obj || Z_TYPE_P(obj) != IS_OBJECT) {
        return NULL;
    }

    /* Prevent stack overflow from deeply nested trees */
    if (depth > MAX_TREE_DEPTH) {
        php_error_docref(NULL, E_WARNING, "Maximum node tree depth exceeded (%d)", MAX_TREE_DEPTH);
        return NULL;
    }

    zend_class_entry *ce = Z_OBJCE_P(obj);
    tui_node *node = NULL;
    zval rv;

    if (instanceof_function(ce, tui_box_ce)) {
        /* Create box node */
        node = tui_node_create_box();
        if (!node) return NULL;

        /* Read properties and apply to Yoga node */
        zval *prop;

        /* flexDirection */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "flexDirection", sizeof("flexDirection")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            const char *dir = Z_STRVAL_P(prop);
            if (strcmp(dir, "row") == 0) {
                YGNodeStyleSetFlexDirection(node->yoga_node, YGFlexDirectionRow);
            } else if (strcmp(dir, "row-reverse") == 0) {
                YGNodeStyleSetFlexDirection(node->yoga_node, YGFlexDirectionRowReverse);
            } else if (strcmp(dir, "column-reverse") == 0) {
                YGNodeStyleSetFlexDirection(node->yoga_node, YGFlexDirectionColumnReverse);
            } else {
                YGNodeStyleSetFlexDirection(node->yoga_node, YGFlexDirectionColumn);
            }
        }

        /* alignItems */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "alignItems", sizeof("alignItems")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            const char *align = Z_STRVAL_P(prop);
            if (strcmp(align, "flex-start") == 0 || strcmp(align, "start") == 0) {
                YGNodeStyleSetAlignItems(node->yoga_node, YGAlignFlexStart);
            } else if (strcmp(align, "center") == 0) {
                YGNodeStyleSetAlignItems(node->yoga_node, YGAlignCenter);
            } else if (strcmp(align, "flex-end") == 0 || strcmp(align, "end") == 0) {
                YGNodeStyleSetAlignItems(node->yoga_node, YGAlignFlexEnd);
            } else if (strcmp(align, "stretch") == 0) {
                YGNodeStyleSetAlignItems(node->yoga_node, YGAlignStretch);
            } else if (strcmp(align, "baseline") == 0) {
                YGNodeStyleSetAlignItems(node->yoga_node, YGAlignBaseline);
            }
        }

        /* justifyContent */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "justifyContent", sizeof("justifyContent")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            const char *justify = Z_STRVAL_P(prop);
            if (strcmp(justify, "flex-start") == 0 || strcmp(justify, "start") == 0) {
                YGNodeStyleSetJustifyContent(node->yoga_node, YGJustifyFlexStart);
            } else if (strcmp(justify, "center") == 0) {
                YGNodeStyleSetJustifyContent(node->yoga_node, YGJustifyCenter);
            } else if (strcmp(justify, "flex-end") == 0 || strcmp(justify, "end") == 0) {
                YGNodeStyleSetJustifyContent(node->yoga_node, YGJustifyFlexEnd);
            } else if (strcmp(justify, "space-between") == 0) {
                YGNodeStyleSetJustifyContent(node->yoga_node, YGJustifySpaceBetween);
            } else if (strcmp(justify, "space-around") == 0) {
                YGNodeStyleSetJustifyContent(node->yoga_node, YGJustifySpaceAround);
            } else if (strcmp(justify, "space-evenly") == 0) {
                YGNodeStyleSetJustifyContent(node->yoga_node, YGJustifySpaceEvenly);
            }
        }

        /* alignSelf - allows individual child to override parent's alignItems */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "alignSelf", sizeof("alignSelf")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            const char *alignSelf = Z_STRVAL_P(prop);
            if (strcmp(alignSelf, "auto") == 0) {
                YGNodeStyleSetAlignSelf(node->yoga_node, YGAlignAuto);
            } else if (strcmp(alignSelf, "flex-start") == 0 || strcmp(alignSelf, "start") == 0) {
                YGNodeStyleSetAlignSelf(node->yoga_node, YGAlignFlexStart);
            } else if (strcmp(alignSelf, "center") == 0) {
                YGNodeStyleSetAlignSelf(node->yoga_node, YGAlignCenter);
            } else if (strcmp(alignSelf, "flex-end") == 0 || strcmp(alignSelf, "end") == 0) {
                YGNodeStyleSetAlignSelf(node->yoga_node, YGAlignFlexEnd);
            } else if (strcmp(alignSelf, "stretch") == 0) {
                YGNodeStyleSetAlignSelf(node->yoga_node, YGAlignStretch);
            } else if (strcmp(alignSelf, "baseline") == 0) {
                YGNodeStyleSetAlignSelf(node->yoga_node, YGAlignBaseline);
            }
        }

        /* flexGrow */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "flexGrow", sizeof("flexGrow")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            YGNodeStyleSetFlexGrow(node->yoga_node, (float)zval_get_double(prop));
        }

        /* flexShrink */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "flexShrink", sizeof("flexShrink")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            YGNodeStyleSetFlexShrink(node->yoga_node, (float)zval_get_double(prop));
        }

        /* flexBasis - initial size before flex distribution */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "flexBasis", sizeof("flexBasis")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            YGNodeStyleSetFlexBasis(node->yoga_node, (float)zval_get_double(prop));
        } else if (prop && Z_TYPE_P(prop) == IS_STRING) {
            const char *basis = Z_STRVAL_P(prop);
            if (strcmp(basis, "auto") == 0) {
                YGNodeStyleSetFlexBasisAuto(node->yoga_node);
            } else {
                int pct;
                if (sscanf(basis, "%d%%", &pct) == 1 && pct >= 0 && pct <= 100) {
                    YGNodeStyleSetFlexBasisPercent(node->yoga_node, pct);
                }
            }
        }

        /* width */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "width", sizeof("width")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            YGNodeStyleSetWidth(node->yoga_node, (float)zval_get_double(prop));
        } else if (prop && Z_TYPE_P(prop) == IS_STRING) {
            const char *w = Z_STRVAL_P(prop);
            if (strcmp(w, "100%") == 0) {
                YGNodeStyleSetWidthPercent(node->yoga_node, 100);
            } else {
                int pct;
                if (sscanf(w, "%d%%", &pct) == 1 && pct >= 0 && pct <= 100) {
                    YGNodeStyleSetWidthPercent(node->yoga_node, pct);
                }
            }
        }

        /* height */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "height", sizeof("height")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            YGNodeStyleSetHeight(node->yoga_node, (float)zval_get_double(prop));
        } else if (prop && Z_TYPE_P(prop) == IS_STRING) {
            const char *h = Z_STRVAL_P(prop);
            if (strcmp(h, "100%") == 0) {
                YGNodeStyleSetHeightPercent(node->yoga_node, 100);
            } else {
                int pct;
                if (sscanf(h, "%d%%", &pct) == 1 && pct >= 0 && pct <= 100) {
                    YGNodeStyleSetHeightPercent(node->yoga_node, pct);
                }
            }
        }

        /* padding - only apply non-zero values to avoid overriding YGEdgeAll */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "padding", sizeof("padding")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float p = (float)zval_get_double(prop);
            if (p > 0) {
                YGNodeStyleSetPadding(node->yoga_node, YGEdgeAll, p);
            }
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "paddingTop", sizeof("paddingTop")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float pt = (float)zval_get_double(prop);
            if (pt > 0) {
                YGNodeStyleSetPadding(node->yoga_node, YGEdgeTop, pt);
            }
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "paddingBottom", sizeof("paddingBottom")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float pb = (float)zval_get_double(prop);
            if (pb > 0) {
                YGNodeStyleSetPadding(node->yoga_node, YGEdgeBottom, pb);
            }
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "paddingLeft", sizeof("paddingLeft")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float pl = (float)zval_get_double(prop);
            if (pl > 0) {
                YGNodeStyleSetPadding(node->yoga_node, YGEdgeLeft, pl);
            }
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "paddingRight", sizeof("paddingRight")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float pr = (float)zval_get_double(prop);
            if (pr > 0) {
                YGNodeStyleSetPadding(node->yoga_node, YGEdgeRight, pr);
            }
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "paddingX", sizeof("paddingX")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float px = (float)zval_get_double(prop);
            if (px > 0) {
                YGNodeStyleSetPadding(node->yoga_node, YGEdgeLeft, px);
                YGNodeStyleSetPadding(node->yoga_node, YGEdgeRight, px);
            }
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "paddingY", sizeof("paddingY")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float py = (float)zval_get_double(prop);
            if (py > 0) {
                YGNodeStyleSetPadding(node->yoga_node, YGEdgeTop, py);
                YGNodeStyleSetPadding(node->yoga_node, YGEdgeBottom, py);
            }
        }

        /* margin - only apply non-zero values to avoid overriding YGEdgeAll */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "margin", sizeof("margin")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float m = (float)zval_get_double(prop);
            if (m > 0) {
                YGNodeStyleSetMargin(node->yoga_node, YGEdgeAll, m);
            }
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "marginTop", sizeof("marginTop")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float mt = (float)zval_get_double(prop);
            if (mt > 0) {
                YGNodeStyleSetMargin(node->yoga_node, YGEdgeTop, mt);
            }
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "marginBottom", sizeof("marginBottom")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float mb = (float)zval_get_double(prop);
            if (mb > 0) {
                YGNodeStyleSetMargin(node->yoga_node, YGEdgeBottom, mb);
            }
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "marginLeft", sizeof("marginLeft")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float ml = (float)zval_get_double(prop);
            if (ml > 0) {
                YGNodeStyleSetMargin(node->yoga_node, YGEdgeLeft, ml);
            }
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "marginRight", sizeof("marginRight")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float mr = (float)zval_get_double(prop);
            if (mr > 0) {
                YGNodeStyleSetMargin(node->yoga_node, YGEdgeRight, mr);
            }
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "marginX", sizeof("marginX")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float mx = (float)zval_get_double(prop);
            if (mx > 0) {
                YGNodeStyleSetMargin(node->yoga_node, YGEdgeLeft, mx);
                YGNodeStyleSetMargin(node->yoga_node, YGEdgeRight, mx);
            }
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "marginY", sizeof("marginY")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float my = (float)zval_get_double(prop);
            if (my > 0) {
                YGNodeStyleSetMargin(node->yoga_node, YGEdgeTop, my);
                YGNodeStyleSetMargin(node->yoga_node, YGEdgeBottom, my);
            }
        }

        /* gap - only apply non-zero values to avoid overriding YGGutterAll */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "gap", sizeof("gap")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float g = (float)zval_get_double(prop);
            if (g > 0) {
                YGNodeStyleSetGap(node->yoga_node, YGGutterAll, g);
            }
        }

        /* columnGap */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "columnGap", sizeof("columnGap")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float cg = (float)zval_get_double(prop);
            if (cg > 0) {
                YGNodeStyleSetGap(node->yoga_node, YGGutterColumn, cg);
            }
        }

        /* rowGap */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "rowGap", sizeof("rowGap")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            float rg = (float)zval_get_double(prop);
            if (rg > 0) {
                YGNodeStyleSetGap(node->yoga_node, YGGutterRow, rg);
            }
        }

        /* flexWrap */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "flexWrap", sizeof("flexWrap")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            const char *wrap = Z_STRVAL_P(prop);
            if (strcmp(wrap, "wrap") == 0) {
                YGNodeStyleSetFlexWrap(node->yoga_node, YGWrapWrap);
            } else if (strcmp(wrap, "wrap-reverse") == 0) {
                YGNodeStyleSetFlexWrap(node->yoga_node, YGWrapWrapReverse);
            } else {
                YGNodeStyleSetFlexWrap(node->yoga_node, YGWrapNoWrap);
            }
        }

        /* minWidth */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "minWidth", sizeof("minWidth")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            YGNodeStyleSetMinWidth(node->yoga_node, (float)zval_get_double(prop));
        } else if (prop && Z_TYPE_P(prop) == IS_STRING) {
            int pct;
            if (sscanf(Z_STRVAL_P(prop), "%d%%", &pct) == 1 && pct >= 0 && pct <= 100) {
                YGNodeStyleSetMinWidthPercent(node->yoga_node, pct);
            }
        }

        /* minHeight */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "minHeight", sizeof("minHeight")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            YGNodeStyleSetMinHeight(node->yoga_node, (float)zval_get_double(prop));
        } else if (prop && Z_TYPE_P(prop) == IS_STRING) {
            int pct;
            if (sscanf(Z_STRVAL_P(prop), "%d%%", &pct) == 1 && pct >= 0 && pct <= 100) {
                YGNodeStyleSetMinHeightPercent(node->yoga_node, pct);
            }
        }

        /* maxWidth */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "maxWidth", sizeof("maxWidth")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            YGNodeStyleSetMaxWidth(node->yoga_node, (float)zval_get_double(prop));
        } else if (prop && Z_TYPE_P(prop) == IS_STRING) {
            int pct;
            if (sscanf(Z_STRVAL_P(prop), "%d%%", &pct) == 1 && pct >= 0 && pct <= 100) {
                YGNodeStyleSetMaxWidthPercent(node->yoga_node, pct);
            }
        }

        /* maxHeight */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "maxHeight", sizeof("maxHeight")-1, 1, &rv);
        if (prop && (Z_TYPE_P(prop) == IS_LONG || Z_TYPE_P(prop) == IS_DOUBLE)) {
            YGNodeStyleSetMaxHeight(node->yoga_node, (float)zval_get_double(prop));
        } else if (prop && Z_TYPE_P(prop) == IS_STRING) {
            int pct;
            if (sscanf(Z_STRVAL_P(prop), "%d%%", &pct) == 1 && pct >= 0 && pct <= 100) {
                YGNodeStyleSetMaxHeightPercent(node->yoga_node, pct);
            }
        }

        /* overflow */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "overflow", sizeof("overflow")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            const char *overflow = Z_STRVAL_P(prop);
            if (strcmp(overflow, "hidden") == 0) {
                YGNodeStyleSetOverflow(node->yoga_node, YGOverflowHidden);
            } else if (strcmp(overflow, "scroll") == 0) {
                YGNodeStyleSetOverflow(node->yoga_node, YGOverflowScroll);
            } else {
                YGNodeStyleSetOverflow(node->yoga_node, YGOverflowVisible);
            }
        }

        /* display */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "display", sizeof("display")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            const char *display = Z_STRVAL_P(prop);
            if (strcmp(display, "none") == 0) {
                YGNodeStyleSetDisplay(node->yoga_node, YGDisplayNone);
            } else {
                YGNodeStyleSetDisplay(node->yoga_node, YGDisplayFlex);
            }
        }

        /* position */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "position", sizeof("position")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            const char *position = Z_STRVAL_P(prop);
            if (strcmp(position, "absolute") == 0) {
                YGNodeStyleSetPositionType(node->yoga_node, YGPositionTypeAbsolute);
            } else {
                YGNodeStyleSetPositionType(node->yoga_node, YGPositionTypeRelative);
            }
        }

        /* aspectRatio - maintain width/height proportions */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "aspectRatio", sizeof("aspectRatio")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) != IS_NULL) {
            float ar = (float)zval_get_double(prop);
            if (ar > 0) {
                YGNodeStyleSetAspectRatio(node->yoga_node, ar);
            }
        }

        /* direction - RTL/LTR layout direction */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "direction", sizeof("direction")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            const char *dir = Z_STRVAL_P(prop);
            if (strcmp(dir, "rtl") == 0) {
                YGNodeStyleSetDirection(node->yoga_node, YGDirectionRTL);
            } else if (strcmp(dir, "ltr") == 0) {
                YGNodeStyleSetDirection(node->yoga_node, YGDirectionLTR);
            }
            /* YGDirectionInherit is the default */
        }

        /* borderStyle */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "borderStyle", sizeof("borderStyle")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            const char *border = Z_STRVAL_P(prop);
            if (strcmp(border, "single") == 0) {
                node->border_style = TUI_BORDER_SINGLE;
                YGNodeStyleSetBorder(node->yoga_node, YGEdgeAll, 1);
            } else if (strcmp(border, "double") == 0) {
                node->border_style = TUI_BORDER_DOUBLE;
                YGNodeStyleSetBorder(node->yoga_node, YGEdgeAll, 1);
            } else if (strcmp(border, "round") == 0) {
                node->border_style = TUI_BORDER_ROUND;
                YGNodeStyleSetBorder(node->yoga_node, YGEdgeAll, 1);
            } else if (strcmp(border, "bold") == 0) {
                node->border_style = TUI_BORDER_BOLD;
                YGNodeStyleSetBorder(node->yoga_node, YGEdgeAll, 1);
            } else if (strcmp(border, "dashed") == 0) {
                node->border_style = TUI_BORDER_DASHED;
                YGNodeStyleSetBorder(node->yoga_node, YGEdgeAll, 1);
            } else {
                node->border_style = TUI_BORDER_NONE;
            }
        }

        /* borderColor */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "borderColor", sizeof("borderColor")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) != IS_NULL) {
            parse_color(prop, &node->border_color);
        }

        /* focusable */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "focusable", sizeof("focusable")-1, 1, &rv);
        if (prop && zend_is_true(prop)) {
            node->focusable = 1;
        }

        /* focused */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "focused", sizeof("focused")-1, 1, &rv);
        if (prop && zend_is_true(prop)) {
            node->focused = 1;
        }

        /* tabIndex - tab order (-1 = skip, 0+ = order) */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "tabIndex", sizeof("tabIndex")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_LONG) {
            node->tab_index = (int)Z_LVAL_P(prop);
        }

        /* focusGroup - group name for scoped tabbing */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "focusGroup", sizeof("focusGroup")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            if (tui_node_set_focus_group(node, Z_STRVAL_P(prop)) < 0) {
                tui_node_destroy(node);
                return NULL;
            }
        }

        /* autoFocus - focus on mount */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "autoFocus", sizeof("autoFocus")-1, 1, &rv);
        if (prop && zend_is_true(prop)) {
            node->auto_focus = 1;
        }

        /* focusTrap - trap focus within container */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "focusTrap", sizeof("focusTrap")-1, 1, &rv);
        if (prop && zend_is_true(prop)) {
            node->focus_trap = 1;
        }

        /* showCursor - show terminal cursor when focused (for text input) */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "showCursor", sizeof("showCursor")-1, 1, &rv);
        if (prop && zend_is_true(prop)) {
            node->show_cursor = 1;
        }

        /* key - for reconciliation */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "key", sizeof("key")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            node->key = strdup(Z_STRVAL_P(prop));
            if (!node->key) {
                tui_node_destroy(node);
                return NULL;
            }
        }

        /* id - for focus-by-id */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "id", sizeof("id")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            if (tui_node_set_id(node, Z_STRVAL_P(prop)) < 0) {
                tui_node_destroy(node);
                return NULL;
            }
        }

        /* Per-side border colors */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "borderTopColor", sizeof("borderTopColor")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) != IS_NULL) {
            parse_color(prop, &node->border_top_color);
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "borderRightColor", sizeof("borderRightColor")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) != IS_NULL) {
            parse_color(prop, &node->border_right_color);
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "borderBottomColor", sizeof("borderBottomColor")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) != IS_NULL) {
            parse_color(prop, &node->border_bottom_color);
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "borderLeftColor", sizeof("borderLeftColor")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) != IS_NULL) {
            parse_color(prop, &node->border_left_color);
        }

        /* Process children */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "children", sizeof("children")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_ARRAY) {
            HashTable *ht = Z_ARRVAL_P(prop);
            zval *child;
            ZEND_HASH_FOREACH_VAL(ht, child) {
                tui_node *child_node = php_to_tui_node(child, depth + 1);
                if (child_node) {
                    if (tui_node_append_child(node, child_node) < 0) {
                        /* Failed to append - destroy the orphan child to prevent leak */
                        tui_node_destroy(child_node);
                    }
                }
            } ZEND_HASH_FOREACH_END();
        }

    } else if (instanceof_function(ce, tui_text_ce)) {
        /* Get text content */
        zval *prop = zend_read_property(ce, Z_OBJ_P(obj), "content", sizeof("content")-1, 1, &rv);
        const char *text = "";
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            text = Z_STRVAL_P(prop);
        }

        /* Create text node */
        node = tui_node_create_text(text);
        if (!node) return NULL;

        /* Apply style properties */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "color", sizeof("color")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) != IS_NULL) {
            parse_color(prop, &node->style.fg);
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "backgroundColor", sizeof("backgroundColor")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) != IS_NULL) {
            parse_color(prop, &node->style.bg);
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "bold", sizeof("bold")-1, 1, &rv);
        if (prop && zend_is_true(prop)) {
            node->style.bold = 1;
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "dim", sizeof("dim")-1, 1, &rv);
        if (prop && zend_is_true(prop)) {
            node->style.dim = 1;
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "italic", sizeof("italic")-1, 1, &rv);
        if (prop && zend_is_true(prop)) {
            node->style.italic = 1;
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "underline", sizeof("underline")-1, 1, &rv);
        if (prop && zend_is_true(prop)) {
            node->style.underline = 1;
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "inverse", sizeof("inverse")-1, 1, &rv);
        if (prop && zend_is_true(prop)) {
            node->style.inverse = 1;
        }

        prop = zend_read_property(ce, Z_OBJ_P(obj), "strikethrough", sizeof("strikethrough")-1, 1, &rv);
        if (prop && zend_is_true(prop)) {
            node->style.strikethrough = 1;
        }

        /* wrap mode */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "wrap", sizeof("wrap")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            const char *wrap = Z_STRVAL_P(prop);
            if (strcmp(wrap, "word") == 0) {
                node->wrap_mode = TUI_WRAP_WORD;
            } else if (strcmp(wrap, "char") == 0) {
                node->wrap_mode = TUI_WRAP_CHAR;
            } else if (strcmp(wrap, "word-char") == 0) {
                node->wrap_mode = TUI_WRAP_WORD_CHAR;
            } else {
                node->wrap_mode = TUI_WRAP_NONE;
            }
        }

        /* key - for reconciliation */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "key", sizeof("key")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            node->key = strdup(Z_STRVAL_P(prop));
            if (!node->key) {
                tui_node_destroy(node);
                return NULL;
            }
        }

        /* id - for focus-by-id and measureElement */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "id", sizeof("id")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            if (tui_node_set_id(node, Z_STRVAL_P(prop)) < 0) {
                tui_node_destroy(node);
                return NULL;
            }
        }

        /* hyperlink - OSC 8 hyperlink URL */
        prop = zend_read_property(ce, Z_OBJ_P(obj), "hyperlink", sizeof("hyperlink")-1, 1, &rv);
        if (prop && Z_TYPE_P(prop) == IS_STRING) {
            if (tui_node_set_hyperlink(node, Z_STRVAL_P(prop), NULL) < 0) {
                tui_node_destroy(node);
                return NULL;
            }
        } else if (prop && Z_TYPE_P(prop) == IS_ARRAY) {
            /* Allow {url: 'http://...', id: 'link-1'} format */
            HashTable *ht = Z_ARRVAL_P(prop);
            zval *url_val = zend_hash_str_find(ht, "url", sizeof("url")-1);
            zval *id_val = zend_hash_str_find(ht, "id", sizeof("id")-1);
            const char *url = (url_val && Z_TYPE_P(url_val) == IS_STRING) ? Z_STRVAL_P(url_val) : NULL;
            const char *link_id = (id_val && Z_TYPE_P(id_val) == IS_STRING) ? Z_STRVAL_P(id_val) : NULL;
            if (url && tui_node_set_hyperlink(node, url, link_id) < 0) {
                tui_node_destroy(node);
                return NULL;
            }
        }
    }

    return node;
}

/* ------------------------------------------------------------------
 * TuiBox class
 * ------------------------------------------------------------------ */

/* {{{ TuiBox::__construct(array $props = []) */
PHP_METHOD(TuiBox, __construct)
{
    zval *props = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(props)
    ZEND_PARSE_PARAMETERS_END();

    /* Initialize defaults */
    zend_update_property_string(tui_box_ce, Z_OBJ_P(ZEND_THIS), "flexDirection", sizeof("flexDirection")-1, "column");
    zend_update_property_null(tui_box_ce, Z_OBJ_P(ZEND_THIS), "alignItems", sizeof("alignItems")-1);
    zend_update_property_null(tui_box_ce, Z_OBJ_P(ZEND_THIS), "justifyContent", sizeof("justifyContent")-1);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "flexGrow", sizeof("flexGrow")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "flexShrink", sizeof("flexShrink")-1, 1);
    zend_update_property_null(tui_box_ce, Z_OBJ_P(ZEND_THIS), "width", sizeof("width")-1);
    zend_update_property_null(tui_box_ce, Z_OBJ_P(ZEND_THIS), "height", sizeof("height")-1);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "padding", sizeof("padding")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "paddingTop", sizeof("paddingTop")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "paddingBottom", sizeof("paddingBottom")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "paddingLeft", sizeof("paddingLeft")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "paddingRight", sizeof("paddingRight")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "paddingX", sizeof("paddingX")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "paddingY", sizeof("paddingY")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "margin", sizeof("margin")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "marginTop", sizeof("marginTop")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "marginBottom", sizeof("marginBottom")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "marginLeft", sizeof("marginLeft")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "marginRight", sizeof("marginRight")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "marginX", sizeof("marginX")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "marginY", sizeof("marginY")-1, 0);
    zend_update_property_long(tui_box_ce, Z_OBJ_P(ZEND_THIS), "gap", sizeof("gap")-1, 0);
    zend_update_property_null(tui_box_ce, Z_OBJ_P(ZEND_THIS), "borderStyle", sizeof("borderStyle")-1);
    zend_update_property_null(tui_box_ce, Z_OBJ_P(ZEND_THIS), "borderColor", sizeof("borderColor")-1);
    zend_update_property_bool(tui_box_ce, Z_OBJ_P(ZEND_THIS), "focusable", sizeof("focusable")-1, 0);
    zend_update_property_bool(tui_box_ce, Z_OBJ_P(ZEND_THIS), "focused", sizeof("focused")-1, 0);

    /* Initialize empty children array */
    zval children;
    array_init(&children);
    zend_update_property(tui_box_ce, Z_OBJ_P(ZEND_THIS), "children", sizeof("children")-1, &children);
    zval_ptr_dtor(&children);

    /* Apply passed properties */
    if (props) {
        HashTable *ht = Z_ARRVAL_P(props);
        zend_string *key;
        zval *val;
        ZEND_HASH_FOREACH_STR_KEY_VAL(ht, key, val) {
            if (key) {
                zend_update_property(tui_box_ce, Z_OBJ_P(ZEND_THIS), ZSTR_VAL(key), ZSTR_LEN(key), val);
            }
        } ZEND_HASH_FOREACH_END();
    }
}
/* }}} */

/* {{{ TuiBox::addChild(TuiBox|TuiText $child): self */
PHP_METHOD(TuiBox, addChild)
{
    zval *child;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT(child)
    ZEND_PARSE_PARAMETERS_END();

    /* Verify child is TuiBox or TuiText */
    if (!instanceof_function(Z_OBJCE_P(child), tui_box_ce) &&
        !instanceof_function(Z_OBJCE_P(child), tui_text_ce)) {
        zend_throw_exception(zend_ce_exception, "Child must be TuiBox or TuiText", 0);
        RETURN_THROWS();
    }

    /* Get children array */
    zval rv;
    zval *children = zend_read_property(tui_box_ce, Z_OBJ_P(ZEND_THIS), "children", sizeof("children")-1, 1, &rv);

    if (Z_TYPE_P(children) != IS_ARRAY) {
        zval arr;
        array_init(&arr);
        zend_update_property(tui_box_ce, Z_OBJ_P(ZEND_THIS), "children", sizeof("children")-1, &arr);
        children = zend_read_property(tui_box_ce, Z_OBJ_P(ZEND_THIS), "children", sizeof("children")-1, 1, &rv);
        zval_ptr_dtor(&arr);
    }

    /* Add child to array */
    Z_TRY_ADDREF_P(child);
    add_next_index_zval(children, child);

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}
/* }}} */

/* TuiBox arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tuibox_construct, 0, 0, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, props, IS_ARRAY, 1, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tuibox_addchild, 0, 0, 1)
    ZEND_ARG_OBJ_INFO(0, child, TuiBox, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry tui_box_methods[] = {
    PHP_ME(TuiBox, __construct, arginfo_tuibox_construct, ZEND_ACC_PUBLIC)
    PHP_ME(TuiBox, addChild, arginfo_tuibox_addchild, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* ------------------------------------------------------------------
 * TuiText class
 * ------------------------------------------------------------------ */

/* {{{ TuiText::__construct(string $content = '', array $props = []) */
PHP_METHOD(TuiText, __construct)
{
    zend_string *content = NULL;
    zval *props = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 2)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(content)
        Z_PARAM_ARRAY(props)
    ZEND_PARSE_PARAMETERS_END();

    /* Initialize defaults */
    zend_update_property_string(tui_text_ce, Z_OBJ_P(ZEND_THIS), "content", sizeof("content")-1, content ? ZSTR_VAL(content) : "");
    zend_update_property_null(tui_text_ce, Z_OBJ_P(ZEND_THIS), "color", sizeof("color")-1);
    zend_update_property_null(tui_text_ce, Z_OBJ_P(ZEND_THIS), "backgroundColor", sizeof("backgroundColor")-1);
    zend_update_property_bool(tui_text_ce, Z_OBJ_P(ZEND_THIS), "bold", sizeof("bold")-1, 0);
    zend_update_property_bool(tui_text_ce, Z_OBJ_P(ZEND_THIS), "dim", sizeof("dim")-1, 0);
    zend_update_property_bool(tui_text_ce, Z_OBJ_P(ZEND_THIS), "italic", sizeof("italic")-1, 0);
    zend_update_property_bool(tui_text_ce, Z_OBJ_P(ZEND_THIS), "underline", sizeof("underline")-1, 0);
    zend_update_property_bool(tui_text_ce, Z_OBJ_P(ZEND_THIS), "inverse", sizeof("inverse")-1, 0);
    zend_update_property_bool(tui_text_ce, Z_OBJ_P(ZEND_THIS), "strikethrough", sizeof("strikethrough")-1, 0);
    zend_update_property_null(tui_text_ce, Z_OBJ_P(ZEND_THIS), "wrap", sizeof("wrap")-1);

    /* Apply passed properties */
    if (props) {
        HashTable *ht = Z_ARRVAL_P(props);
        zend_string *key;
        zval *val;
        ZEND_HASH_FOREACH_STR_KEY_VAL(ht, key, val) {
            if (key) {
                zend_update_property(tui_text_ce, Z_OBJ_P(ZEND_THIS), ZSTR_VAL(key), ZSTR_LEN(key), val);
            }
        } ZEND_HASH_FOREACH_END();
    }
}
/* }}} */

/* TuiText arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tuitext_construct, 0, 0, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, content, IS_STRING, 0, "\"\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, props, IS_ARRAY, 1, "[]")
ZEND_END_ARG_INFO()

static const zend_function_entry tui_text_methods[] = {
    PHP_ME(TuiText, __construct, arginfo_tuitext_construct, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* ------------------------------------------------------------------
 * TuiInstance class
 * ------------------------------------------------------------------ */

/* {{{ TuiInstance::rerender(): void */
PHP_METHOD(TuiInstance, rerender)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (obj->app) {
        /* Use rerender_callback to properly rebuild tree with Instance parameter */
        if (obj->app->rerender_callback) {
            obj->app->rerender_callback(obj->app);
        }
        tui_app_render_tree(obj->app);
    }
}
/* }}} */

/* {{{ TuiInstance::unmount(): void */
PHP_METHOD(TuiInstance, unmount)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (obj->app) {
        tui_app_stop(obj->app);
        tui_app_destroy(obj->app);
        obj->app = NULL;
    }
}
/* }}} */

/* {{{ TuiInstance::waitUntilExit(): void */
PHP_METHOD(TuiInstance, waitUntilExit)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (obj->app) {
        tui_app_wait_until_exit(obj->app);
    }
}
/* }}} */

/* {{{ TuiInstance::exit(int $code = 0): void */
PHP_METHOD(TuiInstance, exit)
{
    zend_long code = 0;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(code)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (obj->app) {
        tui_app_exit(obj->app, (int)code);
    }
}
/* }}} */

/* {{{ TuiInstance::useState(mixed $initial): array */
PHP_METHOD(TuiInstance, useState)
{
    zval *initial;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ZVAL(initial)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    tui_app *app = obj->app;
    int is_new = 0;
    int index = tui_app_get_or_create_state_slot(app, initial, &is_new);

    if (index < 0) {
        zend_throw_exception(zend_ce_exception,
            "Failed to allocate state slot: maximum states exceeded", 0);
        RETURN_THROWS();
    }

    /* Create setter closure if this is a new state slot */
    if (is_new) {
        /* Create a closure that calls our state setter
         * We use a simple approach: create an internal callable that captures
         * the app pointer and index via a wrapper object */
        zval setter_obj;
        object_init(&setter_obj);

        /* Store app pointer and index as properties we can retrieve later */
        zend_update_property_long(Z_OBJCE(setter_obj), Z_OBJ(setter_obj),
            "_app_ptr", sizeof("_app_ptr")-1, (zend_long)(uintptr_t)app);
        zend_update_property_long(Z_OBJCE(setter_obj), Z_OBJ(setter_obj),
            "_index", sizeof("_index")-1, index);

        /* For now, store a simple callable array - the actual setter logic
         * will be handled when called via a special internal function */
        ZVAL_COPY(&app->states[index].setter, &setter_obj);
        zval_ptr_dtor(&setter_obj);
    }

    /* Return [value, setter] array */
    array_init(return_value);

    /* Add current value */
    zval value_copy;
    ZVAL_COPY(&value_copy, &app->states[index].value);
    add_next_index_zval(return_value, &value_copy);

    /* Add setter (we need a callable - for now return the index for manual handling) */
    add_next_index_long(return_value, index);
}
/* }}} */

/* {{{ TuiInstance::useInput(callable $handler, array $options = []): void */
PHP_METHOD(TuiInstance, useInput)
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    zval *options = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_FUNC(fci, fcc)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(options)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    /* Check isActive option */
    zend_bool is_active = 1;
    if (options) {
        zval *val = zend_hash_str_find(Z_ARRVAL_P(options), "isActive", sizeof("isActive")-1);
        if (val) {
            is_active = zend_is_true(val);
        }
    }

    if (is_active) {
        tui_app_set_input_handler(obj->app, &fci, &fcc);
    }
}
/* }}} */

/* {{{ TuiInstance::useFocus(array $options = []): Focus */
PHP_METHOD(TuiInstance, useFocus)
{
    zval *options = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(options)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    /* Create Focus object */
    object_init_ex(return_value, tui_focus_ce);
    tui_focus_object *focus_obj = Z_TUI_FOCUS_P(return_value);
    focus_obj->app = obj->app;

    /* Check autoFocus option */
    if (options) {
        zval *auto_focus = zend_hash_str_find(Z_ARRVAL_P(options), "autoFocus", sizeof("autoFocus")-1);
        if (auto_focus && zend_is_true(auto_focus)) {
            /* Auto-focus the first focusable element */
            tui_app_focus_next(obj->app);
        }
    }

    /* Set isFocused property based on current focus state */
    zend_bool is_focused = obj->app->focused_node != NULL;
    zend_update_property_bool(tui_focus_ce, Z_OBJ_P(return_value),
        "isFocused", sizeof("isFocused")-1, is_focused);
}
/* }}} */

/* {{{ TuiInstance::useFocusManager(): FocusManager */
PHP_METHOD(TuiInstance, useFocusManager)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    /* Create FocusManager object */
    object_init_ex(return_value, tui_focus_manager_ce);
    tui_focus_manager_object *fm_obj = Z_TUI_FOCUS_MANAGER_P(return_value);
    fm_obj->app = obj->app;
}
/* }}} */

/* {{{ TuiInstance::useStdin(): StdinContext */
PHP_METHOD(TuiInstance, useStdin)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    /* Create StdinContext object */
    object_init_ex(return_value, tui_stdin_context_ce);

    /* Set properties */
    zend_update_property_bool(tui_stdin_context_ce, Z_OBJ_P(return_value),
        "isRawModeSupported", sizeof("isRawModeSupported")-1, isatty(STDIN_FILENO));
}
/* }}} */

/* {{{ TuiInstance::useStdout(): StdoutContext */
PHP_METHOD(TuiInstance, useStdout)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    /* Create StdoutContext object */
    object_init_ex(return_value, tui_stdout_context_ce);

    /* Set properties */
    zend_update_property_long(tui_stdout_context_ce, Z_OBJ_P(return_value),
        "columns", sizeof("columns")-1, obj->app->width);
    zend_update_property_long(tui_stdout_context_ce, Z_OBJ_P(return_value),
        "rows", sizeof("rows")-1, obj->app->height);
    zend_update_property_bool(tui_stdout_context_ce, Z_OBJ_P(return_value),
        "isTTY", sizeof("isTTY")-1, isatty(STDOUT_FILENO));
}
/* }}} */

/* {{{ TuiInstance::useStderr(): StderrContext */
PHP_METHOD(TuiInstance, useStderr)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    /* Create StderrContext object */
    object_init_ex(return_value, tui_stderr_context_ce);

    /* Set properties */
    zend_update_property_long(tui_stderr_context_ce, Z_OBJ_P(return_value),
        "columns", sizeof("columns")-1, obj->app->width);
    zend_update_property_long(tui_stderr_context_ce, Z_OBJ_P(return_value),
        "rows", sizeof("rows")-1, obj->app->height);
    zend_update_property_bool(tui_stderr_context_ce, Z_OBJ_P(return_value),
        "isTTY", sizeof("isTTY")-1, isatty(STDERR_FILENO));
}
/* }}} */

/* {{{ TuiInstance::getTerminalSize(): array */
PHP_METHOD(TuiInstance, getTerminalSize)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    array_init(return_value);
    add_assoc_long(return_value, "columns", obj->app->width);
    add_assoc_long(return_value, "rows", obj->app->height);
}
/* }}} */

/* {{{ TuiInstance::getSize(): array - Get terminal size with width/height aliases */
PHP_METHOD(TuiInstance, getSize)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    array_init(return_value);
    add_assoc_long(return_value, "width", obj->app->width);
    add_assoc_long(return_value, "height", obj->app->height);
    add_assoc_long(return_value, "columns", obj->app->width);
    add_assoc_long(return_value, "rows", obj->app->height);
}
/* }}} */

/* {{{ TuiInstance::setState(int $index, mixed $value): void - Internal for setter */
PHP_METHOD(TuiInstance, setState)
{
    zend_long index;
    zval *value;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(index)
        Z_PARAM_ZVAL(value)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }
    if (index < 0 || index >= obj->app->state_count) {
        zend_throw_exception(zend_ce_exception,
            "Invalid state index", 0);
        RETURN_THROWS();
    }

    /* Update state value */
    zval_ptr_dtor(&obj->app->states[index].value);
    ZVAL_COPY(&obj->app->states[index].value, value);

    /* Trigger re-render */
    obj->app->render_pending = 1;
}
/* }}} */

/* {{{ TuiInstance::focusNext(): void */
PHP_METHOD(TuiInstance, focusNext)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (obj->app) {
        tui_app_focus_next(obj->app);
    }
}
/* }}} */

/* {{{ TuiInstance::focusPrev(): void */
PHP_METHOD(TuiInstance, focusPrev)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (obj->app) {
        tui_app_focus_prev(obj->app);
    }
}
/* }}} */

/* Helper: find node by ID in tree (with depth limit to prevent stack overflow) */
static tui_node* find_node_by_id_in_tree_impl(tui_node *node, const char *id, int depth)
{
    if (!node || !id) return NULL;

    /* Prevent stack overflow from deeply nested trees */
    if (depth > MAX_TREE_DEPTH) return NULL;

    if (node->id && strcmp(node->id, id) == 0) {
        return node;
    }

    for (int i = 0; i < node->child_count; i++) {
        tui_node *found = find_node_by_id_in_tree_impl(node->children[i], id, depth + 1);
        if (found) return found;
    }

    return NULL;
}

static tui_node* find_node_by_id_in_tree(tui_node *node, const char *id)
{
    return find_node_by_id_in_tree_impl(node, id, 0);
}

/* {{{ TuiInstance::measureElement(string $id): ?array */
PHP_METHOD(TuiInstance, measureElement)
{
    zend_string *id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(id)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app || !obj->app->root_node) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    tui_node *node = find_node_by_id_in_tree(obj->app->root_node, ZSTR_VAL(id));
    if (!node) {
        RETURN_NULL();
    }

    /* Return computed layout from Yoga */
    array_init(return_value);
    add_assoc_long(return_value, "x", (zend_long)node->x);
    add_assoc_long(return_value, "y", (zend_long)node->y);
    add_assoc_long(return_value, "width", (zend_long)node->width);
    add_assoc_long(return_value, "height", (zend_long)node->height);
}
/* }}} */

/* {{{ TuiInstance::setInputHandler(callable $handler): void */
PHP_METHOD(TuiInstance, setInputHandler)
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }
    tui_app_set_input_handler(obj->app, &fci, &fcc);
}
/* }}} */

/* {{{ TuiInstance::setFocusHandler(callable $handler): void */
PHP_METHOD(TuiInstance, setFocusHandler)
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }
    tui_app_set_focus_handler(obj->app, &fci, &fcc);
}
/* }}} */

/* {{{ TuiInstance::setResizeHandler(callable $handler): void */
PHP_METHOD(TuiInstance, setResizeHandler)
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }
    tui_app_set_resize_handler(obj->app, &fci, &fcc);
}
/* }}} */

/* {{{ TuiInstance::setTickHandler(callable $handler): void */
PHP_METHOD(TuiInstance, setTickHandler)
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }
    tui_app_set_tick_handler(obj->app, &fci, &fcc);
}
/* }}} */

/* {{{ TuiInstance::addTimer(int $intervalMs, callable $callback): int */
PHP_METHOD(TuiInstance, addTimer)
{
    zend_long interval_ms;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(interval_ms)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }

    int timer_id = tui_app_add_timer(obj->app, (int)interval_ms, &fci, &fcc);
    RETURN_LONG(timer_id);
}
/* }}} */

/* {{{ TuiInstance::removeTimer(int $timerId): void */
PHP_METHOD(TuiInstance, removeTimer)
{
    zend_long timer_id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(timer_id)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }
    tui_app_remove_timer(obj->app, (int)timer_id);
}
/* }}} */

/* {{{ TuiInstance::clear(): void */
PHP_METHOD(TuiInstance, clear)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (!obj->app) {
        zend_throw_exception(zend_ce_exception,
            "TuiInstance has been destroyed or unmounted", 0);
        RETURN_THROWS();
    }
    if (obj->app->buffer) {
        tui_buffer_clear(obj->app->buffer);
        tui_output_flush(obj->app->output);
    }
}
/* }}} */

/* {{{ TuiInstance::getCapturedOutput(): ?string */
PHP_METHOD(TuiInstance, getCapturedOutput)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (obj->app && obj->app->captured_output && obj->app->captured_output_len > 0) {
        RETURN_STRINGL(obj->app->captured_output, obj->app->captured_output_len);
    }
    RETURN_NULL();
}
/* }}} */

/* TuiInstance arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_rerender, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_unmount, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_waituntilexit, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_exit, 0, 0, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, code, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_usestate, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, initial, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_useinput, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tuiinstance_usefocus, 0, 0, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tuiinstance_usefocusmanager, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tuiinstance_usestdin, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tuiinstance_usestdout, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tuiinstance_usestderr, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_getterminalsize, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_getsize, 0, 0, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_setstate, 0, 2, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_focusnext, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_focusprev, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_measureelement, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_TYPE_INFO(0, id, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_setinputhandler, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_setfocushandler, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_setresizehandler, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_settickhandler, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_addtimer, 0, 2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, intervalMs, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_removetimer, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, timerId, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_clear, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tuiinstance_getcapturedoutput, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry tui_instance_methods[] = {
    PHP_ME(TuiInstance, rerender, arginfo_tuiinstance_rerender, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, unmount, arginfo_tuiinstance_unmount, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, waitUntilExit, arginfo_tuiinstance_waituntilexit, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, exit, arginfo_tuiinstance_exit, ZEND_ACC_PUBLIC)
    /* New method names */
    PHP_MALIAS(TuiInstance, state, useState, arginfo_tuiinstance_usestate, ZEND_ACC_PUBLIC)
    PHP_MALIAS(TuiInstance, onInput, useInput, arginfo_tuiinstance_useinput, ZEND_ACC_PUBLIC)
    PHP_MALIAS(TuiInstance, focus, useFocus, arginfo_tuiinstance_usefocus, ZEND_ACC_PUBLIC)
    PHP_MALIAS(TuiInstance, focusManager, useFocusManager, arginfo_tuiinstance_usefocusmanager, ZEND_ACC_PUBLIC)
    PHP_MALIAS(TuiInstance, stdin, useStdin, arginfo_tuiinstance_usestdin, ZEND_ACC_PUBLIC)
    PHP_MALIAS(TuiInstance, stdout, useStdout, arginfo_tuiinstance_usestdout, ZEND_ACC_PUBLIC)
    PHP_MALIAS(TuiInstance, stderr, useStderr, arginfo_tuiinstance_usestderr, ZEND_ACC_PUBLIC)
    /* Deprecated aliases (old names) */
    PHP_ME(TuiInstance, useState, arginfo_tuiinstance_usestate, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
    PHP_ME(TuiInstance, useInput, arginfo_tuiinstance_useinput, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
    PHP_ME(TuiInstance, useFocus, arginfo_tuiinstance_usefocus, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
    PHP_ME(TuiInstance, useFocusManager, arginfo_tuiinstance_usefocusmanager, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
    PHP_ME(TuiInstance, useStdin, arginfo_tuiinstance_usestdin, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
    PHP_ME(TuiInstance, useStdout, arginfo_tuiinstance_usestdout, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
    PHP_ME(TuiInstance, useStderr, arginfo_tuiinstance_usestderr, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
    PHP_ME(TuiInstance, getTerminalSize, arginfo_tuiinstance_getterminalsize, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, getSize, arginfo_tuiinstance_getsize, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, setState, arginfo_tuiinstance_setstate, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, focusNext, arginfo_tuiinstance_focusnext, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, focusPrev, arginfo_tuiinstance_focusprev, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, measureElement, arginfo_tuiinstance_measureelement, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, setInputHandler, arginfo_tuiinstance_setinputhandler, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, setFocusHandler, arginfo_tuiinstance_setfocushandler, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, setResizeHandler, arginfo_tuiinstance_setresizehandler, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, setTickHandler, arginfo_tuiinstance_settickhandler, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, addTimer, arginfo_tuiinstance_addtimer, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, removeTimer, arginfo_tuiinstance_removetimer, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, clear, arginfo_tuiinstance_clear, ZEND_ACC_PUBLIC)
    PHP_ME(TuiInstance, getCapturedOutput, arginfo_tuiinstance_getcapturedoutput, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* ------------------------------------------------------------------
 * TuiFocus class
 * ------------------------------------------------------------------ */

/* {{{ Focus::focus(string $id): void */
PHP_METHOD(Focus, focus)
{
    zend_string *id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(id)
    ZEND_PARSE_PARAMETERS_END();

    tui_focus_object *intern = Z_TUI_FOCUS_P(ZEND_THIS);
    if (intern->app) {
        tui_app_focus_by_id(intern->app, ZSTR_VAL(id));
    }
}
/* }}} */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_focus_focus, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, id, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry tui_focus_methods[] = {
    PHP_ME(Focus, focus, arginfo_focus_focus, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* ------------------------------------------------------------------
 * TuiFocusManager class
 * ------------------------------------------------------------------ */

/* {{{ FocusManager::focusNext(): void */
PHP_METHOD(FocusManager, focusNext)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_focus_manager_object *intern = Z_TUI_FOCUS_MANAGER_P(ZEND_THIS);
    if (intern->app) {
        tui_app_focus_next(intern->app);
    }
}
/* }}} */

/* {{{ FocusManager::focusPrevious(): void */
PHP_METHOD(FocusManager, focusPrevious)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_focus_manager_object *intern = Z_TUI_FOCUS_MANAGER_P(ZEND_THIS);
    if (intern->app) {
        tui_app_focus_prev(intern->app);
    }
}
/* }}} */

/* {{{ FocusManager::focus(string $id): void */
PHP_METHOD(FocusManager, focus)
{
    zend_string *id;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(id)
    ZEND_PARSE_PARAMETERS_END();

    tui_focus_manager_object *intern = Z_TUI_FOCUS_MANAGER_P(ZEND_THIS);
    if (intern->app) {
        tui_app_focus_by_id(intern->app, ZSTR_VAL(id));
    }
}
/* }}} */

/* {{{ FocusManager::enableFocus(): void */
PHP_METHOD(FocusManager, enableFocus)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_focus_manager_object *intern = Z_TUI_FOCUS_MANAGER_P(ZEND_THIS);
    if (intern->app) {
        tui_app_enable_focus(intern->app);
    }
}
/* }}} */

/* {{{ FocusManager::disableFocus(): void */
PHP_METHOD(FocusManager, disableFocus)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_focus_manager_object *intern = Z_TUI_FOCUS_MANAGER_P(ZEND_THIS);
    if (intern->app) {
        tui_app_disable_focus(intern->app);
    }
}
/* }}} */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_focusmanager_focusnext, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_focusmanager_focusprevious, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_focusmanager_focus, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, id, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_focusmanager_enablefocus, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_focusmanager_disablefocus, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry tui_focus_manager_methods[] = {
    PHP_ME(FocusManager, focusNext, arginfo_focusmanager_focusnext, ZEND_ACC_PUBLIC)
    PHP_ME(FocusManager, focusPrevious, arginfo_focusmanager_focusprevious, ZEND_ACC_PUBLIC)
    PHP_ME(FocusManager, focus, arginfo_focusmanager_focus, ZEND_ACC_PUBLIC)
    PHP_ME(FocusManager, enableFocus, arginfo_focusmanager_enablefocus, ZEND_ACC_PUBLIC)
    PHP_ME(FocusManager, disableFocus, arginfo_focusmanager_disablefocus, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* ------------------------------------------------------------------
 * Stream Context classes (StdinContext, StdoutContext, StderrContext)
 * ------------------------------------------------------------------ */

/* {{{ StdoutContext::write(string $data): void */
PHP_METHOD(StdoutContext, write)
{
    zend_string *data;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();

    /* Write to stdout (outside of TUI rendering) */
    fwrite(ZSTR_VAL(data), 1, ZSTR_LEN(data), stdout);
    fflush(stdout);
}
/* }}} */

/* {{{ StderrContext::write(string $data): void */
PHP_METHOD(StderrContext, write)
{
    zend_string *data;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();

    /* Write to stderr */
    fwrite(ZSTR_VAL(data), 1, ZSTR_LEN(data), stderr);
    fflush(stderr);
}
/* }}} */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_context_write, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry tui_stdin_context_methods[] = {
    PHP_FE_END
};

static const zend_function_entry tui_stdout_context_methods[] = {
    PHP_ME(StdoutContext, write, arginfo_context_write, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

static const zend_function_entry tui_stderr_context_methods[] = {
    PHP_ME(StderrContext, write, arginfo_context_write, ZEND_ACC_PUBLIC)
    PHP_FE_END
};


/* {{{ arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_terminal_size, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_is_interactive, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_is_ci, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

/* Cursor shape functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_cursor_shape, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, shape, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_cursor_show, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_cursor_hide, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

/* Window title functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_set_title, 0, 1, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, title, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_reset_title, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

/* Capability functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_capabilities, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_has_capability, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

/* Notification functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_bell, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_flash, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_notify, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, title, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, body, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, priority, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

/* Mouse functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_mouse_enable, 0, 0, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "2")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_mouse_disable, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_mouse_get_mode, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

/* Bracketed paste functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_bracketed_paste_enable, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_bracketed_paste_disable, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_bracketed_paste_is_enabled, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

/* Clipboard functions */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_clipboard_copy, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, target, IS_STRING, 0, "\"clipboard\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_clipboard_request, 0, 0, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, target, IS_STRING, 0, "\"clipboard\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_clipboard_clear, 0, 0, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, target, IS_STRING, 0, "\"clipboard\"")
ZEND_END_ARG_INFO()

/* History functions */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_history_create, 0, 0, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, maxEntries, IS_LONG, 0, "1000")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_history_destroy, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, history)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_history_add, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, history)
    ZEND_ARG_TYPE_INFO(0, entry, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_history_prev, 0, 1, IS_STRING, 1)
    ZEND_ARG_INFO(0, history)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_history_next, 0, 1, IS_STRING, 1)
    ZEND_ARG_INFO(0, history)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_history_reset, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, history)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_history_save_temp, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, history)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_history_get_temp, 0, 1, IS_STRING, 1)
    ZEND_ARG_INFO(0, history)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_string_width, 0, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_wrap_text, 0, 2, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_truncate, 0, 2, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, ellipsis, IS_STRING, 0, "\"...\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, position, IS_STRING, 0, "\"end\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_render, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, component, IS_CALLABLE, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_ARRAY, 1, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_rerender, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_unmount, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_wait_until_exit, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_set_input_handler, 0, 2, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_set_focus_handler, 0, 2, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_focus_next, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_focus_prev, 0, 1, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_set_resize_handler, 0, 2, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_size, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_focused_node, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
ZEND_END_ARG_INFO()

/* Timer arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_add_timer, 0, 3, IS_LONG, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
    ZEND_ARG_TYPE_INFO(0, interval_ms, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_remove_timer, 0, 2, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
    ZEND_ARG_TYPE_INFO(0, timer_id, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_set_tick_handler, 0, 2, IS_VOID, 0)
    ZEND_ARG_OBJ_INFO(0, instance, TuiInstance, 0)
    ZEND_ARG_TYPE_INFO(0, handler, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

/* Text utility arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_pad, 0, 2, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, align, IS_STRING, 0, "\"l\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, pad_char, IS_STRING, 0, "\" \"")
ZEND_END_ARG_INFO()

/* ANSI utility arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_strip_ansi, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_string_width_ansi, 0, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_slice_ansi, 0, 3, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, start, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, end, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_grapheme_count, 0, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

/* Buffer arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_buffer_create, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_buffer_clear, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_buffer_render, 0, 1, IS_STRING, 0)
    ZEND_ARG_INFO(0, buffer)
ZEND_END_ARG_INFO()

/* Drawing primitives arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_draw_line, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, x2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_draw_rect, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, border, IS_STRING, 1, "\"single\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_fill_rect, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "\" \"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_draw_circle, 0, 4, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, cx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, cy, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, radius, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_fill_circle, 0, 4, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, cx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, cy, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, radius, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_draw_ellipse, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, cx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, cy, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, rx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, ry, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_fill_ellipse, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, cx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, cy, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, rx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, ry, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_draw_triangle, 0, 7, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, x2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, x3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_fill_triangle, 0, 7, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, x2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, x3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y3, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, char, IS_STRING, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

/* Canvas arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_canvas_create, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_STRING, 1, "\"braille\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_set, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_unset, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_toggle, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_get, 0, 3, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_clear, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_line, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, x1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, x2, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y2, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_rect, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_fill_rect, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_circle, 0, 4, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, cx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, cy, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, radius, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_fill_circle, 0, 4, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, cx, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, cy, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, radius, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_set_color, 0, 4, IS_VOID, 0)
    ZEND_ARG_INFO(0, canvas)
    ZEND_ARG_TYPE_INFO(0, r, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, g, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, b, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_get_resolution, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_INFO(0, canvas)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_canvas_render, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_INFO(0, canvas)
ZEND_END_ARG_INFO()

/* Animation arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_ease, 0, 1, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, t, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, easing, IS_STRING, 1, "\"linear\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_lerp, 0, 3, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, a, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, b, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, t, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_lerp_color, 0, 3, IS_STRING, 0)
    ZEND_ARG_INFO(0, a)
    ZEND_ARG_INFO(0, b)
    ZEND_ARG_TYPE_INFO(0, t, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_color_from_hex, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, hex, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_gradient, 0, 2, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, colors, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, steps, IS_LONG, 0)
ZEND_END_ARG_INFO()

/* Table arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_table_create, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, headers, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_table_add_row, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_TYPE_INFO(0, cells, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_table_set_align, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_TYPE_INFO(0, column, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, right_align, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_table_render_to_buffer, 0, 4, IS_LONG, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, border, IS_STRING, 1, "\"single\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, header_style, IS_ARRAY, 1, "null")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cell_style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

/* Progress/Spinner arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_render_progress_bar, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, progress, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_render_busy_bar, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, frame, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style_name, IS_STRING, 1, "\"pulse\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_spinner_frame, 0, 2, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, frame, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_spinner_frame_count, 0, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_render_spinner, 0, 5, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, frame, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, style, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

/* Sprite arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_sprite_create, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, frames, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, name, IS_STRING, 1, "\"default\"")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, loop, _IS_BOOL, 0, "true")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_update, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, sprite)
    ZEND_ARG_TYPE_INFO(0, delta_ms, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_set_animation, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, sprite)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_set_position, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, sprite)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_flip, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, sprite)
    ZEND_ARG_TYPE_INFO(0, flipped, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_set_visible, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, sprite)
    ZEND_ARG_TYPE_INFO(0, visible, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_render, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_INFO(0, sprite)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_get_bounds, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_INFO(0, sprite)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sprite_collides, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, a)
    ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()

/* Testing framework arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_test_create, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_destroy, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, renderer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_render, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_OBJ_INFO(0, element, TuiBox, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_get_output, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_INFO(0, renderer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_to_string, 0, 1, IS_STRING, 0)
    ZEND_ARG_INFO(0, renderer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_send_input, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_send_key, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_TYPE_INFO(0, keyCode, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_advance_frame, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, renderer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_run_timers, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_TYPE_INFO(0, ms, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_test_get_by_id, 0, 0, 2)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_TYPE_INFO(0, id, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_test_get_by_text, 0, 2, IS_ARRAY, 0)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

/* Metrics functions arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_metrics_enable, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_metrics_disable, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_metrics_enabled, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_metrics_reset, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_metrics, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_node_metrics, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_reconciler_metrics, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_render_metrics, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_loop_metrics, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_pool_metrics, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

/* Virtual list functions arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_virtual_create, 0, 0, 3)
    ZEND_ARG_TYPE_INFO(0, item_count, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, item_height, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, viewport_height, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, overscan, IS_LONG, 0, "5")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_get_range, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_INFO(0, vlist)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_scroll_to, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_scroll_by, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, delta, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_scroll_items, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, items, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_ensure_visible, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_page_up, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_page_down, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_scroll_top, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_scroll_bottom, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_set_count, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, item_count, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_set_viewport, 0, 2, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, viewport_height, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_item_offset, 0, 2, IS_LONG, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_is_visible, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, vlist)
    ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_virtual_destroy, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, vlist)
ZEND_END_ARG_INFO()

/* Smooth scrolling functions arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_scroll_create, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_set_spring, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, anim)
    ZEND_ARG_TYPE_INFO(0, stiffness, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, damping, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_set_target, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, anim)
    ZEND_ARG_TYPE_INFO(0, x, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_by, 0, 3, IS_VOID, 0)
    ZEND_ARG_INFO(0, anim)
    ZEND_ARG_TYPE_INFO(0, dx, IS_DOUBLE, 0)
    ZEND_ARG_TYPE_INFO(0, dy, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_update, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, anim)
    ZEND_ARG_TYPE_INFO(0, dt, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_snap, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, anim)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_get_position, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_INFO(0, anim)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_is_animating, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, anim)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_progress, 0, 1, IS_DOUBLE, 0)
    ZEND_ARG_INFO(0, anim)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_scroll_destroy, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, anim)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ Graphics arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_image_load, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_image_create, 0, 0, 3)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, format, IS_STRING, 0, "\"rgba\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_image_transmit, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, image)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_image_display, 0, 3, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, image)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, cols, IS_LONG, 0, "0")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, rows, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_image_delete, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, image)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_image_clear, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_image_destroy, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, image)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_image_get_info, 0, 1, IS_ARRAY, 1)
    ZEND_ARG_INFO(0, image)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_graphics_supported, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_graphics_protocol, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_iterm2_supported, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_sixel_supported, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

/* Recording arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tui_record_create, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, width, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, height, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, title, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_start, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_pause, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_resume, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_stop, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_capture, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, recording)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_duration, 0, 1, IS_DOUBLE, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_frame_count, 0, 1, IS_LONG, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_export, 0, 1, IS_STRING, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_save, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_INFO(0, recording)
    ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_record_destroy, 0, 1, IS_VOID, 0)
    ZEND_ARG_INFO(0, recording)
ZEND_END_ARG_INFO()

/* Accessibility arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_announce, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, message, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, priority, IS_STRING, 0, "\"polite\"")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_prefers_reduced_motion, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_prefers_high_contrast, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_accessibility_features, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_aria_role_to_string, 0, 1, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, role, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_aria_role_from_string, 0, 1, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, role, IS_STRING, 0)
ZEND_END_ARG_INFO()

/* Drag and drop arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_start, 0, 4, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_move, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, x, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, y, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_end, 0, 0, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dropped, _IS_BOOL, 0, "true")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_cancel, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_is_active, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_get_type, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_get_data, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_drag_get_state, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ tui_functions[] */
static const zend_function_entry tui_functions[] = {
    /* Terminal info */
    PHP_FE(tui_get_terminal_size, arginfo_tui_get_terminal_size)
    PHP_FE(tui_is_interactive, arginfo_tui_is_interactive)
    PHP_FE(tui_is_ci, arginfo_tui_is_ci)

    /* Cursor control */
    PHP_FE(tui_cursor_shape, arginfo_tui_cursor_shape)
    PHP_FE(tui_cursor_show, arginfo_tui_cursor_show)
    PHP_FE(tui_cursor_hide, arginfo_tui_cursor_hide)

    /* Window title */
    PHP_FE(tui_set_title, arginfo_tui_set_title)
    PHP_FE(tui_reset_title, arginfo_tui_reset_title)

    /* Capabilities */
    PHP_FE(tui_get_capabilities, arginfo_tui_get_capabilities)
    PHP_FE(tui_has_capability, arginfo_tui_has_capability)

    /* Notifications */
    PHP_FE(tui_bell, arginfo_tui_bell)
    PHP_FE(tui_flash, arginfo_tui_flash)
    PHP_FE(tui_notify, arginfo_tui_notify)

    /* Mouse support */
    PHP_FE(tui_mouse_enable, arginfo_tui_mouse_enable)
    PHP_FE(tui_mouse_disable, arginfo_tui_mouse_disable)
    PHP_FE(tui_mouse_get_mode, arginfo_tui_mouse_get_mode)

    /* Bracketed paste */
    PHP_FE(tui_bracketed_paste_enable, arginfo_tui_bracketed_paste_enable)
    PHP_FE(tui_bracketed_paste_disable, arginfo_tui_bracketed_paste_disable)
    PHP_FE(tui_bracketed_paste_is_enabled, arginfo_tui_bracketed_paste_is_enabled)

    /* Clipboard (OSC 52) */
    PHP_FE(tui_clipboard_copy, arginfo_tui_clipboard_copy)
    PHP_FE(tui_clipboard_request, arginfo_tui_clipboard_request)
    PHP_FE(tui_clipboard_clear, arginfo_tui_clipboard_clear)

    /* Input history */
    PHP_FE(tui_history_create, arginfo_tui_history_create)
    PHP_FE(tui_history_destroy, arginfo_tui_history_destroy)
    PHP_FE(tui_history_add, arginfo_tui_history_add)
    PHP_FE(tui_history_prev, arginfo_tui_history_prev)
    PHP_FE(tui_history_next, arginfo_tui_history_next)
    PHP_FE(tui_history_reset, arginfo_tui_history_reset)
    PHP_FE(tui_history_save_temp, arginfo_tui_history_save_temp)
    PHP_FE(tui_history_get_temp, arginfo_tui_history_get_temp)

    /* Text utilities */
    PHP_FE(tui_string_width, arginfo_tui_string_width)
    PHP_FE(tui_wrap_text, arginfo_tui_wrap_text)
    PHP_FE(tui_truncate, arginfo_tui_truncate)
    PHP_FE(tui_pad, arginfo_tui_pad)

    /* ANSI utilities */
    PHP_FE(tui_strip_ansi, arginfo_tui_strip_ansi)
    PHP_FE(tui_string_width_ansi, arginfo_tui_string_width_ansi)
    PHP_FE(tui_slice_ansi, arginfo_tui_slice_ansi)

    /* Grapheme utilities */
    PHP_FE(tui_grapheme_count, arginfo_tui_grapheme_count)

    /* Render/App */
    PHP_FE(tui_render, arginfo_tui_render)
    PHP_FE(tui_rerender, arginfo_tui_rerender)
    PHP_FE(tui_unmount, arginfo_tui_unmount)
    PHP_FE(tui_wait_until_exit, arginfo_tui_wait_until_exit)
    PHP_FE(tui_set_input_handler, arginfo_tui_set_input_handler)
    PHP_FE(tui_set_focus_handler, arginfo_tui_set_focus_handler)
    PHP_FE(tui_focus_next, arginfo_tui_focus_next)
    PHP_FE(tui_focus_prev, arginfo_tui_focus_prev)
    PHP_FE(tui_set_resize_handler, arginfo_tui_set_resize_handler)
    PHP_FE(tui_get_size, arginfo_tui_get_size)
    PHP_FE(tui_get_focused_node, arginfo_tui_get_focused_node)

    /* Timers */
    PHP_FE(tui_add_timer, arginfo_tui_add_timer)
    PHP_FE(tui_remove_timer, arginfo_tui_remove_timer)
    PHP_FE(tui_set_tick_handler, arginfo_tui_set_tick_handler)

    /* Buffer */
    PHP_FE(tui_buffer_create, arginfo_tui_buffer_create)
    PHP_FE(tui_buffer_clear, arginfo_tui_buffer_clear)
    PHP_FE(tui_buffer_render, arginfo_tui_buffer_render)

    /* Drawing primitives */
    PHP_FE(tui_draw_line, arginfo_tui_draw_line)
    PHP_FE(tui_draw_rect, arginfo_tui_draw_rect)
    PHP_FE(tui_fill_rect, arginfo_tui_fill_rect)
    PHP_FE(tui_draw_circle, arginfo_tui_draw_circle)
    PHP_FE(tui_fill_circle, arginfo_tui_fill_circle)
    PHP_FE(tui_draw_ellipse, arginfo_tui_draw_ellipse)
    PHP_FE(tui_fill_ellipse, arginfo_tui_fill_ellipse)
    PHP_FE(tui_draw_triangle, arginfo_tui_draw_triangle)
    PHP_FE(tui_fill_triangle, arginfo_tui_fill_triangle)

    /* Canvas (braille) */
    PHP_FE(tui_canvas_create, arginfo_tui_canvas_create)
    PHP_FE(tui_canvas_set, arginfo_tui_canvas_set)
    PHP_FE(tui_canvas_unset, arginfo_tui_canvas_unset)
    PHP_FE(tui_canvas_toggle, arginfo_tui_canvas_toggle)
    PHP_FE(tui_canvas_get, arginfo_tui_canvas_get)
    PHP_FE(tui_canvas_clear, arginfo_tui_canvas_clear)
    PHP_FE(tui_canvas_line, arginfo_tui_canvas_line)
    PHP_FE(tui_canvas_rect, arginfo_tui_canvas_rect)
    PHP_FE(tui_canvas_fill_rect, arginfo_tui_canvas_fill_rect)
    PHP_FE(tui_canvas_circle, arginfo_tui_canvas_circle)
    PHP_FE(tui_canvas_fill_circle, arginfo_tui_canvas_fill_circle)
    PHP_FE(tui_canvas_set_color, arginfo_tui_canvas_set_color)
    PHP_FE(tui_canvas_get_resolution, arginfo_tui_canvas_get_resolution)
    PHP_FE(tui_canvas_render, arginfo_tui_canvas_render)

    /* Animation */
    PHP_FE(tui_ease, arginfo_tui_ease)
    PHP_FE(tui_lerp, arginfo_tui_lerp)
    PHP_FE(tui_lerp_color, arginfo_tui_lerp_color)
    PHP_FE(tui_color_from_hex, arginfo_tui_color_from_hex)
    PHP_FE(tui_gradient, arginfo_tui_gradient)

    /* Table */
    PHP_FE(tui_table_create, arginfo_tui_table_create)
    PHP_FE(tui_table_add_row, arginfo_tui_table_add_row)
    PHP_FE(tui_table_set_align, arginfo_tui_table_set_align)
    PHP_FE(tui_table_render_to_buffer, arginfo_tui_table_render_to_buffer)

    /* Progress/Spinner */
    PHP_FE(tui_render_progress_bar, arginfo_tui_render_progress_bar)
    PHP_FE(tui_render_busy_bar, arginfo_tui_render_busy_bar)
    PHP_FE(tui_spinner_frame, arginfo_tui_spinner_frame)
    PHP_FE(tui_spinner_frame_count, arginfo_tui_spinner_frame_count)
    PHP_FE(tui_render_spinner, arginfo_tui_render_spinner)

    /* Sprite */
    PHP_FE(tui_sprite_create, arginfo_tui_sprite_create)
    PHP_FE(tui_sprite_update, arginfo_tui_sprite_update)
    PHP_FE(tui_sprite_set_animation, arginfo_tui_sprite_set_animation)
    PHP_FE(tui_sprite_set_position, arginfo_tui_sprite_set_position)
    PHP_FE(tui_sprite_flip, arginfo_tui_sprite_flip)
    PHP_FE(tui_sprite_set_visible, arginfo_tui_sprite_set_visible)
    PHP_FE(tui_sprite_render, arginfo_tui_sprite_render)
    PHP_FE(tui_sprite_get_bounds, arginfo_tui_sprite_get_bounds)
    PHP_FE(tui_sprite_collides, arginfo_tui_sprite_collides)

    /* Testing framework */
    PHP_FE(tui_test_create, arginfo_tui_test_create)
    PHP_FE(tui_test_destroy, arginfo_tui_test_destroy)
    PHP_FE(tui_test_render, arginfo_tui_test_render)
    PHP_FE(tui_test_get_output, arginfo_tui_test_get_output)
    PHP_FE(tui_test_to_string, arginfo_tui_test_to_string)
    PHP_FE(tui_test_send_input, arginfo_tui_test_send_input)
    PHP_FE(tui_test_send_key, arginfo_tui_test_send_key)
    PHP_FE(tui_test_advance_frame, arginfo_tui_test_advance_frame)
    PHP_FE(tui_test_run_timers, arginfo_tui_test_run_timers)
    PHP_FE(tui_test_get_by_id, arginfo_tui_test_get_by_id)
    PHP_FE(tui_test_get_by_text, arginfo_tui_test_get_by_text)

    /* Metrics functions */
    PHP_FE(tui_metrics_enable, arginfo_tui_metrics_enable)
    PHP_FE(tui_metrics_disable, arginfo_tui_metrics_disable)
    PHP_FE(tui_metrics_enabled, arginfo_tui_metrics_enabled)
    PHP_FE(tui_metrics_reset, arginfo_tui_metrics_reset)
    PHP_FE(tui_get_metrics, arginfo_tui_get_metrics)
    PHP_FE(tui_get_node_metrics, arginfo_tui_get_node_metrics)
    PHP_FE(tui_get_reconciler_metrics, arginfo_tui_get_reconciler_metrics)
    PHP_FE(tui_get_render_metrics, arginfo_tui_get_render_metrics)
    PHP_FE(tui_get_loop_metrics, arginfo_tui_get_loop_metrics)
    PHP_FE(tui_get_pool_metrics, arginfo_tui_get_pool_metrics)

    /* Virtual list / windowing */
    PHP_FE(tui_virtual_create, arginfo_tui_virtual_create)
    PHP_FE(tui_virtual_get_range, arginfo_tui_virtual_get_range)
    PHP_FE(tui_virtual_scroll_to, arginfo_tui_virtual_scroll_to)
    PHP_FE(tui_virtual_scroll_by, arginfo_tui_virtual_scroll_by)
    PHP_FE(tui_virtual_scroll_items, arginfo_tui_virtual_scroll_items)
    PHP_FE(tui_virtual_ensure_visible, arginfo_tui_virtual_ensure_visible)
    PHP_FE(tui_virtual_page_up, arginfo_tui_virtual_page_up)
    PHP_FE(tui_virtual_page_down, arginfo_tui_virtual_page_down)
    PHP_FE(tui_virtual_scroll_top, arginfo_tui_virtual_scroll_top)
    PHP_FE(tui_virtual_scroll_bottom, arginfo_tui_virtual_scroll_bottom)
    PHP_FE(tui_virtual_set_count, arginfo_tui_virtual_set_count)
    PHP_FE(tui_virtual_set_viewport, arginfo_tui_virtual_set_viewport)
    PHP_FE(tui_virtual_item_offset, arginfo_tui_virtual_item_offset)
    PHP_FE(tui_virtual_is_visible, arginfo_tui_virtual_is_visible)
    PHP_FE(tui_virtual_destroy, arginfo_tui_virtual_destroy)

    /* Smooth scrolling */
    PHP_FE(tui_scroll_create, arginfo_tui_scroll_create)
    PHP_FE(tui_scroll_set_spring, arginfo_tui_scroll_set_spring)
    PHP_FE(tui_scroll_set_target, arginfo_tui_scroll_set_target)
    PHP_FE(tui_scroll_by, arginfo_tui_scroll_by)
    PHP_FE(tui_scroll_update, arginfo_tui_scroll_update)
    PHP_FE(tui_scroll_snap, arginfo_tui_scroll_snap)
    PHP_FE(tui_scroll_get_position, arginfo_tui_scroll_get_position)
    PHP_FE(tui_scroll_is_animating, arginfo_tui_scroll_is_animating)
    PHP_FE(tui_scroll_progress, arginfo_tui_scroll_progress)
    PHP_FE(tui_scroll_destroy, arginfo_tui_scroll_destroy)

    /* Graphics (Kitty protocol) */
    PHP_FE(tui_image_load, arginfo_tui_image_load)
    PHP_FE(tui_image_create, arginfo_tui_image_create)
    PHP_FE(tui_image_transmit, arginfo_tui_image_transmit)
    PHP_FE(tui_image_display, arginfo_tui_image_display)
    PHP_FE(tui_image_delete, arginfo_tui_image_delete)
    PHP_FE(tui_image_clear, arginfo_tui_image_clear)
    PHP_FE(tui_image_destroy, arginfo_tui_image_destroy)
    PHP_FE(tui_image_get_info, arginfo_tui_image_get_info)
    PHP_FE(tui_graphics_supported, arginfo_tui_graphics_supported)
    PHP_FE(tui_graphics_protocol, arginfo_tui_graphics_protocol)
    PHP_FE(tui_iterm2_supported, arginfo_tui_iterm2_supported)
    PHP_FE(tui_sixel_supported, arginfo_tui_sixel_supported)

    /* Recording */
    PHP_FE(tui_record_create, arginfo_tui_record_create)
    PHP_FE(tui_record_start, arginfo_tui_record_start)
    PHP_FE(tui_record_pause, arginfo_tui_record_pause)
    PHP_FE(tui_record_resume, arginfo_tui_record_resume)
    PHP_FE(tui_record_stop, arginfo_tui_record_stop)
    PHP_FE(tui_record_capture, arginfo_tui_record_capture)
    PHP_FE(tui_record_duration, arginfo_tui_record_duration)
    PHP_FE(tui_record_frame_count, arginfo_tui_record_frame_count)
    PHP_FE(tui_record_export, arginfo_tui_record_export)
    PHP_FE(tui_record_save, arginfo_tui_record_save)
    PHP_FE(tui_record_destroy, arginfo_tui_record_destroy)

    /* Accessibility */
    PHP_FE(tui_announce, arginfo_tui_announce)
    PHP_FE(tui_prefers_reduced_motion, arginfo_tui_prefers_reduced_motion)
    PHP_FE(tui_prefers_high_contrast, arginfo_tui_prefers_high_contrast)
    PHP_FE(tui_get_accessibility_features, arginfo_tui_get_accessibility_features)
    PHP_FE(tui_aria_role_to_string, arginfo_tui_aria_role_to_string)
    PHP_FE(tui_aria_role_from_string, arginfo_tui_aria_role_from_string)

    /* Drag and drop */
    PHP_FE(tui_drag_start, arginfo_tui_drag_start)
    PHP_FE(tui_drag_move, arginfo_tui_drag_move)
    PHP_FE(tui_drag_end, arginfo_tui_drag_end)
    PHP_FE(tui_drag_cancel, arginfo_tui_drag_cancel)
    PHP_FE(tui_drag_is_active, arginfo_tui_drag_is_active)
    PHP_FE(tui_drag_get_type, arginfo_tui_drag_get_type)
    PHP_FE(tui_drag_get_data, arginfo_tui_drag_get_data)
    PHP_FE(tui_drag_get_state, arginfo_tui_drag_get_state)

    PHP_FE_END
};
/* }}} */

/* {{{ PHP_GINIT_FUNCTION */
static PHP_GINIT_FUNCTION(tui)
{
#if defined(COMPILE_DL_TUI) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    tui_globals->raw_mode_active = 0;
    tui_globals->original_termios_saved = 0;
    tui_globals->terminal_width = 80;
    tui_globals->terminal_height = 24;
    tui_globals->metrics_enabled = 0;
    memset(&tui_globals->metrics, 0, sizeof(tui_metrics));
    tui_globals->pools = NULL;
}
/* }}} */

/* {{{ INI settings */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("tui.max_buffer_width", "500", PHP_INI_ALL,
                      OnUpdateLong, max_buffer_width, zend_tui_globals, tui_globals)
    STD_PHP_INI_ENTRY("tui.max_buffer_height", "500", PHP_INI_ALL,
                      OnUpdateLong, max_buffer_height, zend_tui_globals, tui_globals)
    STD_PHP_INI_ENTRY("tui.max_tree_depth", "100", PHP_INI_ALL,
                      OnUpdateLong, max_tree_depth, zend_tui_globals, tui_globals)
    STD_PHP_INI_ENTRY("tui.max_states", "64", PHP_INI_ALL,
                      OnUpdateLong, max_states, zend_tui_globals, tui_globals)
    STD_PHP_INI_ENTRY("tui.max_timers", "32", PHP_INI_ALL,
                      OnUpdateLong, max_timers, zend_tui_globals, tui_globals)
    STD_PHP_INI_ENTRY("tui.min_render_interval", "16", PHP_INI_ALL,
                      OnUpdateLong, min_render_interval, zend_tui_globals, tui_globals)
    STD_PHP_INI_ENTRY("tui.metrics_enabled", "0", PHP_INI_ALL,
                      OnUpdateBool, metrics_enabled, zend_tui_globals, tui_globals)
PHP_INI_END()
/* }}} */

/* {{{ PHP_MINIT_FUNCTION */
static PHP_MINIT_FUNCTION(tui)
{
    zend_class_entry ce;

    REGISTER_INI_ENTRIES();

    /* Apply INI settings to subsystems */
    tui_buffer_set_max_dimensions(
        (int)TUI_G(max_buffer_width),
        (int)TUI_G(max_buffer_height)
    );

    /* Initialize shared Yoga configuration */
    TUI_G(yoga_config) = YGConfigNew();
    if (TUI_G(yoga_config)) {
        YGConfigSetPointScaleFactor(TUI_G(yoga_config), 1.0f);  /* Terminal uses integer positions */
        YGConfigSetUseWebDefaults(TUI_G(yoga_config), false);
    }

    /* Initialize object pools */
    TUI_G(pools) = malloc(sizeof(tui_pools));
    if (TUI_G(pools)) {
        if (tui_pools_init(TUI_G(pools)) != 0) {
            free(TUI_G(pools));
            TUI_G(pools) = NULL;
        }
    }

    /* Register resource types for canvas, table, sprite, buffer, test renderer */
    le_tui_canvas = zend_register_list_destructors_ex(tui_canvas_dtor, NULL, TUI_CANVAS_RES_NAME, module_number);
    le_tui_table = zend_register_list_destructors_ex(tui_table_dtor, NULL, TUI_TABLE_RES_NAME, module_number);
    le_tui_sprite = zend_register_list_destructors_ex(tui_sprite_dtor, NULL, TUI_SPRITE_RES_NAME, module_number);
    le_tui_buffer = zend_register_list_destructors_ex(tui_buffer_dtor, NULL, TUI_BUFFER_RES_NAME, module_number);
    le_tui_test_renderer = zend_register_list_destructors_ex(tui_test_renderer_dtor, NULL, TUI_TEST_RENDERER_RES_NAME, module_number);
    le_tui_history = zend_register_list_destructors_ex(tui_history_dtor, NULL, TUI_HISTORY_RES_NAME, module_number);
    le_tui_virtual_list = zend_register_list_destructors_ex(tui_virtual_list_dtor, NULL, TUI_VIRTUAL_LIST_RES_NAME, module_number);
    le_tui_scroll_animation = zend_register_list_destructors_ex(tui_scroll_animation_dtor, NULL, TUI_SCROLL_ANIM_RES_NAME, module_number);
    le_tui_image = zend_register_list_destructors_ex(tui_image_dtor, NULL, TUI_IMAGE_RES_NAME, module_number);
    le_tui_recording = zend_register_list_destructors_ex(tui_recording_dtor, NULL, "TuiRecording", module_number);

    /* Register version constants */
    REGISTER_STRING_CONSTANT("TUI_VERSION", PHP_TUI_VERSION, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_VERSION_ID", PHP_TUI_VERSION_ID, CONST_CS | CONST_PERSISTENT);

    /* Register mouse mode constants */
    REGISTER_LONG_CONSTANT("TUI_MOUSE_MODE_OFF", TUI_MOUSE_MODE_OFF, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_MOUSE_MODE_CLICK", TUI_MOUSE_MODE_CLICK, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_MOUSE_MODE_BUTTON", TUI_MOUSE_MODE_BUTTON, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_MOUSE_MODE_ALL", TUI_MOUSE_MODE_ALL, CONST_CS | CONST_PERSISTENT);

    /* Register notification priority constants */
    REGISTER_LONG_CONSTANT("TUI_NOTIFY_NORMAL", 0, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_NOTIFY_URGENT", 1, CONST_CS | CONST_PERSISTENT);

    /* Register ARIA role constants */
    REGISTER_LONG_CONSTANT("TUI_ARIA_ROLE_NONE", 0, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_ARIA_ROLE_BUTTON", 1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_ARIA_ROLE_CHECKBOX", 2, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_ARIA_ROLE_DIALOG", 3, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_ARIA_ROLE_NAVIGATION", 14, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_ARIA_ROLE_MENU", 12, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_ARIA_ROLE_MENUITEM", 13, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_ARIA_ROLE_TEXTBOX", 26, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_ARIA_ROLE_ALERT", 29, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_ARIA_ROLE_STATUS", 22, CONST_CS | CONST_PERSISTENT);

    /* Register Xocdr\Tui\Ext\Box class with methods */
    INIT_CLASS_ENTRY(ce, "Xocdr\\Tui\\Ext\\Box", tui_box_methods);
    tui_box_ce = zend_register_internal_class(&ce);

    /* TuiBox properties */
    zend_declare_property_string(tui_box_ce, "flexDirection", sizeof("flexDirection")-1, "column", ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "alignItems", sizeof("alignItems")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "justifyContent", sizeof("justifyContent")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "flexGrow", sizeof("flexGrow")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "flexShrink", sizeof("flexShrink")-1, 1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "flexBasis", sizeof("flexBasis")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "alignSelf", sizeof("alignSelf")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "width", sizeof("width")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "height", sizeof("height")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "padding", sizeof("padding")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "paddingTop", sizeof("paddingTop")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "paddingBottom", sizeof("paddingBottom")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "paddingLeft", sizeof("paddingLeft")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "paddingRight", sizeof("paddingRight")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "paddingX", sizeof("paddingX")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "paddingY", sizeof("paddingY")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "margin", sizeof("margin")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "marginTop", sizeof("marginTop")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "marginBottom", sizeof("marginBottom")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "marginLeft", sizeof("marginLeft")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "marginRight", sizeof("marginRight")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "marginX", sizeof("marginX")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "marginY", sizeof("marginY")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "gap", sizeof("gap")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "columnGap", sizeof("columnGap")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "rowGap", sizeof("rowGap")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "flexWrap", sizeof("flexWrap")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "minWidth", sizeof("minWidth")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "minHeight", sizeof("minHeight")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "maxWidth", sizeof("maxWidth")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "maxHeight", sizeof("maxHeight")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_string(tui_box_ce, "overflow", sizeof("overflow")-1, "visible", ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "overflowX", sizeof("overflowX")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "overflowY", sizeof("overflowY")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_string(tui_box_ce, "display", sizeof("display")-1, "flex", ZEND_ACC_PUBLIC);
    zend_declare_property_string(tui_box_ce, "position", sizeof("position")-1, "relative", ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "aspectRatio", sizeof("aspectRatio")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "direction", sizeof("direction")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "borderStyle", sizeof("borderStyle")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "borderColor", sizeof("borderColor")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_box_ce, "focusable", sizeof("focusable")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_box_ce, "focused", sizeof("focused")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_box_ce, "tabIndex", sizeof("tabIndex")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "focusGroup", sizeof("focusGroup")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_box_ce, "autoFocus", sizeof("autoFocus")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_box_ce, "focusTrap", sizeof("focusTrap")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_box_ce, "showCursor", sizeof("showCursor")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "children", sizeof("children")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "key", sizeof("key")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "id", sizeof("id")-1, ZEND_ACC_PUBLIC);
    /* Per-side border colors */
    zend_declare_property_null(tui_box_ce, "borderTopColor", sizeof("borderTopColor")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "borderRightColor", sizeof("borderRightColor")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "borderBottomColor", sizeof("borderBottomColor")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "borderLeftColor", sizeof("borderLeftColor")-1, ZEND_ACC_PUBLIC);
    /* Border title */
    zend_declare_property_null(tui_box_ce, "borderTitle", sizeof("borderTitle")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_string(tui_box_ce, "borderTitlePosition", sizeof("borderTitlePosition")-1, "top-center", ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "borderTitleColor", sizeof("borderTitleColor")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "borderTitleStyle", sizeof("borderTitleStyle")-1, ZEND_ACC_PUBLIC);

    /* Register Xocdr\Tui\Ext\Text class with methods */
    INIT_CLASS_ENTRY(ce, "Xocdr\\Tui\\Ext\\Text", tui_text_methods);
    tui_text_ce = zend_register_internal_class(&ce);

    /* TuiText properties */
    zend_declare_property_string(tui_text_ce, "content", sizeof("content")-1, "", ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_text_ce, "color", sizeof("color")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_text_ce, "backgroundColor", sizeof("backgroundColor")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_text_ce, "bold", sizeof("bold")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_text_ce, "dim", sizeof("dim")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_text_ce, "italic", sizeof("italic")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_text_ce, "underline", sizeof("underline")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_text_ce, "inverse", sizeof("inverse")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_text_ce, "strikethrough", sizeof("strikethrough")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_text_ce, "wrap", sizeof("wrap")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_text_ce, "hyperlink", sizeof("hyperlink")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_text_ce, "key", sizeof("key")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_text_ce, "id", sizeof("id")-1, ZEND_ACC_PUBLIC);

    /* Register Xocdr\Tui\Ext\Instance class with methods and custom object handlers */
    INIT_CLASS_ENTRY(ce, "Xocdr\\Tui\\Ext\\Instance", tui_instance_methods);
    tui_instance_ce = zend_register_internal_class(&ce);
    tui_instance_ce->create_object = tui_instance_create_object;

    memcpy(&tui_instance_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    tui_instance_handlers.offset = XtOffsetOf(tui_instance_object, std);
    tui_instance_handlers.free_obj = tui_instance_free_object;

    /* Register Xocdr\Tui\Ext\Key class */
    INIT_CLASS_ENTRY(ce, "Xocdr\\Tui\\Ext\\Key", NULL);
    tui_key_ce = zend_register_internal_class(&ce);

    /* TuiKey properties */
    zend_declare_property_string(tui_key_ce, "key", sizeof("key")-1, "", ZEND_ACC_PUBLIC);
    zend_declare_property_string(tui_key_ce, "name", sizeof("name")-1, "", ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "upArrow", sizeof("upArrow")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "downArrow", sizeof("downArrow")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "leftArrow", sizeof("leftArrow")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "rightArrow", sizeof("rightArrow")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "return", sizeof("return")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "escape", sizeof("escape")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "backspace", sizeof("backspace")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "delete", sizeof("delete")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "tab", sizeof("tab")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "home", sizeof("home")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "end", sizeof("end")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "pageUp", sizeof("pageUp")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "pageDown", sizeof("pageDown")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_key_ce, "functionKey", sizeof("functionKey")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "ctrl", sizeof("ctrl")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "alt", sizeof("alt")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "meta", sizeof("meta")-1, 0, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_key_ce, "shift", sizeof("shift")-1, 0, ZEND_ACC_PUBLIC);

    /* Register Xocdr\Tui\Ext\FocusEvent class */
    INIT_CLASS_ENTRY(ce, "Xocdr\\Tui\\Ext\\FocusEvent", NULL);
    tui_focus_event_ce = zend_register_internal_class(&ce);

    /* TuiFocusEvent properties */
    zend_declare_property_null(tui_focus_event_ce, "previous", sizeof("previous")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_focus_event_ce, "current", sizeof("current")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_string(tui_focus_event_ce, "direction", sizeof("direction")-1, "", ZEND_ACC_PUBLIC);

    /* Register Xocdr\Tui\Ext\Focus class with custom object handlers */
    INIT_CLASS_ENTRY(ce, "Xocdr\\Tui\\Ext\\Focus", tui_focus_methods);
    tui_focus_ce = zend_register_internal_class(&ce);
    tui_focus_ce->create_object = tui_focus_create_object;

    memcpy(&tui_focus_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    tui_focus_handlers.offset = XtOffsetOf(tui_focus_object, std);
    tui_focus_handlers.free_obj = tui_focus_free_object;

    /* Focus properties */
    zend_declare_property_bool(tui_focus_ce, "isFocused", sizeof("isFocused")-1, 0, ZEND_ACC_PUBLIC);

    /* Register Xocdr\Tui\Ext\FocusManager class with custom object handlers */
    INIT_CLASS_ENTRY(ce, "Xocdr\\Tui\\Ext\\FocusManager", tui_focus_manager_methods);
    tui_focus_manager_ce = zend_register_internal_class(&ce);
    tui_focus_manager_ce->create_object = tui_focus_manager_create_object;

    memcpy(&tui_focus_manager_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    tui_focus_manager_handlers.offset = XtOffsetOf(tui_focus_manager_object, std);
    tui_focus_manager_handlers.free_obj = tui_focus_manager_free_object;

    /* Register Xocdr\Tui\Ext\StdinContext class */
    INIT_CLASS_ENTRY(ce, "Xocdr\\Tui\\Ext\\StdinContext", tui_stdin_context_methods);
    tui_stdin_context_ce = zend_register_internal_class(&ce);

    /* StdinContext properties */
    zend_declare_property_bool(tui_stdin_context_ce, "isRawModeSupported", sizeof("isRawModeSupported")-1, 1, ZEND_ACC_PUBLIC);

    /* Register Xocdr\Tui\Ext\StdoutContext class */
    INIT_CLASS_ENTRY(ce, "Xocdr\\Tui\\Ext\\StdoutContext", tui_stdout_context_methods);
    tui_stdout_context_ce = zend_register_internal_class(&ce);

    /* StdoutContext properties */
    zend_declare_property_long(tui_stdout_context_ce, "columns", sizeof("columns")-1, 80, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_stdout_context_ce, "rows", sizeof("rows")-1, 24, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_stdout_context_ce, "isTTY", sizeof("isTTY")-1, 1, ZEND_ACC_PUBLIC);

    /* Register Xocdr\Tui\Ext\StderrContext class */
    INIT_CLASS_ENTRY(ce, "Xocdr\\Tui\\Ext\\StderrContext", tui_stderr_context_methods);
    tui_stderr_context_ce = zend_register_internal_class(&ce);

    /* StderrContext properties */
    zend_declare_property_long(tui_stderr_context_ce, "columns", sizeof("columns")-1, 80, ZEND_ACC_PUBLIC);
    zend_declare_property_long(tui_stderr_context_ce, "rows", sizeof("rows")-1, 24, ZEND_ACC_PUBLIC);
    zend_declare_property_bool(tui_stderr_context_ce, "isTTY", sizeof("isTTY")-1, 1, ZEND_ACC_PUBLIC);

    /* Register Xocdr\Tui\Ext\Newline class (extends Box) */
    INIT_CLASS_ENTRY(ce, "Xocdr\\Tui\\Ext\\Newline", NULL);
    tui_newline_ce = zend_register_internal_class_ex(&ce, tui_box_ce);

    /* Newline properties */
    zend_declare_property_long(tui_newline_ce, "count", sizeof("count")-1, 1, ZEND_ACC_PUBLIC);

    /* Register Xocdr\Tui\Ext\Spacer class (extends Box) */
    INIT_CLASS_ENTRY(ce, "Xocdr\\Tui\\Ext\\Spacer", NULL);
    tui_spacer_ce = zend_register_internal_class_ex(&ce, tui_box_ce);

    /* Register Xocdr\Tui\Ext\Transform class (extends Box) */
    INIT_CLASS_ENTRY(ce, "Xocdr\\Tui\\Ext\\Transform", NULL);
    tui_transform_ce = zend_register_internal_class_ex(&ce, tui_box_ce);

    /* Transform properties */
    zend_declare_property_null(tui_transform_ce, "transform", sizeof("transform")-1, ZEND_ACC_PUBLIC);

    /* Register Xocdr\Tui\Ext\StaticOutput class (extends Box) */
    INIT_CLASS_ENTRY(ce, "Xocdr\\Tui\\Ext\\StaticOutput", NULL);
    tui_static_ce = zend_register_internal_class_ex(&ce, tui_box_ce);

    /* StaticOutput properties */
    zend_declare_property_null(tui_static_ce, "items", sizeof("items")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_static_ce, "render", sizeof("render")-1, ZEND_ACC_PUBLIC);

    /* Register Xocdr\Tui\Ext\Color enum (backed by string hex values) */
    tui_color_ce = zend_register_internal_enum("Xocdr\\Tui\\Ext\\Color", IS_STRING, tui_color_methods);

    /* Add all color cases from named_colors table (skip aliases like grey/darkgrey/lightgrey) */
    for (int i = 0; named_colors[i].name != NULL; i++) {
        /* Skip British spellings (grey variants) - prefer American spellings (gray) */
        const char *name = named_colors[i].name;
        size_t len = strlen(name);
        if (len >= 4 && strcmp(name + len - 4, "grey") == 0) {
            continue;  /* Skip all *grey variants */
        }

        /* Convert name to PascalCase */
        char pascal_name[32];
        css_name_to_pascal_case(named_colors[i].name, pascal_name, sizeof(pascal_name));

        /* Build hex value */
        char hex[8];
        int hex_len = snprintf(hex, sizeof(hex), "#%02x%02x%02x",
                 named_colors[i].r, named_colors[i].g, named_colors[i].b);
        if (hex_len < 0 || (size_t)hex_len >= sizeof(hex)) {
            continue;  /* Skip on encoding error */
        }

        /* Add enum case with interned string (persistent=1 for module-lifetime) */
        zval value;
        zend_string *str = zend_string_init_interned(hex, 7, 1);
        ZVAL_INTERNED_STR(&value, str);
        zend_enum_add_case_cstr(tui_color_ce, pascal_name, &value);
    }

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION */
static PHP_RINIT_FUNCTION(tui)
{
    /* Reset object pools between requests */
    if (TUI_G(pools)) {
        tui_pools_reset(TUI_G(pools));
    }
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
static PHP_MSHUTDOWN_FUNCTION(tui)
{
    UNREGISTER_INI_ENTRIES();

    /* Free object pools */
    if (TUI_G(pools)) {
        tui_pools_shutdown(TUI_G(pools));
        free(TUI_G(pools));
        TUI_G(pools) = NULL;
    }

    /* Free shared Yoga configuration */
    if (TUI_G(yoga_config)) {
        YGConfigFree(TUI_G(yoga_config));
        TUI_G(yoga_config) = NULL;
    }
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
static PHP_MINFO_FUNCTION(tui)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "tui support", "enabled");
    php_info_print_table_row(2, "Version", PHP_TUI_VERSION);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ tui_module_entry */
zend_module_entry tui_module_entry = {
    STANDARD_MODULE_HEADER,
    "tui",
    tui_functions,
    PHP_MINIT(tui),
    PHP_MSHUTDOWN(tui),
    PHP_RINIT(tui),
    NULL,  /* RSHUTDOWN */
    PHP_MINFO(tui),
    PHP_TUI_VERSION,
    PHP_MODULE_GLOBALS(tui),
    PHP_GINIT(tui),
    NULL,  /* GSHUTDOWN */
    NULL,  /* PRSHUTDOWN */
    STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_TUI
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(tui)
#endif
