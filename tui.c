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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_exceptions.h"
#include "zend_enum.h"
#include "php_tui.h"

#include "src/text/measure.h"
#include "src/text/wrap.h"
#include "src/app/app.h"
#include "src/node/node.h"
#include "src/terminal/terminal.h"
#include "src/terminal/ansi.h"
#include "src/event/input.h"
#include "src/drawing/primitives.h"
#include "src/drawing/canvas.h"
#include "src/drawing/animation.h"
#include "src/drawing/table.h"
#include "src/drawing/progress.h"
#include "src/drawing/sprite.h"
#include "src/render/buffer.h"
#include "src/testing/renderer.h"
#include "src/testing/query.h"
#include "src/pool/pool.h"

#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <math.h>

ZEND_DECLARE_MODULE_GLOBALS(tui)

/* Resource type IDs */
static int le_tui_canvas;
static int le_tui_table;
static int le_tui_sprite;
static int le_tui_buffer;
static int le_tui_test_renderer;
static int le_tui_history;

/* Maximum depth for recursive tree operations - matches app.c */
#define MAX_TREE_DEPTH 256

#define TUI_CANVAS_RES_NAME "TuiCanvas"
#define TUI_TABLE_RES_NAME "TuiTable"
#define TUI_SPRITE_RES_NAME "TuiSprite"
#define TUI_BUFFER_RES_NAME "TuiBuffer"
#define TUI_TEST_RENDERER_RES_NAME "TuiTestRenderer"
#define TUI_HISTORY_RES_NAME "TuiHistory"

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

/* Object handlers */
static zend_object_handlers tui_instance_handlers;
static zend_object_handlers tui_focus_handlers;
static zend_object_handlers tui_focus_manager_handlers;

/* ------------------------------------------------------------------
 * TuiInstance custom object structure
 * ------------------------------------------------------------------ */
typedef struct {
    tui_app *app;
    zend_object std;
} tui_instance_object;

static inline tui_instance_object *tui_instance_from_obj(zend_object *obj) {
    return (tui_instance_object *)((char *)(obj) - XtOffsetOf(tui_instance_object, std));
}

#define Z_TUI_INSTANCE_P(zv) tui_instance_from_obj(Z_OBJ_P(zv))

static zend_object *tui_instance_create_object(zend_class_entry *ce)
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
 * TuiFocus custom object structure
 * ------------------------------------------------------------------ */
typedef struct {
    tui_app *app;
    zend_object std;
} tui_focus_object;

static inline tui_focus_object *tui_focus_from_obj(zend_object *obj) {
    return (tui_focus_object *)((char *)(obj) - XtOffsetOf(tui_focus_object, std));
}

#define Z_TUI_FOCUS_P(zv) tui_focus_from_obj(Z_OBJ_P(zv))

static zend_object *tui_focus_create_object(zend_class_entry *ce)
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
 * TuiFocusManager custom object structure
 * ------------------------------------------------------------------ */
typedef struct {
    tui_app *app;
    zend_object std;
} tui_focus_manager_object;

static inline tui_focus_manager_object *tui_focus_manager_from_obj(zend_object *obj) {
    return (tui_focus_manager_object *)((char *)(obj) - XtOffsetOf(tui_focus_manager_object, std));
}

#define Z_TUI_FOCUS_MANAGER_P(zv) tui_focus_manager_from_obj(Z_OBJ_P(zv))

static zend_object *tui_focus_manager_create_object(zend_class_entry *ce)
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
 * CSS Named Colors lookup table (all 147 CSS colors)
 * ------------------------------------------------------------------ */
static const struct {
    const char *name;
    uint8_t r, g, b;
} named_colors[] = {
    /* Basic colors (19) */
    {"black", 0, 0, 0},
    {"white", 255, 255, 255},
    {"red", 255, 0, 0},
    {"green", 0, 128, 0},
    {"blue", 0, 0, 255},
    {"yellow", 255, 255, 0},
    {"cyan", 0, 255, 255},
    {"magenta", 255, 0, 255},
    {"aqua", 0, 255, 255},
    {"fuchsia", 255, 0, 255},
    {"lime", 0, 255, 0},
    {"maroon", 128, 0, 0},
    {"navy", 0, 0, 128},
    {"olive", 128, 128, 0},
    {"purple", 128, 0, 128},
    {"silver", 192, 192, 192},
    {"teal", 0, 128, 128},
    {"gray", 128, 128, 128},
    {"grey", 128, 128, 128},

    /* Reds (8) */
    {"indianred", 205, 92, 92},
    {"lightcoral", 240, 128, 128},
    {"salmon", 250, 128, 114},
    {"darksalmon", 233, 150, 122},
    {"lightsalmon", 255, 160, 122},
    {"crimson", 220, 20, 60},
    {"firebrick", 178, 34, 34},
    {"darkred", 139, 0, 0},

    /* Pinks (6) */
    {"pink", 255, 192, 203},
    {"lightpink", 255, 182, 193},
    {"hotpink", 255, 105, 180},
    {"deeppink", 255, 20, 147},
    {"mediumvioletred", 199, 21, 133},
    {"palevioletred", 219, 112, 147},

    /* Oranges (5) */
    {"orange", 255, 165, 0},
    {"darkorange", 255, 140, 0},
    {"orangered", 255, 69, 0},
    {"tomato", 255, 99, 71},
    {"coral", 255, 127, 80},

    /* Yellows (10) */
    {"gold", 255, 215, 0},
    {"lightyellow", 255, 255, 224},
    {"lemonchiffon", 255, 250, 205},
    {"lightgoldenrodyellow", 250, 250, 210},
    {"papayawhip", 255, 239, 213},
    {"moccasin", 255, 228, 181},
    {"peachpuff", 255, 218, 185},
    {"palegoldenrod", 238, 232, 170},
    {"khaki", 240, 230, 140},
    {"darkkhaki", 189, 183, 107},

    /* Purples (16) */
    {"lavender", 230, 230, 250},
    {"thistle", 216, 191, 216},
    {"plum", 221, 160, 221},
    {"violet", 238, 130, 238},
    {"orchid", 218, 112, 214},
    {"mediumorchid", 186, 85, 211},
    {"mediumpurple", 147, 112, 219},
    {"rebeccapurple", 102, 51, 153},
    {"blueviolet", 138, 43, 226},
    {"darkviolet", 148, 0, 211},
    {"darkorchid", 153, 50, 204},
    {"darkmagenta", 139, 0, 139},
    {"indigo", 75, 0, 130},
    {"slateblue", 106, 90, 205},
    {"darkslateblue", 72, 61, 139},
    {"mediumslateblue", 123, 104, 238},

    /* Greens (19) */
    {"greenyellow", 173, 255, 47},
    {"chartreuse", 127, 255, 0},
    {"lawngreen", 124, 252, 0},
    {"limegreen", 50, 205, 50},
    {"palegreen", 152, 251, 152},
    {"lightgreen", 144, 238, 144},
    {"mediumspringgreen", 0, 250, 154},
    {"springgreen", 0, 255, 127},
    {"mediumseagreen", 60, 179, 113},
    {"seagreen", 46, 139, 87},
    {"forestgreen", 34, 139, 34},
    {"darkgreen", 0, 100, 0},
    {"yellowgreen", 154, 205, 50},
    {"olivedrab", 107, 142, 35},
    {"darkolivegreen", 85, 107, 47},
    {"mediumaquamarine", 102, 205, 170},
    {"darkseagreen", 143, 188, 143},
    {"lightseagreen", 32, 178, 170},
    {"darkcyan", 0, 139, 139},

    /* Blues/Cyans (20) */
    {"lightcyan", 224, 255, 255},
    {"paleturquoise", 175, 238, 238},
    {"aquamarine", 127, 255, 212},
    {"turquoise", 64, 224, 208},
    {"mediumturquoise", 72, 209, 204},
    {"darkturquoise", 0, 206, 209},
    {"cadetblue", 95, 158, 160},
    {"steelblue", 70, 130, 180},
    {"lightsteelblue", 176, 196, 222},
    {"powderblue", 176, 224, 230},
    {"lightblue", 173, 216, 230},
    {"skyblue", 135, 206, 235},
    {"lightskyblue", 135, 206, 250},
    {"deepskyblue", 0, 191, 255},
    {"dodgerblue", 30, 144, 255},
    {"cornflowerblue", 100, 149, 237},
    {"royalblue", 65, 105, 225},
    {"mediumblue", 0, 0, 205},
    {"darkblue", 0, 0, 139},
    {"midnightblue", 25, 25, 112},

    /* Browns (16) */
    {"cornsilk", 255, 248, 220},
    {"blanchedalmond", 255, 235, 205},
    {"bisque", 255, 228, 196},
    {"navajowhite", 255, 222, 173},
    {"wheat", 245, 222, 179},
    {"burlywood", 222, 184, 135},
    {"tan", 210, 180, 140},
    {"rosybrown", 188, 143, 143},
    {"sandybrown", 244, 164, 96},
    {"goldenrod", 218, 165, 32},
    {"darkgoldenrod", 184, 134, 11},
    {"peru", 205, 133, 63},
    {"chocolate", 210, 105, 30},
    {"saddlebrown", 139, 69, 19},
    {"sienna", 160, 82, 45},
    {"brown", 165, 42, 42},

    /* Whites (16) */
    {"snow", 255, 250, 250},
    {"honeydew", 240, 255, 240},
    {"mintcream", 245, 255, 250},
    {"azure", 240, 255, 255},
    {"aliceblue", 240, 248, 255},
    {"ghostwhite", 248, 248, 255},
    {"whitesmoke", 245, 245, 245},
    {"seashell", 255, 245, 238},
    {"beige", 245, 245, 220},
    {"oldlace", 253, 245, 230},
    {"floralwhite", 255, 250, 240},
    {"ivory", 255, 255, 240},
    {"antiquewhite", 250, 235, 215},
    {"linen", 250, 240, 230},
    {"lavenderblush", 255, 240, 245},
    {"mistyrose", 255, 228, 225},

    /* Grays (12 - includes grey variants) */
    {"gainsboro", 220, 220, 220},
    {"lightgray", 211, 211, 211},
    {"lightgrey", 211, 211, 211},
    {"darkgray", 169, 169, 169},
    {"darkgrey", 169, 169, 169},
    {"dimgray", 105, 105, 105},
    {"dimgrey", 105, 105, 105},
    {"lightslategray", 119, 136, 153},
    {"lightslategrey", 119, 136, 153},
    {"slategray", 112, 128, 144},
    {"slategrey", 112, 128, 144},
    {"darkslategray", 47, 79, 79},
    {"darkslategrey", 47, 79, 79},

    /* Sentinel */
    {NULL, 0, 0, 0}
};

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
static tui_node* php_to_tui_node_impl(zval *obj, int depth);

/* Public wrapper that starts recursion at depth 0 */
static tui_node* php_to_tui_node(zval *obj)
{
    return php_to_tui_node_impl(obj, 0);
}

static tui_node* php_to_tui_node_impl(zval *obj, int depth)
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

    if (ce == tui_box_ce) {
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
                tui_node *child_node = php_to_tui_node_impl(child, depth + 1);
                if (child_node) {
                    if (tui_node_append_child(node, child_node) < 0) {
                        /* Failed to append - destroy the orphan child to prevent leak */
                        tui_node_destroy(child_node);
                    }
                }
            } ZEND_HASH_FOREACH_END();
        }

    } else if (ce == tui_text_ce) {
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
        tui_app_render(obj->app);
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
        RETURN_NULL();
    }

    tui_app *app = obj->app;
    int is_new = 0;
    int index = tui_app_get_or_create_state_slot(app, initial, &is_new);

    if (index < 0) {
        RETURN_NULL();
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
        RETURN_NULL();
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
        RETURN_NULL();
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
        RETURN_NULL();
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
        RETURN_NULL();
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
        RETURN_NULL();
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
        RETURN_NULL();
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
        RETURN_NULL();
    }

    array_init(return_value);
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
    if (!obj->app || index < 0 || index >= obj->app->state_count) {
        RETURN_NULL();
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
        RETURN_NULL();
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
    if (obj->app) {
        tui_app_set_input_handler(obj->app, &fci, &fcc);
    }
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
    if (obj->app) {
        tui_app_set_focus_handler(obj->app, &fci, &fcc);
    }
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
    if (obj->app) {
        tui_app_set_resize_handler(obj->app, &fci, &fcc);
    }
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
    if (obj->app) {
        tui_app_set_tick_handler(obj->app, &fci, &fcc);
    }
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
        RETURN_LONG(-1);
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
    if (obj->app) {
        tui_app_remove_timer(obj->app, (int)timer_id);
    }
}
/* }}} */

