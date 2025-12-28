/*
  +----------------------------------------------------------------------+
  | ext-tui: Virtual DOM node                                           |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_NODE_H
#define TUI_NODE_H

#include <stdint.h>
#include <yoga/Yoga.h>
#include "../text/wrap.h"

typedef enum {
    TUI_NODE_BOX,
    TUI_NODE_TEXT
} tui_node_type;

typedef struct {
    uint8_t r, g, b;
    uint8_t is_set;
} tui_color;

typedef struct {
    tui_color fg;
    tui_color bg;
    uint8_t bold;
    uint8_t dim;
    uint8_t italic;
    uint8_t underline;
    uint8_t inverse;
    uint8_t strikethrough;
} tui_style;

/* Text wrap modes - defined in text/wrap.h */
/* TUI_WRAP_NONE, TUI_WRAP_CHAR, TUI_WRAP_WORD, TUI_WRAP_WORD_CHAR */

typedef enum {
    TUI_BORDER_NONE,
    TUI_BORDER_SINGLE,
    TUI_BORDER_DOUBLE,
    TUI_BORDER_ROUND,
    TUI_BORDER_BOLD,
    TUI_BORDER_DASHED
} tui_border_style;

typedef struct tui_node {
    tui_node_type type;
    char *key;                   /* Node identity for reconciler */
    tui_style style;

    /* For text nodes */
    char *text;
    tui_wrap_mode wrap_mode;

    /* For box nodes with borders */
    tui_border_style border_style;
    tui_color border_color;

    /* Focus management */
    int focusable;
    int focused;

    /* Yoga layout node */
    YGNodeRef yoga_node;

    /* Tree structure */
    struct tui_node *parent;
    struct tui_node **children;
    int child_count;
    int child_capacity;

    /* Computed layout (from Yoga) */
    float x, y, width, height;
} tui_node;

/* Lifecycle */
tui_node* tui_node_create_box(void);
tui_node* tui_node_create_text(const char *text);
void tui_node_destroy(tui_node *node);

/* Tree manipulation */
void tui_node_append_child(tui_node *parent, tui_node *child);
void tui_node_remove_child(tui_node *parent, tui_node *child);
void tui_node_insert_before(tui_node *parent, tui_node *child, tui_node *before);

/* Style */
void tui_node_set_style(tui_node *node, const tui_style *style);

/* Yoga style properties */
typedef enum {
    TUI_YOGA_WIDTH,
    TUI_YOGA_HEIGHT,
    TUI_YOGA_MIN_WIDTH,
    TUI_YOGA_MIN_HEIGHT,
    TUI_YOGA_MAX_WIDTH,
    TUI_YOGA_MAX_HEIGHT,
    TUI_YOGA_FLEX_GROW,
    TUI_YOGA_FLEX_SHRINK,
    TUI_YOGA_FLEX_BASIS,
    TUI_YOGA_PADDING_ALL,
    TUI_YOGA_PADDING_TOP,
    TUI_YOGA_PADDING_BOTTOM,
    TUI_YOGA_PADDING_LEFT,
    TUI_YOGA_PADDING_RIGHT,
    TUI_YOGA_MARGIN_ALL,
    TUI_YOGA_MARGIN_TOP,
    TUI_YOGA_MARGIN_BOTTOM,
    TUI_YOGA_MARGIN_LEFT,
    TUI_YOGA_MARGIN_RIGHT,
    TUI_YOGA_GAP_ALL,
    TUI_YOGA_GAP_ROW,
    TUI_YOGA_GAP_COLUMN,
    TUI_YOGA_BORDER_ALL,
    TUI_YOGA_BORDER_TOP,
    TUI_YOGA_BORDER_BOTTOM,
    TUI_YOGA_BORDER_LEFT,
    TUI_YOGA_BORDER_RIGHT
} tui_yoga_property;

void tui_node_set_yoga_style(tui_node *node, tui_yoga_property property, float value);

/* Layout */
void tui_node_calculate_layout(tui_node *root, float width, float height);

#endif /* TUI_NODE_H */