/* {{{ TuiInstance::clear(): void */
PHP_METHOD(TuiInstance, clear)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(ZEND_THIS);
    if (obj->app && obj->app->buffer) {
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

/* ------------------------------------------------------------------
 * PHP Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_get_terminal_size(): array */
PHP_FUNCTION(tui_get_terminal_size)
{
    struct winsize ws;
    int width = 80;   /* Default fallback */
    int height = 24;

    ZEND_PARSE_PARAMETERS_NONE();

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        /* Validate returned values before using */
        if (ws.ws_col > 0 && ws.ws_col <= 1000) {
            width = ws.ws_col;
        }
        if (ws.ws_row > 0 && ws.ws_row <= 1000) {
            height = ws.ws_row;
        }
    }

    TUI_G(terminal_width) = width;
    TUI_G(terminal_height) = height;

    array_init(return_value);
    add_next_index_long(return_value, width);
    add_next_index_long(return_value, height);
}
/* }}} */

/* {{{ tui_is_interactive(): bool */
PHP_FUNCTION(tui_is_interactive)
{
    ZEND_PARSE_PARAMETERS_NONE();

    RETURN_BOOL(isatty(STDIN_FILENO) && isatty(STDOUT_FILENO));
}
/* }}} */

/* {{{ tui_is_ci(): bool */
PHP_FUNCTION(tui_is_ci)
{
    ZEND_PARSE_PARAMETERS_NONE();

    /* Check common CI environment variables */
    if (getenv("CI") != NULL ||
        getenv("GITHUB_ACTIONS") != NULL ||
        getenv("GITLAB_CI") != NULL ||
        getenv("CIRCLECI") != NULL ||
        getenv("TRAVIS") != NULL ||
        getenv("JENKINS_URL") != NULL) {
        RETURN_TRUE;
    }

    RETURN_FALSE;
}
/* }}} */

/* ------------------------------------------------------------------
 * Terminal Mode Functions (Mouse, Bracketed Paste)
 * ------------------------------------------------------------------ */

/* {{{ tui_mouse_enable(int $mode = TUI_MOUSE_MODE_BUTTON): bool */
PHP_FUNCTION(tui_mouse_enable)
{
    zend_long mode = TUI_MOUSE_MODE_BUTTON;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(mode)
    ZEND_PARSE_PARAMETERS_END();

    if (mode < TUI_MOUSE_MODE_OFF || mode > TUI_MOUSE_MODE_ALL) {
        zend_throw_exception(zend_ce_exception, "Invalid mouse mode", 0);
        RETURN_THROWS();
    }

    int result = tui_terminal_enable_mouse((tui_mouse_mode)mode);
    RETURN_BOOL(result == 0);
}
/* }}} */

/* {{{ tui_mouse_disable(): bool */
PHP_FUNCTION(tui_mouse_disable)
{
    ZEND_PARSE_PARAMETERS_NONE();

    int result = tui_terminal_disable_mouse();
    RETURN_BOOL(result == 0);
}
/* }}} */

/* {{{ tui_mouse_get_mode(): int */
PHP_FUNCTION(tui_mouse_get_mode)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_mouse_mode mode = tui_terminal_get_mouse_mode();
    RETURN_LONG((zend_long)mode);
}
/* }}} */

/* {{{ tui_bracketed_paste_enable(): bool */
PHP_FUNCTION(tui_bracketed_paste_enable)
{
    ZEND_PARSE_PARAMETERS_NONE();

    int result = tui_terminal_enable_bracketed_paste();
    RETURN_BOOL(result == 0);
}
/* }}} */

/* {{{ tui_bracketed_paste_disable(): bool */
PHP_FUNCTION(tui_bracketed_paste_disable)
{
    ZEND_PARSE_PARAMETERS_NONE();

    int result = tui_terminal_disable_bracketed_paste();
    RETURN_BOOL(result == 0);
}
/* }}} */

/* {{{ tui_bracketed_paste_is_enabled(): bool */
PHP_FUNCTION(tui_bracketed_paste_is_enabled)
{
    ZEND_PARSE_PARAMETERS_NONE();

    int result = tui_terminal_is_bracketed_paste_enabled();
    RETURN_BOOL(result != 0);
}
/* }}} */

/* ------------------------------------------------------------------
 * Clipboard Functions (OSC 52)
 * ------------------------------------------------------------------ */

/* {{{ tui_clipboard_copy(string $text, string $target = 'clipboard'): bool */
PHP_FUNCTION(tui_clipboard_copy)
{
    zend_string *text;
    zend_string *target = NULL;
    tui_clipboard_target clipboard_target = TUI_CLIPBOARD_CLIPBOARD;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_STR(text)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(target)
    ZEND_PARSE_PARAMETERS_END();

    if (target != NULL) {
        if (zend_string_equals_literal(target, "primary")) {
            clipboard_target = TUI_CLIPBOARD_PRIMARY;
        } else if (zend_string_equals_literal(target, "secondary")) {
            clipboard_target = TUI_CLIPBOARD_SECONDARY;
        } else if (!zend_string_equals_literal(target, "clipboard")) {
            zend_throw_exception(zend_ce_exception,
                "Invalid clipboard target. Use 'clipboard', 'primary', or 'secondary'", 0);
            RETURN_THROWS();
        }
    }

    /* Allocate buffer for base64-encoded output */
    size_t buf_size = tui_base64_encode_len(ZSTR_LEN(text)) + 32;
    char *buf = emalloc(buf_size);

    int len = tui_ansi_clipboard_write(buf, buf_size, ZSTR_VAL(text), ZSTR_LEN(text), clipboard_target);
    if (len < 0) {
        efree(buf);
        RETURN_FALSE;
    }

    /* Write to stdout */
    ssize_t written = write(STDOUT_FILENO, buf, (size_t)len);
    efree(buf);

    RETURN_BOOL(written == len);
}
/* }}} */

/* {{{ tui_clipboard_request(string $target = 'clipboard'): void */
PHP_FUNCTION(tui_clipboard_request)
{
    zend_string *target = NULL;
    tui_clipboard_target clipboard_target = TUI_CLIPBOARD_CLIPBOARD;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(target)
    ZEND_PARSE_PARAMETERS_END();

    if (target != NULL) {
        if (zend_string_equals_literal(target, "primary")) {
            clipboard_target = TUI_CLIPBOARD_PRIMARY;
        } else if (zend_string_equals_literal(target, "secondary")) {
            clipboard_target = TUI_CLIPBOARD_SECONDARY;
        } else if (!zend_string_equals_literal(target, "clipboard")) {
            zend_throw_exception(zend_ce_exception,
                "Invalid clipboard target. Use 'clipboard', 'primary', or 'secondary'", 0);
            RETURN_THROWS();
        }
    }

    char buf[32];
    size_t len;
    tui_ansi_clipboard_request(buf, &len, clipboard_target);
    write(STDOUT_FILENO, buf, len);
}
/* }}} */

/* {{{ tui_clipboard_clear(string $target = 'clipboard'): void */
PHP_FUNCTION(tui_clipboard_clear)
{
    zend_string *target = NULL;
    tui_clipboard_target clipboard_target = TUI_CLIPBOARD_CLIPBOARD;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(target)
    ZEND_PARSE_PARAMETERS_END();

    if (target != NULL) {
        if (zend_string_equals_literal(target, "primary")) {
            clipboard_target = TUI_CLIPBOARD_PRIMARY;
        } else if (zend_string_equals_literal(target, "secondary")) {
            clipboard_target = TUI_CLIPBOARD_SECONDARY;
        } else if (!zend_string_equals_literal(target, "clipboard")) {
            zend_throw_exception(zend_ce_exception,
                "Invalid clipboard target. Use 'clipboard', 'primary', or 'secondary'", 0);
            RETURN_THROWS();
        }
    }

    char buf[32];
    size_t len;
    tui_ansi_clipboard_clear(buf, &len, clipboard_target);
    write(STDOUT_FILENO, buf, len);
}
/* }}} */

/* ------------------------------------------------------------------
 * Input History Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_history_create(int $maxEntries = 1000): resource */
PHP_FUNCTION(tui_history_create)
{
    zend_long max_entries = 1000;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(max_entries)
    ZEND_PARSE_PARAMETERS_END();

    if (max_entries < 1 || max_entries > 100000) {
        zend_throw_exception(zend_ce_exception, "maxEntries must be between 1 and 100000", 0);
        RETURN_THROWS();
    }

    tui_input_history *history = tui_history_create((int)max_entries);
    if (!history) {
        RETURN_NULL();
    }

    RETURN_RES(zend_register_resource(history, le_tui_history));
}
/* }}} */

/* {{{ tui_history_destroy(resource $history): void */
PHP_FUNCTION(tui_history_destroy)
{
    zval *zhistory;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zhistory)
    ZEND_PARSE_PARAMETERS_END();

    /* Manually delete the resource - this will call the destructor */
    zend_list_close(Z_RES_P(zhistory));
}
/* }}} */

/* {{{ tui_history_add(resource $history, string $entry): void */
PHP_FUNCTION(tui_history_add)
{
    zval *zhistory;
    zend_string *entry;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zhistory)
        Z_PARAM_STR(entry)
    ZEND_PARSE_PARAMETERS_END();

    tui_input_history *history = (tui_input_history *)zend_fetch_resource(
        Z_RES_P(zhistory), TUI_HISTORY_RES_NAME, le_tui_history);
    if (!history) {
        RETURN_THROWS();
    }

    tui_history_add(history, ZSTR_VAL(entry));
}
/* }}} */

/* {{{ tui_history_prev(resource $history): ?string */
PHP_FUNCTION(tui_history_prev)
{
    zval *zhistory;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zhistory)
    ZEND_PARSE_PARAMETERS_END();

    tui_input_history *history = (tui_input_history *)zend_fetch_resource(
        Z_RES_P(zhistory), TUI_HISTORY_RES_NAME, le_tui_history);
    if (!history) {
        RETURN_THROWS();
    }

    const char *prev = tui_history_prev(history);
    if (prev) {
        RETURN_STRING(prev);
    }
    RETURN_NULL();
}
/* }}} */

/* {{{ tui_history_next(resource $history): ?string */
PHP_FUNCTION(tui_history_next)
{
    zval *zhistory;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zhistory)
    ZEND_PARSE_PARAMETERS_END();

    tui_input_history *history = (tui_input_history *)zend_fetch_resource(
        Z_RES_P(zhistory), TUI_HISTORY_RES_NAME, le_tui_history);
    if (!history) {
        RETURN_THROWS();
    }

    const char *next = tui_history_next(history);
    if (next) {
        RETURN_STRING(next);
    }
    RETURN_NULL();
}
/* }}} */

/* {{{ tui_history_reset(resource $history): void */
PHP_FUNCTION(tui_history_reset)
{
    zval *zhistory;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zhistory)
    ZEND_PARSE_PARAMETERS_END();

    tui_input_history *history = (tui_input_history *)zend_fetch_resource(
        Z_RES_P(zhistory), TUI_HISTORY_RES_NAME, le_tui_history);
    if (!history) {
        RETURN_THROWS();
    }

    tui_history_reset_position(history);
}
/* }}} */

/* {{{ tui_history_save_temp(resource $history, string $input): void */
PHP_FUNCTION(tui_history_save_temp)
{
    zval *zhistory;
    zend_string *input;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zhistory)
        Z_PARAM_STR(input)
    ZEND_PARSE_PARAMETERS_END();

    tui_input_history *history = (tui_input_history *)zend_fetch_resource(
        Z_RES_P(zhistory), TUI_HISTORY_RES_NAME, le_tui_history);
    if (!history) {
        RETURN_THROWS();
    }

    tui_history_save_temp(history, ZSTR_VAL(input));
}
/* }}} */

/* {{{ tui_history_get_temp(resource $history): ?string */
PHP_FUNCTION(tui_history_get_temp)
{
    zval *zhistory;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zhistory)
    ZEND_PARSE_PARAMETERS_END();

    tui_input_history *history = (tui_input_history *)zend_fetch_resource(
        Z_RES_P(zhistory), TUI_HISTORY_RES_NAME, le_tui_history);
    if (!history) {
        RETURN_THROWS();
    }

    const char *temp = tui_history_get_temp(history);
    if (temp) {
        RETURN_STRING(temp);
    }
    RETURN_NULL();
}
/* }}} */

/* {{{ tui_string_width(string $text): int */
PHP_FUNCTION(tui_string_width)
{
    zend_string *text;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();

    /* Use the C implementation from text/measure.c */
    int width = tui_string_width_n(ZSTR_VAL(text), (int)ZSTR_LEN(text));

    RETURN_LONG(width);
}
/* }}} */

/* {{{ tui_wrap_text(string $text, int $width): array */
PHP_FUNCTION(tui_wrap_text)
{
    zend_string *text;
    zend_long width;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(text)
        Z_PARAM_LONG(width)
    ZEND_PARSE_PARAMETERS_END();

    array_init(return_value);

    /* Use the C implementation from text/wrap.c
     * TUI_WRAP_WORD_CHAR: prefer word breaks but break long words at character boundary */
    tui_wrapped_text *wrapped = tui_wrap_text(ZSTR_VAL(text), (int)width, TUI_WRAP_WORD_CHAR);

    if (wrapped) {
        for (int i = 0; i < wrapped->count; i++) {
            add_next_index_string(return_value, wrapped->lines[i]);
        }
        tui_wrapped_text_free(wrapped);
    }
}
/* }}} */

/* {{{ tui_truncate(string $text, int $width, string $ellipsis = '...', string $position = 'end'): string */
PHP_FUNCTION(tui_truncate)
{
    zend_string *text;
    zend_long width;
    zend_string *ellipsis = NULL;
    zend_string *position = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 4)
        Z_PARAM_STR(text)
        Z_PARAM_LONG(width)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(ellipsis)
        Z_PARAM_STR(position)
    ZEND_PARSE_PARAMETERS_END();

    const char *ellipsis_str = ellipsis ? ZSTR_VAL(ellipsis) : "...";

    /* Parse position mode */
    tui_truncate_position pos = TUI_TRUNCATE_END;
    if (position) {
        const char *pos_str = ZSTR_VAL(position);
        if (strcmp(pos_str, "start") == 0) {
            pos = TUI_TRUNCATE_START;
        } else if (strcmp(pos_str, "middle") == 0) {
            pos = TUI_TRUNCATE_MIDDLE;
        }
        /* "end" is default */
    }

    /* Use the C implementation from text/wrap.c */
    char *result = tui_truncate_text_ex(ZSTR_VAL(text), (int)width, ellipsis_str, pos);

    if (result) {
        RETVAL_STRING(result);
        free(result);
    } else {
        RETURN_EMPTY_STRING();
    }
}
/* }}} */

/* {{{ tui_strip_ansi(string $text): string */
PHP_FUNCTION(tui_strip_ansi)
{
    zend_string *text;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();

    char *result = tui_strip_ansi(ZSTR_VAL(text));

    if (result) {
        RETVAL_STRING(result);
        free(result);
    } else {
        RETURN_EMPTY_STRING();
    }
}
/* }}} */

/* {{{ tui_string_width_ansi(string $text): int */
PHP_FUNCTION(tui_string_width_ansi)
{
    zend_string *text;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();

    int width = tui_string_width_ansi(ZSTR_VAL(text));

    RETURN_LONG(width);
}
/* }}} */

/* {{{ tui_slice_ansi(string $text, int $start, int $end): string */
PHP_FUNCTION(tui_slice_ansi)
{
    zend_string *text;
    zend_long start;
    zend_long end;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_STR(text)
        Z_PARAM_LONG(start)
        Z_PARAM_LONG(end)
    ZEND_PARSE_PARAMETERS_END();

    char *result = tui_slice_ansi(ZSTR_VAL(text), (int)start, (int)end);

    if (result) {
        RETVAL_STRING(result);
        free(result);
    } else {
        RETURN_EMPTY_STRING();
    }
}
/* }}} */

/* Render callback wrapper - calls PHP component and converts result to C nodes */
static void render_component_callback(tui_app *app)
{
    if (!app || !app->instance_zval) return;

    zval retval;

    /* Reset state index before each render (for useState hook ordering) */
    tui_app_reset_state_index(app);

    /* Set up parameters - pass Instance as first argument */
    zval params[1];
    ZVAL_COPY(&params[0], app->instance_zval);

    /* Save original param count and set up for our call */
    zend_fcall_info fci_copy = app->component_fci;
    fci_copy.param_count = 1;
    fci_copy.params = params;
    fci_copy.retval = &retval;

    /*
     * Console interception: Start output buffering before calling the component.
     * This prevents any stray echo/print/var_dump from corrupting the TUI.
     * The captured output is stored in app->captured_output for later retrieval.
     */
    php_output_start_default();

    /* Call the PHP component function with Instance parameter */
    if (zend_call_function(&fci_copy, &app->component_fcc) == SUCCESS) {
        /* Convert PHP object tree to C node tree */
        if (Z_TYPE(retval) == IS_OBJECT) {
            /* Free old tree */
            if (app->root_node) {
                tui_node_destroy(app->root_node);
            }

            /* Build new tree */
            app->root_node = php_to_tui_node(&retval);
        }

        zval_ptr_dtor(&retval);
    }

    /* Capture any output that occurred during render */
    {
        zval captured;
        ZVAL_UNDEF(&captured);
        php_output_get_contents(&captured);

        /* Free previous captured output */
        if (app->captured_output) {
            efree(app->captured_output);
            app->captured_output = NULL;
            app->captured_output_len = 0;
        }

        /* Store new captured output (if any) */
        if (Z_TYPE(captured) == IS_STRING && Z_STRLEN(captured) > 0) {
            app->captured_output = estrndup(Z_STRVAL(captured), Z_STRLEN(captured));
            app->captured_output_len = Z_STRLEN(captured);
        }

        zval_ptr_dtor(&captured);
    }

    /* Discard the buffer (don't flush to stdout) */
    php_output_discard();

    /* Clean up the copied zval reference */
    zval_ptr_dtor(&params[0]);
}

/* {{{ tui_render(callable $component, array $options = []): TuiInstance */
PHP_FUNCTION(tui_render)
{
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
    zval *options = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_FUNC(fci, fcc)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY(options)
    ZEND_PARSE_PARAMETERS_END();

    /* Create app instance */
    tui_app *app = tui_app_create();
    if (!app) {
        php_error_docref(NULL, E_ERROR, "Failed to create TUI application");
        RETURN_NULL();
    }

    /* Parse options */
    if (options) {
        zval *opt;
        if ((opt = zend_hash_str_find(Z_ARRVAL_P(options), "fullscreen", sizeof("fullscreen")-1)) != NULL) {
            tui_app_set_fullscreen(app, zend_is_true(opt));
        }
        if ((opt = zend_hash_str_find(Z_ARRVAL_P(options), "exitOnCtrlC", sizeof("exitOnCtrlC")-1)) != NULL) {
            tui_app_set_exit_on_ctrl_c(app, zend_is_true(opt));
        }
    }

    /* Set component callback */
    tui_app_set_component(app, &fci, &fcc);

    /* Set rerender callback for auto-rerender on resize */
    app->rerender_callback = render_component_callback;

    /* Create Instance object FIRST - same object passed to callback and returned */
    object_init_ex(return_value, tui_instance_ce);
    tui_instance_object *instance_obj = Z_TUI_INSTANCE_P(return_value);
    instance_obj->app = app;

    /* Store reference to Instance zval in app for render_component_callback */
    app->instance_zval = return_value;

    /* Initial render - call component to get tree with Instance parameter */
    zval retval;

    /* Reset state index for initial render */
    tui_app_reset_state_index(app);

    /* Set up parameters - pass the same Instance we're returning */
    zval params[1];
    ZVAL_COPY(&params[0], return_value);

    zend_fcall_info fci_copy = fci;
    fci_copy.param_count = 1;
    fci_copy.params = params;
    fci_copy.retval = &retval;

    if (zend_call_function(&fci_copy, &fcc) == SUCCESS) {
        if (Z_TYPE(retval) == IS_OBJECT) {
            if (!instanceof_function(Z_OBJCE(retval), tui_box_ce) &&
                !instanceof_function(Z_OBJCE(retval), tui_text_ce)) {
                zval_ptr_dtor(&retval);
                zval_ptr_dtor(&params[0]);
                app->instance_zval = NULL;
                instance_obj->app = NULL;
                tui_app_destroy(app);
                zend_throw_exception(zend_ce_exception,
                    "Component must return Box or Text, got other object", 0);
                RETURN_THROWS();
            }
            app->root_node = php_to_tui_node(&retval);
        } else if (Z_TYPE(retval) != IS_NULL) {
            zval_ptr_dtor(&retval);
            zval_ptr_dtor(&params[0]);
            app->instance_zval = NULL;
            instance_obj->app = NULL;
            tui_app_destroy(app);
            zend_throw_exception(zend_ce_exception,
                "Component must return Box, Text, or null", 0);
            RETURN_THROWS();
        }
        zval_ptr_dtor(&retval);
    } else {
        zval_ptr_dtor(&params[0]);
        app->instance_zval = NULL;
        instance_obj->app = NULL;
        tui_app_destroy(app);
        zend_throw_exception(zend_ce_exception,
            "Component callback failed to execute", 0);
        RETURN_THROWS();
    }

    /* Clean up the copied param reference */
    zval_ptr_dtor(&params[0]);

    /* Start the app */
    if (tui_app_start(app) != 0) {
        app->instance_zval = NULL;
        instance_obj->app = NULL;
        tui_app_destroy(app);
        php_error_docref(NULL, E_ERROR, "Failed to start TUI application");
        RETURN_NULL();
    }

    /* return_value already contains the Instance object */
}
/* }}} */

/* Helper to get app from TuiInstance */
static tui_app* get_app_from_instance(zval *instance)
{
    tui_instance_object *obj = Z_TUI_INSTANCE_P(instance);
    return obj->app;
}

/* {{{ tui_rerender(TuiInstance $instance): void */
PHP_FUNCTION(tui_rerender)
{
    zval *instance;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        render_component_callback(app);  /* Call PHP component, build node tree */
        tui_app_render_tree(app);        /* Render tree to screen (no double component call) */
    }
}
/* }}} */

/* {{{ tui_unmount(TuiInstance $instance): void */
PHP_FUNCTION(tui_unmount)
{
    zval *instance;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
    ZEND_PARSE_PARAMETERS_END();

    tui_instance_object *obj = Z_TUI_INSTANCE_P(instance);
    if (obj->app) {
        tui_app_stop(obj->app);
        tui_app_destroy(obj->app);
        /* Clear the app pointer in the C struct */
        obj->app = NULL;
    }
}
/* }}} */

/* {{{ tui_wait_until_exit(TuiInstance $instance): void */
PHP_FUNCTION(tui_wait_until_exit)
{
    zval *instance;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_wait_until_exit(app);
    }
}
/* }}} */

/* {{{ tui_set_input_handler(TuiInstance $instance, callable $handler): void */
PHP_FUNCTION(tui_set_input_handler)
{
    zval *instance;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_set_input_handler(app, &fci, &fcc);
    }
}
/* }}} */

/* {{{ tui_set_focus_handler(TuiInstance $instance, callable $handler): void */
PHP_FUNCTION(tui_set_focus_handler)
{
    zval *instance;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_set_focus_handler(app, &fci, &fcc);
    }
}
/* }}} */

/* {{{ tui_focus_next(TuiInstance $instance): void */
PHP_FUNCTION(tui_focus_next)
{
    zval *instance;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_focus_next(app);
    }
}
/* }}} */

/* {{{ tui_focus_prev(TuiInstance $instance): void */
PHP_FUNCTION(tui_focus_prev)
{
    zval *instance;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_focus_prev(app);
    }
}
/* }}} */

/* {{{ tui_set_resize_handler(TuiInstance $instance, callable $handler): void */
PHP_FUNCTION(tui_set_resize_handler)
{
    zval *instance;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_set_resize_handler(app, &fci, &fcc);
    }
}
/* }}} */

/* {{{ tui_get_size(TuiInstance $instance): array */
PHP_FUNCTION(tui_get_size)
{
    zval *instance;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        array_init(return_value);
        add_assoc_long(return_value, "width", app->width);
        add_assoc_long(return_value, "height", app->height);
        add_assoc_long(return_value, "columns", app->width);
        add_assoc_long(return_value, "rows", app->height);
    } else {
        RETURN_NULL();
    }
}
/* }}} */

/* {{{ tui_get_focused_node(TuiInstance $instance): ?array */
PHP_FUNCTION(tui_get_focused_node)
{
    zval *instance;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app && app->focused_node) {
        array_init(return_value);
        add_assoc_bool(return_value, "focusable", app->focused_node->focusable);
        add_assoc_bool(return_value, "focused", app->focused_node->focused);
        add_assoc_long(return_value, "x", (zend_long)app->focused_node->x);
        add_assoc_long(return_value, "y", (zend_long)app->focused_node->y);
        add_assoc_long(return_value, "width", (zend_long)app->focused_node->width);
        add_assoc_long(return_value, "height", (zend_long)app->focused_node->height);
        if (app->focused_node->type == TUI_NODE_TEXT && app->focused_node->text) {
            add_assoc_string(return_value, "type", "text");
            add_assoc_string(return_value, "content", app->focused_node->text);
        } else {
            add_assoc_string(return_value, "type", "box");
        }
    } else {
        RETURN_NULL();
    }
}
/* }}} */

/* ------------------------------------------------------------------
 * Timer Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_add_timer(TuiInstance $instance, int $interval_ms, callable $callback): int */
PHP_FUNCTION(tui_add_timer)
{
    zval *instance;
    zend_long interval_ms;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
        Z_PARAM_LONG(interval_ms)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        int timer_id = tui_app_add_timer(app, (int)interval_ms, &fci, &fcc);
        RETURN_LONG(timer_id);
    }

    RETURN_LONG(-1);
}
/* }}} */

/* {{{ tui_remove_timer(TuiInstance $instance, int $timer_id): void */
PHP_FUNCTION(tui_remove_timer)
{
    zval *instance;
    zend_long timer_id;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
        Z_PARAM_LONG(timer_id)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_remove_timer(app, (int)timer_id);
    }
}
/* }}} */

/* {{{ tui_set_tick_handler(TuiInstance $instance, callable $handler): void */
PHP_FUNCTION(tui_set_tick_handler)
{
    zval *instance;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_OBJECT_OF_CLASS(instance, tui_instance_ce)
        Z_PARAM_FUNC(fci, fcc)
    ZEND_PARSE_PARAMETERS_END();

    tui_app *app = get_app_from_instance(instance);
    if (app) {
        tui_app_set_tick_handler(app, &fci, &fcc);
    }
}
/* }}} */

/* ------------------------------------------------------------------
 * Text Utility Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_pad(string $text, int $width, string $align = 'l', string $pad_char = ' '): string */
PHP_FUNCTION(tui_pad)
{
    zend_string *text;
    zend_long width;
    zend_string *align = NULL;
    zend_string *pad_char = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 4)
        Z_PARAM_STR(text)
        Z_PARAM_LONG(width)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(align)
        Z_PARAM_STR(pad_char)
    ZEND_PARSE_PARAMETERS_END();

    /* Validate width to prevent excessive allocation */
    if (width < 0) width = 0;
    if (width > 10000) {
        php_error_docref(NULL, E_WARNING, "Width exceeds maximum of 10000");
        RETURN_FALSE;
    }

    char align_ch = align && ZSTR_LEN(align) > 0 ? ZSTR_VAL(align)[0] : 'l';
    char pad_ch = pad_char && ZSTR_LEN(pad_char) > 0 ? ZSTR_VAL(pad_char)[0] : ' ';

    /* Allocate output buffer (width + original text length + null)
     * This is sufficient because padding can only ADD (width - text_display_width) chars */
    size_t buf_size = (size_t)width + ZSTR_LEN(text) + 1;
    char *output = emalloc(buf_size);

    /* Use tui_pad_n for bounds-checked operation */
    int result_len = tui_pad_n(ZSTR_VAL(text), (int)width, align_ch, pad_ch, output, buf_size);
    if (result_len < 0) {
        efree(output);
        RETURN_FALSE;
    }

    RETVAL_STRINGL(output, result_len);
    efree(output);
}
/* }}} */

/* ------------------------------------------------------------------
 * Buffer Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_buffer_create(int $width, int $height): resource */
PHP_FUNCTION(tui_buffer_create)
{
    zend_long width, height;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = tui_buffer_create((int)width, (int)height);
    if (!buffer) {
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(buffer, le_tui_buffer));
}
/* }}} */

/* {{{ tui_buffer_clear(resource $buffer): void */
PHP_FUNCTION(tui_buffer_clear)
{
    zval *zbuffer;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zbuffer)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    tui_buffer_clear(buffer);
}
/* }}} */

/* {{{ tui_buffer_render(resource $buffer): string */
PHP_FUNCTION(tui_buffer_render)
{
    zval *zbuffer;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zbuffer)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    char *output = tui_buffer_to_string(buffer);
    if (output) {
        RETVAL_STRING(output);
        free(output);
    } else {
        RETURN_EMPTY_STRING();
    }
}
/* }}} */

/* Helper to parse style from array */
static void parse_style_array(zval *arr, tui_style *style)
{
    if (!arr || Z_TYPE_P(arr) != IS_ARRAY) return;

    HashTable *ht = Z_ARRVAL_P(arr);
    zval *val;

    memset(style, 0, sizeof(tui_style));

    if ((val = zend_hash_str_find(ht, "fg", 2)) != NULL ||
        (val = zend_hash_str_find(ht, "color", 5)) != NULL) {
        if (Z_TYPE_P(val) == IS_STRING) {
            style->fg = tui_color_from_hex(Z_STRVAL_P(val));
        } else if (Z_TYPE_P(val) == IS_ARRAY) {
            HashTable *color_ht = Z_ARRVAL_P(val);
            zval *r = zend_hash_index_find(color_ht, 0);
            zval *g = zend_hash_index_find(color_ht, 1);
            zval *b = zend_hash_index_find(color_ht, 2);
            if (r && g && b) {
                style->fg.r = (uint8_t)zval_get_long(r);
                style->fg.g = (uint8_t)zval_get_long(g);
                style->fg.b = (uint8_t)zval_get_long(b);
                style->fg.is_set = 1;
            }
        }
    }

    if ((val = zend_hash_str_find(ht, "bg", 2)) != NULL ||
        (val = zend_hash_str_find(ht, "backgroundColor", 15)) != NULL) {
        if (Z_TYPE_P(val) == IS_STRING) {
            style->bg = tui_color_from_hex(Z_STRVAL_P(val));
        } else if (Z_TYPE_P(val) == IS_ARRAY) {
            HashTable *color_ht = Z_ARRVAL_P(val);
            zval *r = zend_hash_index_find(color_ht, 0);
            zval *g = zend_hash_index_find(color_ht, 1);
            zval *b = zend_hash_index_find(color_ht, 2);
            if (r && g && b) {
                style->bg.r = (uint8_t)zval_get_long(r);
                style->bg.g = (uint8_t)zval_get_long(g);
                style->bg.b = (uint8_t)zval_get_long(b);
                style->bg.is_set = 1;
            }
        }
    }

    if ((val = zend_hash_str_find(ht, "bold", 4)) != NULL) {
        style->bold = zend_is_true(val);
    }
    if ((val = zend_hash_str_find(ht, "dim", 3)) != NULL) {
        style->dim = zend_is_true(val);
    }
    if ((val = zend_hash_str_find(ht, "italic", 6)) != NULL) {
        style->italic = zend_is_true(val);
    }
    if ((val = zend_hash_str_find(ht, "underline", 9)) != NULL) {
        style->underline = zend_is_true(val);
    }
    if ((val = zend_hash_str_find(ht, "inverse", 7)) != NULL) {
        style->inverse = zend_is_true(val);
    }
    if ((val = zend_hash_str_find(ht, "strikethrough", 13)) != NULL) {
        style->strikethrough = zend_is_true(val);
    }
}

/* ------------------------------------------------------------------
 * Drawing Primitive Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_draw_line(resource $buffer, int $x1, int $y1, int $x2, int $y2, string $char = '─', ?array $style = null): void */
PHP_FUNCTION(tui_draw_line)
{
    zval *zbuffer;
    zend_long x1, y1, x2, y2;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 7)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x1)
        Z_PARAM_LONG(y1)
        Z_PARAM_LONG(x2)
        Z_PARAM_LONG(y2)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = 0x2500; /* ─ */
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_draw_line(buffer, (int)x1, (int)y1, (int)x2, (int)y2, codepoint, &style);
}
/* }}} */

/* {{{ tui_draw_rect(resource $buffer, int $x, int $y, int $width, int $height, string $border = 'single', ?array $style = null): void */
PHP_FUNCTION(tui_draw_rect)
{
    zval *zbuffer;
    zend_long x, y, width, height;
    zend_string *border = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 7)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(border)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    tui_border_style border_style = TUI_BORDER_SINGLE;
    if (border) {
        const char *bs = ZSTR_VAL(border);
        if (strcmp(bs, "double") == 0) border_style = TUI_BORDER_DOUBLE;
        else if (strcmp(bs, "round") == 0) border_style = TUI_BORDER_ROUND;
        else if (strcmp(bs, "bold") == 0) border_style = TUI_BORDER_BOLD;
        else if (strcmp(bs, "dashed") == 0) border_style = TUI_BORDER_DASHED;
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_draw_rect(buffer, (int)x, (int)y, (int)width, (int)height, border_style, &style);
}
/* }}} */

/* {{{ tui_fill_rect(resource $buffer, int $x, int $y, int $width, int $height, string $char = ' ', ?array $style = null): void */
PHP_FUNCTION(tui_fill_rect)
{
    zval *zbuffer;
    zend_long x, y, width, height;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 7)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = ' ';
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_fill_rect(buffer, (int)x, (int)y, (int)width, (int)height, codepoint, &style);
}
/* }}} */

/* {{{ tui_draw_circle(resource $buffer, int $cx, int $cy, int $radius, string $char = '●', ?array $style = null): void */
PHP_FUNCTION(tui_draw_circle)
{
    zval *zbuffer;
    zend_long cx, cy, radius;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(4, 6)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(cx)
        Z_PARAM_LONG(cy)
        Z_PARAM_LONG(radius)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = 0x25CF; /* ● */
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_draw_circle(buffer, (int)cx, (int)cy, (int)radius, codepoint, &style);
}
/* }}} */

/* {{{ tui_fill_circle(resource $buffer, int $cx, int $cy, int $radius, string $char = '█', ?array $style = null): void */
PHP_FUNCTION(tui_fill_circle)
{
    zval *zbuffer;
    zend_long cx, cy, radius;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(4, 6)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(cx)
        Z_PARAM_LONG(cy)
        Z_PARAM_LONG(radius)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = 0x2588; /* █ */
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_fill_circle(buffer, (int)cx, (int)cy, (int)radius, codepoint, &style);
}
/* }}} */

/* {{{ tui_draw_ellipse(resource $buffer, int $cx, int $cy, int $rx, int $ry, string $char = '●', ?array $style = null): void */
PHP_FUNCTION(tui_draw_ellipse)
{
    zval *zbuffer;
    zend_long cx, cy, rx, ry;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 7)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(cx)
        Z_PARAM_LONG(cy)
        Z_PARAM_LONG(rx)
        Z_PARAM_LONG(ry)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = 0x25CF;
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_draw_ellipse(buffer, (int)cx, (int)cy, (int)rx, (int)ry, codepoint, &style);
}
/* }}} */

/* {{{ tui_fill_ellipse(resource $buffer, int $cx, int $cy, int $rx, int $ry, string $char = '█', ?array $style = null): void */
PHP_FUNCTION(tui_fill_ellipse)
{
    zval *zbuffer;
    zend_long cx, cy, rx, ry;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 7)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(cx)
        Z_PARAM_LONG(cy)
        Z_PARAM_LONG(rx)
        Z_PARAM_LONG(ry)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = 0x2588;
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_fill_ellipse(buffer, (int)cx, (int)cy, (int)rx, (int)ry, codepoint, &style);
}
/* }}} */

/* {{{ tui_draw_triangle(resource $buffer, int $x1, int $y1, int $x2, int $y2, int $x3, int $y3, string $char = '●', ?array $style = null): void */
PHP_FUNCTION(tui_draw_triangle)
{
    zval *zbuffer;
    zend_long x1, y1, x2, y2, x3, y3;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(7, 9)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x1)
        Z_PARAM_LONG(y1)
        Z_PARAM_LONG(x2)
        Z_PARAM_LONG(y2)
        Z_PARAM_LONG(x3)
        Z_PARAM_LONG(y3)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = 0x25CF;
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_draw_triangle(buffer, (int)x1, (int)y1, (int)x2, (int)y2, (int)x3, (int)y3, codepoint, &style);
}
/* }}} */

/* {{{ tui_fill_triangle(resource $buffer, int $x1, int $y1, int $x2, int $y2, int $x3, int $y3, string $char = '█', ?array $style = null): void */
PHP_FUNCTION(tui_fill_triangle)
{
    zval *zbuffer;
    zend_long x1, y1, x2, y2, x3, y3;
    zend_string *ch = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(7, 9)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x1)
        Z_PARAM_LONG(y1)
        Z_PARAM_LONG(x2)
        Z_PARAM_LONG(y2)
        Z_PARAM_LONG(x3)
        Z_PARAM_LONG(y3)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(ch)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    uint32_t codepoint = 0x2588;
    if (ch && ZSTR_LEN(ch) > 0) {
        tui_utf8_decode(ZSTR_VAL(ch), &codepoint);
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_fill_triangle(buffer, (int)x1, (int)y1, (int)x2, (int)y2, (int)x3, (int)y3, codepoint, &style);
}
/* }}} */

/* ------------------------------------------------------------------
 * Canvas Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_canvas_create(int $width, int $height, string $mode = 'braille'): resource */
PHP_FUNCTION(tui_canvas_create)
{
    zend_long width, height;
    zend_string *mode = NULL;

    ZEND_PARSE_PARAMETERS_START(2, 3)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(mode)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas_mode canvas_mode = TUI_CANVAS_BRAILLE;
    if (mode) {
        const char *m = ZSTR_VAL(mode);
        if (strcmp(m, "block") == 0) canvas_mode = TUI_CANVAS_BLOCK;
        else if (strcmp(m, "ascii") == 0) canvas_mode = TUI_CANVAS_ASCII;
    }

    tui_canvas *canvas = tui_canvas_create((int)width, (int)height, canvas_mode);
    if (!canvas) {
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(canvas, le_tui_canvas));
}
/* }}} */

/* {{{ tui_canvas_set(resource $canvas, int $x, int $y): void */
PHP_FUNCTION(tui_canvas_set)
{
    zval *zcanvas;
    zend_long x, y;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_set(canvas, (int)x, (int)y);
}
/* }}} */

/* {{{ tui_canvas_unset(resource $canvas, int $x, int $y): void */
PHP_FUNCTION(tui_canvas_unset)
{
    zval *zcanvas;
    zend_long x, y;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_unset(canvas, (int)x, (int)y);
}
/* }}} */

/* {{{ tui_canvas_toggle(resource $canvas, int $x, int $y): void */
PHP_FUNCTION(tui_canvas_toggle)
{
    zval *zcanvas;
    zend_long x, y;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_toggle(canvas, (int)x, (int)y);
}
/* }}} */

/* {{{ tui_canvas_get(resource $canvas, int $x, int $y): bool */
PHP_FUNCTION(tui_canvas_get)
{
    zval *zcanvas;
    zend_long x, y;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    RETURN_BOOL(tui_canvas_get(canvas, (int)x, (int)y));
}
/* }}} */

/* {{{ tui_canvas_clear(resource $canvas): void */
PHP_FUNCTION(tui_canvas_clear)
{
    zval *zcanvas;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zcanvas)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_clear_all(canvas);
}
/* }}} */

/* {{{ tui_canvas_line(resource $canvas, int $x1, int $y1, int $x2, int $y2): void */
PHP_FUNCTION(tui_canvas_line)
{
    zval *zcanvas;
    zend_long x1, y1, x2, y2;

    ZEND_PARSE_PARAMETERS_START(5, 5)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(x1)
        Z_PARAM_LONG(y1)
        Z_PARAM_LONG(x2)
        Z_PARAM_LONG(y2)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_line(canvas, (int)x1, (int)y1, (int)x2, (int)y2);
}
/* }}} */

/* {{{ tui_canvas_rect(resource $canvas, int $x, int $y, int $width, int $height): void */
PHP_FUNCTION(tui_canvas_rect)
{
    zval *zcanvas;
    zend_long x, y, width, height;

    ZEND_PARSE_PARAMETERS_START(5, 5)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_rect(canvas, (int)x, (int)y, (int)width, (int)height);
}
/* }}} */

/* {{{ tui_canvas_fill_rect(resource $canvas, int $x, int $y, int $width, int $height): void */
PHP_FUNCTION(tui_canvas_fill_rect)
{
    zval *zcanvas;
    zend_long x, y, width, height;

    ZEND_PARSE_PARAMETERS_START(5, 5)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_fill_rect(canvas, (int)x, (int)y, (int)width, (int)height);
}
/* }}} */

/* {{{ tui_canvas_circle(resource $canvas, int $cx, int $cy, int $radius): void */
PHP_FUNCTION(tui_canvas_circle)
{
    zval *zcanvas;
    zend_long cx, cy, radius;

    ZEND_PARSE_PARAMETERS_START(4, 4)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(cx)
        Z_PARAM_LONG(cy)
        Z_PARAM_LONG(radius)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_circle(canvas, (int)cx, (int)cy, (int)radius);
}
/* }}} */

/* {{{ tui_canvas_fill_circle(resource $canvas, int $cx, int $cy, int $radius): void */
PHP_FUNCTION(tui_canvas_fill_circle)
{
    zval *zcanvas;
    zend_long cx, cy, radius;

    ZEND_PARSE_PARAMETERS_START(4, 4)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(cx)
        Z_PARAM_LONG(cy)
        Z_PARAM_LONG(radius)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_fill_circle(canvas, (int)cx, (int)cy, (int)radius);
}
/* }}} */

/* {{{ tui_canvas_set_color(resource $canvas, int $r, int $g, int $b): void */
PHP_FUNCTION(tui_canvas_set_color)
{
    zval *zcanvas;
    zend_long r, g, b;

    ZEND_PARSE_PARAMETERS_START(4, 4)
        Z_PARAM_RESOURCE(zcanvas)
        Z_PARAM_LONG(r)
        Z_PARAM_LONG(g)
        Z_PARAM_LONG(b)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    tui_canvas_set_color(canvas, (uint8_t)r, (uint8_t)g, (uint8_t)b);
}
/* }}} */

/* {{{ tui_canvas_get_resolution(resource $canvas): array */
PHP_FUNCTION(tui_canvas_get_resolution)
{
    zval *zcanvas;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zcanvas)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    int width, height;
    tui_canvas_get_resolution(canvas, &width, &height);

    array_init(return_value);
    add_assoc_long(return_value, "width", width);
    add_assoc_long(return_value, "height", height);
    add_assoc_long(return_value, "char_width", canvas->char_width);
    add_assoc_long(return_value, "char_height", canvas->char_height);
}
/* }}} */

/* {{{ tui_canvas_render(resource $canvas): array */
PHP_FUNCTION(tui_canvas_render)
{
    zval *zcanvas;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zcanvas)
    ZEND_PARSE_PARAMETERS_END();

    tui_canvas *canvas = (tui_canvas *)zend_fetch_resource(Z_RES_P(zcanvas), TUI_CANVAS_RES_NAME, le_tui_canvas);
    if (!canvas) {
        RETURN_THROWS();
    }

    int line_count;
    char **lines = tui_canvas_render(canvas, &line_count);

    array_init(return_value);
    if (lines) {
        for (int i = 0; i < line_count; i++) {
            add_next_index_string(return_value, lines[i]);
        }
        tui_canvas_render_free(lines, line_count);
    }
}
/* }}} */

/* ------------------------------------------------------------------
 * Animation Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_ease(float $t, string $easing = 'linear'): float */
PHP_FUNCTION(tui_ease)
{
    double t;
    zend_string *easing = NULL;

    ZEND_PARSE_PARAMETERS_START(1, 2)
        Z_PARAM_DOUBLE(t)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(easing)
    ZEND_PARSE_PARAMETERS_END();

    tui_easing ease_type = TUI_EASE_LINEAR;
    if (easing) {
        ease_type = tui_ease_from_name(ZSTR_VAL(easing));
    }

    RETURN_DOUBLE(tui_ease(t, ease_type));
}
/* }}} */

/* {{{ tui_lerp(float $a, float $b, float $t): float */
PHP_FUNCTION(tui_lerp)
{
    double a, b, t;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_DOUBLE(a)
        Z_PARAM_DOUBLE(b)
        Z_PARAM_DOUBLE(t)
    ZEND_PARSE_PARAMETERS_END();

    RETURN_DOUBLE(tui_lerp(a, b, t));
}
/* }}} */

/* {{{ tui_lerp_color(string|array $a, string|array $b, float $t): string */
PHP_FUNCTION(tui_lerp_color)
{
    zval *za, *zb;
    double t;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_ZVAL(za)
        Z_PARAM_ZVAL(zb)
        Z_PARAM_DOUBLE(t)
    ZEND_PARSE_PARAMETERS_END();

    tui_color color_a = {0}, color_b = {0};

    /* Parse color a */
    if (Z_TYPE_P(za) == IS_STRING) {
        color_a = tui_color_from_hex(Z_STRVAL_P(za));
    } else if (Z_TYPE_P(za) == IS_ARRAY) {
        HashTable *ht = Z_ARRVAL_P(za);
        zval *r = zend_hash_index_find(ht, 0);
        zval *g = zend_hash_index_find(ht, 1);
        zval *b = zend_hash_index_find(ht, 2);
        if (r && g && b) {
            color_a.r = (uint8_t)zval_get_long(r);
            color_a.g = (uint8_t)zval_get_long(g);
            color_a.b = (uint8_t)zval_get_long(b);
            color_a.is_set = 1;
        }
    }

    /* Parse color b */
    if (Z_TYPE_P(zb) == IS_STRING) {
        color_b = tui_color_from_hex(Z_STRVAL_P(zb));
    } else if (Z_TYPE_P(zb) == IS_ARRAY) {
        HashTable *ht = Z_ARRVAL_P(zb);
        zval *r = zend_hash_index_find(ht, 0);
        zval *g = zend_hash_index_find(ht, 1);
        zval *b = zend_hash_index_find(ht, 2);
        if (r && g && b) {
            color_b.r = (uint8_t)zval_get_long(r);
            color_b.g = (uint8_t)zval_get_long(g);
            color_b.b = (uint8_t)zval_get_long(b);
            color_b.is_set = 1;
        }
    }

    tui_color result = tui_lerp_color(color_a, color_b, t);

    char hex[8];
    tui_color_to_hex(result, hex, sizeof(hex));
    RETURN_STRING(hex);
}
/* }}} */

/* {{{ tui_color_from_hex(string $hex): array */
PHP_FUNCTION(tui_color_from_hex)
{
    zend_string *hex;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(hex)
    ZEND_PARSE_PARAMETERS_END();

    tui_color color = tui_color_from_hex(ZSTR_VAL(hex));

    array_init(return_value);
    add_next_index_long(return_value, color.r);
    add_next_index_long(return_value, color.g);
    add_next_index_long(return_value, color.b);
}
/* }}} */

/* {{{ tui_gradient(array $colors, int $steps): array */
PHP_FUNCTION(tui_gradient)
{
    zval *zcolors;
    zend_long steps;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY(zcolors)
        Z_PARAM_LONG(steps)
    ZEND_PARSE_PARAMETERS_END();

    HashTable *ht = Z_ARRVAL_P(zcolors);
    int color_count = zend_hash_num_elements(ht);

    /* Handle edge cases */
    if (color_count < 1 || steps < 1) {
        array_init(return_value);
        return;
    }

    tui_color *colors = emalloc(sizeof(tui_color) * color_count);
    tui_color *output = emalloc(sizeof(tui_color) * steps);

    int i = 0;
    zval *zcolor;
    ZEND_HASH_FOREACH_VAL(ht, zcolor) {
        if (Z_TYPE_P(zcolor) == IS_STRING) {
            colors[i] = tui_color_from_hex(Z_STRVAL_P(zcolor));
        } else if (Z_TYPE_P(zcolor) == IS_ARRAY) {
            HashTable *color_ht = Z_ARRVAL_P(zcolor);
            zval *r = zend_hash_index_find(color_ht, 0);
            zval *g = zend_hash_index_find(color_ht, 1);
            zval *b = zend_hash_index_find(color_ht, 2);
            if (r && g && b) {
                colors[i].r = (uint8_t)zval_get_long(r);
                colors[i].g = (uint8_t)zval_get_long(g);
                colors[i].b = (uint8_t)zval_get_long(b);
                colors[i].is_set = 1;
            }
        }
        i++;
    } ZEND_HASH_FOREACH_END();

    /* C function handles single-color case (fills output with same color) */
    tui_gradient(colors, color_count, output, (int)steps);

    array_init(return_value);
    for (i = 0; i < (int)steps; i++) {
        char hex[8];
        tui_color_to_hex(output[i], hex, sizeof(hex));
        add_next_index_string(return_value, hex);
    }

    efree(colors);
    efree(output);
}
/* }}} */

/* ------------------------------------------------------------------
 * Table Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_table_create(array $headers): resource */
PHP_FUNCTION(tui_table_create)
{
    zval *zheaders;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY(zheaders)
    ZEND_PARSE_PARAMETERS_END();

    HashTable *ht = Z_ARRVAL_P(zheaders);
    int header_count = zend_hash_num_elements(ht);

    if (header_count == 0) {
        RETURN_FALSE;
    }

    const char **headers = emalloc(sizeof(char*) * header_count);
    int i = 0;
    zval *val;
    ZEND_HASH_FOREACH_VAL(ht, val) {
        convert_to_string(val);
        headers[i++] = Z_STRVAL_P(val);
    } ZEND_HASH_FOREACH_END();

    tui_table *table = tui_table_create(headers, header_count);
    efree(headers);

    if (!table) {
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(table, le_tui_table));
}
/* }}} */

/* {{{ tui_table_add_row(resource $table, array $cells): void */
PHP_FUNCTION(tui_table_add_row)
{
    zval *ztable;
    zval *zcells;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(ztable)
        Z_PARAM_ARRAY(zcells)
    ZEND_PARSE_PARAMETERS_END();

    tui_table *table = (tui_table *)zend_fetch_resource(Z_RES_P(ztable), TUI_TABLE_RES_NAME, le_tui_table);
    if (!table) {
        RETURN_THROWS();
    }

    HashTable *ht = Z_ARRVAL_P(zcells);
    int cell_count = zend_hash_num_elements(ht);

    const char **cells = emalloc(sizeof(char*) * cell_count);
    int i = 0;
    zval *val;
    ZEND_HASH_FOREACH_VAL(ht, val) {
        convert_to_string(val);
        cells[i++] = Z_STRVAL_P(val);
    } ZEND_HASH_FOREACH_END();

    tui_table_add_row(table, cells);
    efree(cells);
}
/* }}} */

/* {{{ tui_table_set_align(resource $table, int $column, bool $right_align): void */
PHP_FUNCTION(tui_table_set_align)
{
    zval *ztable;
    zend_long column;
    zend_bool right_align;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_RESOURCE(ztable)
        Z_PARAM_LONG(column)
        Z_PARAM_BOOL(right_align)
    ZEND_PARSE_PARAMETERS_END();

    tui_table *table = (tui_table *)zend_fetch_resource(Z_RES_P(ztable), TUI_TABLE_RES_NAME, le_tui_table);
    if (!table) {
        RETURN_THROWS();
    }

    tui_table_set_align(table, (int)column, right_align ? 1 : 0);
}
/* }}} */

/* {{{ tui_table_render_to_buffer(resource $buffer, resource $table, int $x, int $y, string $border = 'single', ?array $header_style = null, ?array $cell_style = null): int */
PHP_FUNCTION(tui_table_render_to_buffer)
{
    zval *zbuffer, *ztable;
    zend_long x, y;
    zend_string *border = NULL;
    zval *zheader_style = NULL, *zcell_style = NULL;

    ZEND_PARSE_PARAMETERS_START(4, 7)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_RESOURCE(ztable)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(border)
        Z_PARAM_ARRAY_OR_NULL(zheader_style)
        Z_PARAM_ARRAY_OR_NULL(zcell_style)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    tui_table *table = (tui_table *)zend_fetch_resource(Z_RES_P(ztable), TUI_TABLE_RES_NAME, le_tui_table);
    if (!table) {
        RETURN_THROWS();
    }

    tui_border_style border_style = TUI_BORDER_SINGLE;
    if (border) {
        const char *bs = ZSTR_VAL(border);
        if (strcmp(bs, "double") == 0) border_style = TUI_BORDER_DOUBLE;
        else if (strcmp(bs, "round") == 0) border_style = TUI_BORDER_ROUND;
        else if (strcmp(bs, "bold") == 0) border_style = TUI_BORDER_BOLD;
    }

    tui_style header_style = {0}, cell_style = {0};
    if (zheader_style) {
        parse_style_array(zheader_style, &header_style);
    }
    if (zcell_style) {
        parse_style_array(zcell_style, &cell_style);
    }

    int height = tui_table_render(buffer, table, (int)x, (int)y, border_style, &header_style, &cell_style);
    RETURN_LONG(height);
}
/* }}} */

/* ------------------------------------------------------------------
 * Progress/Spinner Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_render_progress_bar(resource $buffer, int $x, int $y, int $width, float $progress, ?array $style = null): void */
PHP_FUNCTION(tui_render_progress_bar)
{
    zval *zbuffer;
    zend_long x, y, width;
    double progress;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 6)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_LONG(width)
        Z_PARAM_DOUBLE(progress)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_render_progress(buffer, (int)x, (int)y, (int)width, progress, 0, 0, &style, NULL);
}
/* }}} */

/* {{{ tui_render_busy_bar(resource $buffer, int $x, int $y, int $width, int $frame, string $style_name = 'pulse', ?array $style = null): void */
PHP_FUNCTION(tui_render_busy_bar)
{
    zval *zbuffer;
    zend_long x, y, width, frame;
    zend_string *style_name = NULL;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 7)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(frame)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(style_name)
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    tui_busy_style busy_style = TUI_BUSY_PULSE;
    if (style_name) {
        const char *sn = ZSTR_VAL(style_name);
        if (strcmp(sn, "snake") == 0) busy_style = TUI_BUSY_SNAKE;
        else if (strcmp(sn, "gradient") == 0) busy_style = TUI_BUSY_GRADIENT;
        else if (strcmp(sn, "wave") == 0) busy_style = TUI_BUSY_WAVE;
        else if (strcmp(sn, "shimmer") == 0) busy_style = TUI_BUSY_SHIMMER;
        else if (strcmp(sn, "rainbow") == 0) busy_style = TUI_BUSY_RAINBOW;
    }

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_render_busy(buffer, (int)x, (int)y, (int)width, (int)frame, busy_style, &style);
}
/* }}} */

/* {{{ tui_spinner_frame(string $type, int $frame): string */
PHP_FUNCTION(tui_spinner_frame)
{
    zend_string *type;
    zend_long frame;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(type)
        Z_PARAM_LONG(frame)
    ZEND_PARSE_PARAMETERS_END();

    tui_spinner_type spinner_type = TUI_SPINNER_DOTS;
    const char *t = ZSTR_VAL(type);
    if (strcmp(t, "line") == 0) spinner_type = TUI_SPINNER_LINE;
    else if (strcmp(t, "circle") == 0) spinner_type = TUI_SPINNER_CIRCLE;
    else if (strcmp(t, "arrow") == 0) spinner_type = TUI_SPINNER_ARROW;
    else if (strcmp(t, "box") == 0) spinner_type = TUI_SPINNER_BOX;
    else if (strcmp(t, "bounce") == 0) spinner_type = TUI_SPINNER_BOUNCE;
    else if (strcmp(t, "clock") == 0) spinner_type = TUI_SPINNER_CLOCK;
    else if (strcmp(t, "moon") == 0) spinner_type = TUI_SPINNER_MOON;
    else if (strcmp(t, "earth") == 0) spinner_type = TUI_SPINNER_EARTH;

    char output[16];
    int len = tui_spinner_frame(spinner_type, (int)frame, output);
    RETURN_STRINGL(output, len);
}
/* }}} */

/* {{{ tui_spinner_frame_count(string $type): int */
PHP_FUNCTION(tui_spinner_frame_count)
{
    zend_string *type;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(type)
    ZEND_PARSE_PARAMETERS_END();

    tui_spinner_type spinner_type = TUI_SPINNER_DOTS;
    const char *t = ZSTR_VAL(type);
    if (strcmp(t, "line") == 0) spinner_type = TUI_SPINNER_LINE;
    else if (strcmp(t, "circle") == 0) spinner_type = TUI_SPINNER_CIRCLE;
    else if (strcmp(t, "arrow") == 0) spinner_type = TUI_SPINNER_ARROW;
    else if (strcmp(t, "box") == 0) spinner_type = TUI_SPINNER_BOX;
    else if (strcmp(t, "bounce") == 0) spinner_type = TUI_SPINNER_BOUNCE;
    else if (strcmp(t, "clock") == 0) spinner_type = TUI_SPINNER_CLOCK;
    else if (strcmp(t, "moon") == 0) spinner_type = TUI_SPINNER_MOON;
    else if (strcmp(t, "earth") == 0) spinner_type = TUI_SPINNER_EARTH;

    RETURN_LONG(tui_spinner_frame_count(spinner_type));
}
/* }}} */

/* {{{ tui_render_spinner(resource $buffer, int $x, int $y, string $type, int $frame, ?array $style = null): void */
PHP_FUNCTION(tui_render_spinner)
{
    zval *zbuffer;
    zend_long x, y, frame;
    zend_string *type;
    zval *zstyle = NULL;

    ZEND_PARSE_PARAMETERS_START(5, 6)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_STR(type)
        Z_PARAM_LONG(frame)
        Z_PARAM_OPTIONAL
        Z_PARAM_ARRAY_OR_NULL(zstyle)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    tui_spinner_type spinner_type = TUI_SPINNER_DOTS;
    const char *t = ZSTR_VAL(type);
    if (strcmp(t, "line") == 0) spinner_type = TUI_SPINNER_LINE;
    else if (strcmp(t, "circle") == 0) spinner_type = TUI_SPINNER_CIRCLE;
    else if (strcmp(t, "arrow") == 0) spinner_type = TUI_SPINNER_ARROW;
    else if (strcmp(t, "box") == 0) spinner_type = TUI_SPINNER_BOX;
    else if (strcmp(t, "bounce") == 0) spinner_type = TUI_SPINNER_BOUNCE;
    else if (strcmp(t, "clock") == 0) spinner_type = TUI_SPINNER_CLOCK;
    else if (strcmp(t, "moon") == 0) spinner_type = TUI_SPINNER_MOON;
    else if (strcmp(t, "earth") == 0) spinner_type = TUI_SPINNER_EARTH;

    tui_style style = {0};
    if (zstyle) {
        parse_style_array(zstyle, &style);
    }

    tui_render_spinner(buffer, (int)x, (int)y, spinner_type, (int)frame, &style);
}
/* }}} */

/* ------------------------------------------------------------------
 * Sprite Functions
 * ------------------------------------------------------------------ */

/* {{{ tui_sprite_create(array $frames, string $name = 'default', bool $loop = true): resource */
PHP_FUNCTION(tui_sprite_create)
{
    zval *zframes;
    zend_string *name = NULL;
    zend_bool loop = 1;

    ZEND_PARSE_PARAMETERS_START(1, 3)
        Z_PARAM_ARRAY(zframes)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR_OR_NULL(name)
        Z_PARAM_BOOL(loop)
    ZEND_PARSE_PARAMETERS_END();

    HashTable *ht = Z_ARRVAL_P(zframes);
    int frame_count = zend_hash_num_elements(ht);

    if (frame_count == 0) {
        RETURN_FALSE;
    }

    tui_sprite_frame *frames = ecalloc(frame_count, sizeof(tui_sprite_frame));
    int i = 0;
    zval *zframe;

    ZEND_HASH_FOREACH_VAL(ht, zframe) {
        if (Z_TYPE_P(zframe) != IS_ARRAY) continue;

        HashTable *frame_ht = Z_ARRVAL_P(zframe);
        zval *zlines = zend_hash_str_find(frame_ht, "lines", 5);
        zval *zduration = zend_hash_str_find(frame_ht, "duration", 8);

        int duration = zduration ? (int)zval_get_long(zduration) : 100;

        if (zlines && Z_TYPE_P(zlines) == IS_ARRAY) {
            HashTable *lines_ht = Z_ARRVAL_P(zlines);
            int line_count = zend_hash_num_elements(lines_ht);
            const char **lines = emalloc(sizeof(char*) * line_count);

            int j = 0;
            zval *zline;
            ZEND_HASH_FOREACH_VAL(lines_ht, zline) {
                convert_to_string(zline);
                lines[j++] = Z_STRVAL_P(zline);
            } ZEND_HASH_FOREACH_END();

            frames[i] = tui_sprite_frame_create(lines, line_count, duration);
            efree(lines);
        }
        i++;
    } ZEND_HASH_FOREACH_END();

    const char *anim_name = name ? ZSTR_VAL(name) : "default";
    tui_sprite *sprite = tui_sprite_create(frames, frame_count, anim_name, loop ? 1 : 0);

    /* Free temporary frame data (sprite makes copies) */
    for (i = 0; i < frame_count; i++) {
        tui_sprite_frame_free(&frames[i]);
    }
    efree(frames);

    if (!sprite) {
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(sprite, le_tui_sprite));
}
/* }}} */

/* {{{ tui_sprite_update(resource $sprite, int $delta_ms): void */
PHP_FUNCTION(tui_sprite_update)
{
    zval *zsprite;
    zend_long delta_ms;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zsprite)
        Z_PARAM_LONG(delta_ms)
    ZEND_PARSE_PARAMETERS_END();

    tui_sprite *sprite = (tui_sprite *)zend_fetch_resource(Z_RES_P(zsprite), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!sprite) {
        RETURN_THROWS();
    }

    tui_sprite_update(sprite, (int)delta_ms);
}
/* }}} */

/* {{{ tui_sprite_set_animation(resource $sprite, string $name): bool */
PHP_FUNCTION(tui_sprite_set_animation)
{
    zval *zsprite;
    zend_string *name;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zsprite)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    tui_sprite *sprite = (tui_sprite *)zend_fetch_resource(Z_RES_P(zsprite), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!sprite) {
        RETURN_THROWS();
    }

    RETURN_BOOL(tui_sprite_set_animation(sprite, ZSTR_VAL(name)) == 0);
}
/* }}} */

/* {{{ tui_sprite_set_position(resource $sprite, int $x, int $y): void */
PHP_FUNCTION(tui_sprite_set_position)
{
    zval *zsprite;
    zend_long x, y;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_RESOURCE(zsprite)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
    ZEND_PARSE_PARAMETERS_END();

    tui_sprite *sprite = (tui_sprite *)zend_fetch_resource(Z_RES_P(zsprite), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!sprite) {
        RETURN_THROWS();
    }

    tui_sprite_set_position(sprite, (int)x, (int)y);
}
/* }}} */

/* {{{ tui_sprite_flip(resource $sprite, bool $flipped): void */
PHP_FUNCTION(tui_sprite_flip)
{
    zval *zsprite;
    zend_bool flipped;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zsprite)
        Z_PARAM_BOOL(flipped)
    ZEND_PARSE_PARAMETERS_END();

    tui_sprite *sprite = (tui_sprite *)zend_fetch_resource(Z_RES_P(zsprite), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!sprite) {
        RETURN_THROWS();
    }

    tui_sprite_flip(sprite, flipped ? 1 : 0);
}
/* }}} */

/* {{{ tui_sprite_set_visible(resource $sprite, bool $visible): void */
PHP_FUNCTION(tui_sprite_set_visible)
{
    zval *zsprite;
    zend_bool visible;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zsprite)
        Z_PARAM_BOOL(visible)
    ZEND_PARSE_PARAMETERS_END();

    tui_sprite *sprite = (tui_sprite *)zend_fetch_resource(Z_RES_P(zsprite), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!sprite) {
        RETURN_THROWS();
    }

    tui_sprite_set_visible(sprite, visible ? 1 : 0);
}
/* }}} */

/* {{{ tui_sprite_render(resource $buffer, resource $sprite): void */
PHP_FUNCTION(tui_sprite_render)
{
    zval *zbuffer, *zsprite;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zbuffer)
        Z_PARAM_RESOURCE(zsprite)
    ZEND_PARSE_PARAMETERS_END();

    tui_buffer *buffer = (tui_buffer *)zend_fetch_resource(Z_RES_P(zbuffer), TUI_BUFFER_RES_NAME, le_tui_buffer);
    if (!buffer) {
        RETURN_THROWS();
    }

    tui_sprite *sprite = (tui_sprite *)zend_fetch_resource(Z_RES_P(zsprite), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!sprite) {
        RETURN_THROWS();
    }

    tui_sprite_render(buffer, sprite);
}
/* }}} */

/* {{{ tui_sprite_get_bounds(resource $sprite): array */
PHP_FUNCTION(tui_sprite_get_bounds)
{
    zval *zsprite;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zsprite)
    ZEND_PARSE_PARAMETERS_END();

    tui_sprite *sprite = (tui_sprite *)zend_fetch_resource(Z_RES_P(zsprite), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!sprite) {
        RETURN_THROWS();
    }

    int x, y, width, height;
    tui_sprite_get_bounds(sprite, &x, &y, &width, &height);

    array_init(return_value);
    add_assoc_long(return_value, "x", x);
    add_assoc_long(return_value, "y", y);
    add_assoc_long(return_value, "width", width);
    add_assoc_long(return_value, "height", height);
}
/* }}} */

/* {{{ tui_sprite_collides(resource $a, resource $b): bool */
PHP_FUNCTION(tui_sprite_collides)
{
    zval *za, *zb;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(za)
        Z_PARAM_RESOURCE(zb)
    ZEND_PARSE_PARAMETERS_END();

    tui_sprite *a = (tui_sprite *)zend_fetch_resource(Z_RES_P(za), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!a) {
        RETURN_THROWS();
    }

    tui_sprite *b = (tui_sprite *)zend_fetch_resource(Z_RES_P(zb), TUI_SPRITE_RES_NAME, le_tui_sprite);
    if (!b) {
        RETURN_THROWS();
    }

    RETURN_BOOL(tui_sprite_collides(a, b));
}
/* }}} */

/* {{{ Testing framework functions */

/**
 * tui_test_create(int $width, int $height): resource
 * Create a headless test renderer
 */
PHP_FUNCTION(tui_test_create)
{
    zend_long width, height;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
    ZEND_PARSE_PARAMETERS_END();

    if (width <= 0 || width > 1000 || height <= 0 || height > 1000) {
        zend_throw_exception(NULL, "Invalid dimensions: width and height must be 1-1000", 0);
        RETURN_THROWS();
    }

    tui_test_renderer *renderer = tui_test_renderer_create((int)width, (int)height);
    if (!renderer) {
        zend_throw_exception(NULL, "Failed to create test renderer", 0);
        RETURN_THROWS();
    }

    RETURN_RES(zend_register_resource(renderer, le_tui_test_renderer));
}

/**
 * tui_test_destroy(resource $renderer): void
 * Destroy a test renderer
 */
PHP_FUNCTION(tui_test_destroy)
{
    zval *zrenderer;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrenderer)
    ZEND_PARSE_PARAMETERS_END();

    /* Closing the resource will call the destructor */
    zend_list_close(Z_RES_P(zrenderer));
}

/**
 * tui_test_render(resource $renderer, TuiBox|TuiText $element): void
 * Render an element tree to the test renderer
 */
PHP_FUNCTION(tui_test_render)
{
    zval *zrenderer, *zelement;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zrenderer)
        Z_PARAM_OBJECT(zelement)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    /* Convert PHP element to tui_node tree */
    extern tui_node* php_to_tui_node(zval *element);
    tui_node *root = php_to_tui_node(zelement);
    if (!root) {
        zend_throw_exception(NULL, "Failed to convert element to node", 0);
        RETURN_THROWS();
    }

    /* Render to test buffer */
    tui_test_renderer_render(renderer, root);
}

/**
 * tui_test_get_output(resource $renderer): array
 * Get rendered output as array of lines
 */
PHP_FUNCTION(tui_test_get_output)
{
    zval *zrenderer;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrenderer)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    int line_count;
    char **lines = tui_test_renderer_get_output(renderer, &line_count);
    if (!lines) {
        array_init(return_value);
        return;
    }

    array_init_size(return_value, line_count);
    for (int i = 0; i < line_count; i++) {
        add_next_index_string(return_value, lines[i]);
    }

    tui_test_renderer_free_output(lines, line_count);
}

/**
 * tui_test_to_string(resource $renderer): string
 * Get rendered output as single string with newlines
 */
PHP_FUNCTION(tui_test_to_string)
{
    zval *zrenderer;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrenderer)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    char *output = tui_test_renderer_to_string(renderer);
    if (!output) {
        RETURN_EMPTY_STRING();
    }

    RETVAL_STRING(output);
    free(output);
}

/**
 * tui_test_send_input(resource $renderer, string $input): void
 * Queue input to be processed on next frame
 */
PHP_FUNCTION(tui_test_send_input)
{
    zval *zrenderer;
    zend_string *input;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zrenderer)
        Z_PARAM_STR(input)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    tui_test_renderer_send_input(renderer, ZSTR_VAL(input), (int)ZSTR_LEN(input));
}

/**
 * tui_test_send_key(resource $renderer, int $keyCode): void
 * Queue a special key to be processed on next frame
 */
PHP_FUNCTION(tui_test_send_key)
{
    zval *zrenderer;
    zend_long key_code;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zrenderer)
        Z_PARAM_LONG(key_code)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    tui_test_renderer_send_key(renderer, (int)key_code);
}

/**
 * tui_test_advance_frame(resource $renderer): void
 * Advance one frame - process input and re-render
 */
PHP_FUNCTION(tui_test_advance_frame)
{
    zval *zrenderer;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zrenderer)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    tui_test_renderer_advance_frame(renderer);
}

/**
 * tui_test_run_timers(resource $renderer, int $ms): void
 * Advance simulated time and run due timers
 */
PHP_FUNCTION(tui_test_run_timers)
{
    zval *zrenderer;
    zend_long ms;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zrenderer)
        Z_PARAM_LONG(ms)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    tui_test_renderer_run_timers(renderer, (int)ms);
}

/**
 * tui_test_get_by_id(resource $renderer, string $id): ?array
 * Find a node by ID and return its info
 */
PHP_FUNCTION(tui_test_get_by_id)
{
    zval *zrenderer;
    zend_string *id;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zrenderer)
        Z_PARAM_STR(id)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    if (!renderer->root) {
        RETURN_NULL();
    }

    tui_node *node = tui_test_find_by_id(renderer->root, ZSTR_VAL(id));
    if (!node) {
        RETURN_NULL();
    }

    tui_test_node_info info;
    tui_test_get_node_info(node, &info);

    array_init(return_value);
    if (info.id) add_assoc_string(return_value, "id", (char *)info.id);
    add_assoc_string(return_value, "type", (char *)info.type);
    if (info.text) add_assoc_string(return_value, "text", (char *)info.text);
    add_assoc_long(return_value, "x", info.x);
    add_assoc_long(return_value, "y", info.y);
    add_assoc_long(return_value, "width", info.width);
    add_assoc_long(return_value, "height", info.height);
    add_assoc_bool(return_value, "focusable", info.focusable);
    add_assoc_bool(return_value, "focused", info.focused);
}

/**
 * tui_test_get_by_text(resource $renderer, string $text): array
 * Find all nodes containing specific text
 */
PHP_FUNCTION(tui_test_get_by_text)
{
    zval *zrenderer;
    zend_string *text;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(zrenderer)
        Z_PARAM_STR(text)
    ZEND_PARSE_PARAMETERS_END();

    tui_test_renderer *renderer = (tui_test_renderer *)zend_fetch_resource(
        Z_RES_P(zrenderer), TUI_TEST_RENDERER_RES_NAME, le_tui_test_renderer);
    if (!renderer) {
        RETURN_THROWS();
    }

    array_init(return_value);

    if (!renderer->root) {
        return;
    }

    int count;
    tui_node **nodes = tui_test_find_by_text(renderer->root, ZSTR_VAL(text), &count);
    if (!nodes || count == 0) {
        return;
    }

    for (int i = 0; i < count; i++) {
        zval node_arr;
        tui_test_node_info info;
        tui_test_get_node_info(nodes[i], &info);

        array_init(&node_arr);
        if (info.id) add_assoc_string(&node_arr, "id", (char *)info.id);
        add_assoc_string(&node_arr, "type", (char *)info.type);
        if (info.text) add_assoc_string(&node_arr, "text", (char *)info.text);
        add_assoc_long(&node_arr, "x", info.x);
        add_assoc_long(&node_arr, "y", info.y);
        add_assoc_long(&node_arr, "width", info.width);
        add_assoc_long(&node_arr, "height", info.height);
        add_assoc_bool(&node_arr, "focusable", info.focusable);
        add_assoc_bool(&node_arr, "focused", info.focused);

        add_next_index_zval(return_value, &node_arr);
    }

    free(nodes);
}
/* }}} */

/* {{{ Metrics functions */

/**
 * tui_metrics_enable(): void
 * Enable metrics collection
 */
PHP_FUNCTION(tui_metrics_enable)
{
    ZEND_PARSE_PARAMETERS_NONE();
    TUI_G(metrics_enabled) = 1;
}

/**
 * tui_metrics_disable(): void
 * Disable metrics collection
 */
PHP_FUNCTION(tui_metrics_disable)
{
    ZEND_PARSE_PARAMETERS_NONE();
    TUI_G(metrics_enabled) = 0;
}

/**
 * tui_metrics_enabled(): bool
 * Check if metrics collection is enabled
 */
PHP_FUNCTION(tui_metrics_enabled)
{
    ZEND_PARSE_PARAMETERS_NONE();
    RETURN_BOOL(TUI_G(metrics_enabled));
}

/**
 * tui_metrics_reset(): void
 * Reset all metrics counters to zero
 */
PHP_FUNCTION(tui_metrics_reset)
{
    ZEND_PARSE_PARAMETERS_NONE();
    memset(&TUI_G(metrics), 0, sizeof(tui_metrics));
}

/**
 * tui_get_metrics(): array
 * Get all metrics as an associative array
 */
PHP_FUNCTION(tui_get_metrics)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_metrics *m = &TUI_G(metrics);

    array_init(return_value);

    /* Node metrics */
    add_assoc_long(return_value, "node_count", (zend_long)m->node_count);
    add_assoc_long(return_value, "box_count", (zend_long)m->box_count);
    add_assoc_long(return_value, "text_count", (zend_long)m->text_count);
    add_assoc_long(return_value, "static_count", (zend_long)m->static_count);
    add_assoc_long(return_value, "max_depth", (zend_long)m->max_depth);

    /* Reconciler metrics */
    add_assoc_long(return_value, "diff_runs", (zend_long)m->diff_runs);
    add_assoc_long(return_value, "create_ops", (zend_long)m->create_ops);
    add_assoc_long(return_value, "update_ops", (zend_long)m->update_ops);
    add_assoc_long(return_value, "delete_ops", (zend_long)m->delete_ops);
    add_assoc_long(return_value, "replace_ops", (zend_long)m->replace_ops);
    add_assoc_long(return_value, "reorder_ops", (zend_long)m->reorder_ops);
    add_assoc_long(return_value, "total_ops", (zend_long)(m->create_ops + m->update_ops +
        m->delete_ops + m->replace_ops + m->reorder_ops));

    /* Render timing (convert ns to ms) */
    add_assoc_long(return_value, "render_count", (zend_long)m->render_count);
    add_assoc_double(return_value, "layout_time_ms", (double)m->layout_time_ns / 1000000.0);
    add_assoc_double(return_value, "buffer_time_ms", (double)m->buffer_time_ns / 1000000.0);
    add_assoc_double(return_value, "output_time_ms", (double)m->output_time_ns / 1000000.0);
    add_assoc_double(return_value, "total_render_time_ms",
        (double)(m->layout_time_ns + m->buffer_time_ns + m->output_time_ns) / 1000000.0);
    if (m->render_count > 0) {
        add_assoc_double(return_value, "avg_render_ms",
            (double)(m->layout_time_ns + m->buffer_time_ns + m->output_time_ns) /
            (double)m->render_count / 1000000.0);
    } else {
        add_assoc_double(return_value, "avg_render_ms", 0.0);
    }
    add_assoc_double(return_value, "max_render_ms", (double)m->max_render_ns / 1000000.0);
    add_assoc_double(return_value, "min_render_ms", (double)m->min_render_ns / 1000000.0);

    /* Layout metrics */
    add_assoc_long(return_value, "layout_runs", (zend_long)m->layout_runs);
    add_assoc_long(return_value, "measure_calls", (zend_long)m->measure_calls);
    add_assoc_long(return_value, "baseline_calls", (zend_long)m->baseline_calls);

    /* Event loop metrics */
    add_assoc_long(return_value, "loop_iterations", (zend_long)m->loop_iterations);
    add_assoc_long(return_value, "input_events", (zend_long)m->input_events);
    add_assoc_long(return_value, "resize_events", (zend_long)m->resize_events);
    add_assoc_long(return_value, "timer_fires", (zend_long)m->timer_fires);

    /* Pool metrics */
    if (TUI_G(pools)) {
        tui_pools *p = TUI_G(pools);
        add_assoc_long(return_value, "pool_children_hits", (zend_long)p->children_hits);
        add_assoc_long(return_value, "pool_children_misses", (zend_long)p->children_misses);
        add_assoc_long(return_value, "pool_children_returns", (zend_long)p->children_returns);
        add_assoc_long(return_value, "pool_keymap_reuses", (zend_long)p->key_map_reuses);
        add_assoc_long(return_value, "pool_keymap_misses", (zend_long)p->key_map_misses);
    }
}

/**
 * tui_get_node_metrics(): array
 * Get only node-related metrics
 */
PHP_FUNCTION(tui_get_node_metrics)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_metrics *m = &TUI_G(metrics);

    array_init(return_value);
    add_assoc_long(return_value, "node_count", (zend_long)m->node_count);
    add_assoc_long(return_value, "box_count", (zend_long)m->box_count);
    add_assoc_long(return_value, "text_count", (zend_long)m->text_count);
    add_assoc_long(return_value, "static_count", (zend_long)m->static_count);
    add_assoc_long(return_value, "max_depth", (zend_long)m->max_depth);
}

/**
 * tui_get_reconciler_metrics(): array
 * Get only reconciler-related metrics
 */
PHP_FUNCTION(tui_get_reconciler_metrics)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_metrics *m = &TUI_G(metrics);
    int64_t total = m->create_ops + m->update_ops + m->delete_ops +
                    m->replace_ops + m->reorder_ops;

    array_init(return_value);
    add_assoc_long(return_value, "diff_runs", (zend_long)m->diff_runs);
    add_assoc_long(return_value, "total_ops", (zend_long)total);
    add_assoc_long(return_value, "creates", (zend_long)m->create_ops);
    add_assoc_long(return_value, "updates", (zend_long)m->update_ops);
    add_assoc_long(return_value, "deletes", (zend_long)m->delete_ops);
    add_assoc_long(return_value, "replaces", (zend_long)m->replace_ops);
    add_assoc_long(return_value, "reorders", (zend_long)m->reorder_ops);
    if (m->diff_runs > 0) {
        add_assoc_double(return_value, "avg_ops_per_diff", (double)total / (double)m->diff_runs);
    } else {
        add_assoc_double(return_value, "avg_ops_per_diff", 0.0);
    }
}

/**
 * tui_get_render_metrics(): array
 * Get only render timing metrics
 */
PHP_FUNCTION(tui_get_render_metrics)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_metrics *m = &TUI_G(metrics);
    int64_t total_ns = m->layout_time_ns + m->buffer_time_ns + m->output_time_ns;

    array_init(return_value);
    add_assoc_long(return_value, "render_count", (zend_long)m->render_count);
    add_assoc_double(return_value, "layout_time_ms", (double)m->layout_time_ns / 1000000.0);
    add_assoc_double(return_value, "buffer_time_ms", (double)m->buffer_time_ns / 1000000.0);
    add_assoc_double(return_value, "output_time_ms", (double)m->output_time_ns / 1000000.0);
    add_assoc_double(return_value, "total_render_time_ms", (double)total_ns / 1000000.0);
    if (m->render_count > 0) {
        add_assoc_double(return_value, "avg_render_ms", (double)total_ns / (double)m->render_count / 1000000.0);
    } else {
        add_assoc_double(return_value, "avg_render_ms", 0.0);
    }
    add_assoc_double(return_value, "max_render_ms", (double)m->max_render_ns / 1000000.0);
    add_assoc_double(return_value, "min_render_ms", (double)m->min_render_ns / 1000000.0);
}

/**
 * tui_get_loop_metrics(): array
 * Get only event loop metrics
 */
PHP_FUNCTION(tui_get_loop_metrics)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_metrics *m = &TUI_G(metrics);

    array_init(return_value);
    add_assoc_long(return_value, "loop_iterations", (zend_long)m->loop_iterations);
    add_assoc_long(return_value, "input_events", (zend_long)m->input_events);
    add_assoc_long(return_value, "resize_events", (zend_long)m->resize_events);
    add_assoc_long(return_value, "timer_fires", (zend_long)m->timer_fires);
}

/**
 * tui_get_pool_metrics(): array
 * Get object pool metrics
 */
PHP_FUNCTION(tui_get_pool_metrics)
{
    ZEND_PARSE_PARAMETERS_NONE();

    array_init(return_value);

    if (TUI_G(pools)) {
        tui_pools *p = TUI_G(pools);
        add_assoc_long(return_value, "children_allocs", (zend_long)p->children_hits);
        add_assoc_long(return_value, "children_fallbacks", (zend_long)p->children_misses);
        add_assoc_long(return_value, "children_reuses", (zend_long)p->children_returns);
        add_assoc_long(return_value, "keymap_reuses", (zend_long)p->key_map_reuses);
        add_assoc_long(return_value, "keymap_fallbacks", (zend_long)p->key_map_misses);

        /* Pool efficiency percentages */
        int64_t total_children = p->children_hits + p->children_misses;
        if (total_children > 0) {
            add_assoc_double(return_value, "children_hit_rate",
                (double)p->children_hits / (double)total_children * 100.0);
        } else {
            add_assoc_double(return_value, "children_hit_rate", 0.0);
        }

        int64_t total_keymap = p->key_map_reuses + p->key_map_misses;
        if (total_keymap > 0) {
            add_assoc_double(return_value, "keymap_hit_rate",
                (double)p->key_map_reuses / (double)total_keymap * 100.0);
        } else {
            add_assoc_double(return_value, "keymap_hit_rate", 0.0);
        }
    }
}
/* }}} */

/* {{{ arginfo */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_get_terminal_size, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_is_interactive, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_tui_is_ci, 0, 0, _IS_BOOL, 0)
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
/* }}} */

/* {{{ tui_functions[] */
static const zend_function_entry tui_functions[] = {
    /* Terminal info */
    PHP_FE(tui_get_terminal_size, arginfo_tui_get_terminal_size)
    PHP_FE(tui_is_interactive, arginfo_tui_is_interactive)
    PHP_FE(tui_is_ci, arginfo_tui_is_ci)

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

    /* Register mouse mode constants */
    REGISTER_LONG_CONSTANT("TUI_MOUSE_MODE_OFF", TUI_MOUSE_MODE_OFF, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_MOUSE_MODE_CLICK", TUI_MOUSE_MODE_CLICK, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_MOUSE_MODE_BUTTON", TUI_MOUSE_MODE_BUTTON, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("TUI_MOUSE_MODE_ALL", TUI_MOUSE_MODE_ALL, CONST_CS | CONST_PERSISTENT);

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
    zend_declare_property_null(tui_box_ce, "children", sizeof("children")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "key", sizeof("key")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "id", sizeof("id")-1, ZEND_ACC_PUBLIC);
    /* Per-side border colors */
    zend_declare_property_null(tui_box_ce, "borderTopColor", sizeof("borderTopColor")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "borderRightColor", sizeof("borderRightColor")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "borderBottomColor", sizeof("borderBottomColor")-1, ZEND_ACC_PUBLIC);
    zend_declare_property_null(tui_box_ce, "borderLeftColor", sizeof("borderLeftColor")-1, ZEND_ACC_PUBLIC);

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
