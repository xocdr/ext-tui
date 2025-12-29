/*
  +----------------------------------------------------------------------+
  | ext-tui: Virtual DOM node                                           |
  +----------------------------------------------------------------------+
*/

#include "node.h"
#include "../text/measure.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#define INITIAL_CHILDREN_CAPACITY 4

/* Forward declarations */
static int copy_layout_recursive(tui_node *node);
static YGSize text_measure_func(YGNodeConstRef yg_node, float width,
    YGMeasureMode widthMode, float height, YGMeasureMode heightMode);

tui_node* tui_node_create_box(void)
{
    tui_node *node = calloc(1, sizeof(tui_node));
    if (!node) return NULL;

    node->type = TUI_NODE_BOX;
    node->yoga_node = YGNodeNew();
    if (!node->yoga_node) {
        free(node);
        return NULL;
    }

    node->child_capacity = INITIAL_CHILDREN_CAPACITY;
    node->children = calloc(node->child_capacity, sizeof(tui_node*));
    if (!node->children) {
        YGNodeFree(node->yoga_node);
        free(node);
        return NULL;
    }

    return node;
}

tui_node* tui_node_create_text(const char *text)
{
    tui_node *node = calloc(1, sizeof(tui_node));
    if (!node) return NULL;

    node->type = TUI_NODE_TEXT;
    node->yoga_node = YGNodeNew();
    if (!node->yoga_node) {
        free(node);
        return NULL;
    }

    node->text = strdup(text ? text : "");
    if (!node->text) {
        YGNodeFree(node->yoga_node);
        free(node);
        return NULL;
    }

    /* Set up text measurement for Yoga */
    YGNodeSetContext(node->yoga_node, node);
    YGNodeSetMeasureFunc(node->yoga_node, text_measure_func);

    return node;
}

tui_node* tui_node_create_static(void)
{
    tui_node *node = calloc(1, sizeof(tui_node));
    if (!node) return NULL;

    node->type = TUI_NODE_STATIC;
    node->yoga_node = YGNodeNew();
    if (!node->yoga_node) {
        free(node);
        return NULL;
    }

    node->child_capacity = INITIAL_CHILDREN_CAPACITY;
    node->children = calloc(node->child_capacity, sizeof(tui_node*));
    if (!node->children) {
        YGNodeFree(node->yoga_node);
        free(node);
        return NULL;
    }

    return node;
}

tui_node* tui_node_create_newline(int count)
{
    tui_node *node = calloc(1, sizeof(tui_node));
    if (!node) return NULL;

    node->type = TUI_NODE_NEWLINE;
    node->newline_count = count > 0 ? count : 1;
    node->yoga_node = YGNodeNew();
    if (!node->yoga_node) {
        free(node);
        return NULL;
    }

    /* Set height to number of lines */
    YGNodeStyleSetHeight(node->yoga_node, (float)node->newline_count);

    return node;
}

tui_node* tui_node_create_spacer(void)
{
    tui_node *node = calloc(1, sizeof(tui_node));
    if (!node) return NULL;

    node->type = TUI_NODE_SPACER;
    node->yoga_node = YGNodeNew();
    if (!node->yoga_node) {
        free(node);
        return NULL;
    }

    /* Spacer has flexGrow: 1 by default */
    YGNodeStyleSetFlexGrow(node->yoga_node, 1.0f);

    return node;
}

int tui_node_set_id(tui_node *node, const char *id)
{
    if (!node) return -1;

    free(node->id);
    if (id) {
        node->id = strdup(id);
        if (!node->id) return -1;  /* Allocation failed */
    } else {
        node->id = NULL;
    }
    return 0;
}

void tui_node_destroy(tui_node *node)
{
    if (!node) return;

    /* Destroy children recursively */
    for (int i = 0; i < node->child_count; i++) {
        tui_node_destroy(node->children[i]);
    }

    /* Free resources */
    if (node->yoga_node) {
        YGNodeFree(node->yoga_node);
    }
    free(node->children);
    free(node->text);
    free(node->key);
    free(node->id);
    free(node);
}

void tui_node_append_child(tui_node *parent, tui_node *child)
{
    if (!parent || !child) return;

    /* Grow array if needed */
    if (parent->child_count >= parent->child_capacity) {
        /* Check for overflow before doubling */
        if (parent->child_capacity > INT_MAX / 2) return;
        int new_capacity = parent->child_capacity * 2;

        /* Check for size_t overflow in allocation */
        if ((size_t)new_capacity > SIZE_MAX / sizeof(tui_node*)) return;

        tui_node **new_children = realloc(parent->children,
            (size_t)new_capacity * sizeof(tui_node*));
        if (!new_children) return;  /* Allocation failed */
        parent->children = new_children;
        parent->child_capacity = new_capacity;
    }

    parent->children[parent->child_count++] = child;
    child->parent = parent;

    /* Update Yoga tree */
    YGNodeInsertChild(parent->yoga_node, child->yoga_node, parent->child_count - 1);
}

void tui_node_remove_child(tui_node *parent, tui_node *child)
{
    if (!parent || !child) return;

    int index = -1;
    for (int i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == child) {
            index = i;
            break;
        }
    }

    if (index >= 0) {
        /* Shift remaining children */
        for (int i = index; i < parent->child_count - 1; i++) {
            parent->children[i] = parent->children[i + 1];
        }
        parent->child_count--;
        child->parent = NULL;

        /* Update Yoga tree */
        YGNodeRemoveChild(parent->yoga_node, child->yoga_node);
    }
}

void tui_node_insert_before(tui_node *parent, tui_node *child, tui_node *before)
{
    if (!parent || !child) return;

    if (!before) {
        tui_node_append_child(parent, child);
        return;
    }

    int index = -1;
    for (int i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == before) {
            index = i;
            break;
        }
    }

    if (index >= 0) {
        /* Grow array if needed */
        if (parent->child_count >= parent->child_capacity) {
            /* Check for overflow before doubling */
            if (parent->child_capacity > INT_MAX / 2) return;
            int new_capacity = parent->child_capacity * 2;

            /* Check for size_t overflow in allocation */
            if ((size_t)new_capacity > SIZE_MAX / sizeof(tui_node*)) return;

            tui_node **new_children = realloc(parent->children,
                (size_t)new_capacity * sizeof(tui_node*));
            if (!new_children) return;  /* Allocation failed */
            parent->children = new_children;
            parent->child_capacity = new_capacity;
        }

        /* Shift children to make room */
        for (int i = parent->child_count; i > index; i--) {
            parent->children[i] = parent->children[i - 1];
        }

        parent->children[index] = child;
        parent->child_count++;
        child->parent = parent;

        /* Update Yoga tree */
        YGNodeInsertChild(parent->yoga_node, child->yoga_node, index);
    }
}

void tui_node_set_style(tui_node *node, const tui_style *style)
{
    if (node && style) {
        node->style = *style;
    }
}

void tui_node_set_yoga_style(tui_node *node, tui_yoga_property property, float value)
{
    if (!node || !node->yoga_node) return;

    switch (property) {
        case TUI_YOGA_WIDTH:
            YGNodeStyleSetWidth(node->yoga_node, value);
            break;
        case TUI_YOGA_HEIGHT:
            YGNodeStyleSetHeight(node->yoga_node, value);
            break;
        case TUI_YOGA_MIN_WIDTH:
            YGNodeStyleSetMinWidth(node->yoga_node, value);
            break;
        case TUI_YOGA_MIN_HEIGHT:
            YGNodeStyleSetMinHeight(node->yoga_node, value);
            break;
        case TUI_YOGA_MAX_WIDTH:
            YGNodeStyleSetMaxWidth(node->yoga_node, value);
            break;
        case TUI_YOGA_MAX_HEIGHT:
            YGNodeStyleSetMaxHeight(node->yoga_node, value);
            break;
        case TUI_YOGA_FLEX_GROW:
            YGNodeStyleSetFlexGrow(node->yoga_node, value);
            break;
        case TUI_YOGA_FLEX_SHRINK:
            YGNodeStyleSetFlexShrink(node->yoga_node, value);
            break;
        case TUI_YOGA_FLEX_BASIS:
            YGNodeStyleSetFlexBasis(node->yoga_node, value);
            break;
        case TUI_YOGA_PADDING_ALL:
            YGNodeStyleSetPadding(node->yoga_node, YGEdgeAll, value);
            break;
        case TUI_YOGA_PADDING_TOP:
            YGNodeStyleSetPadding(node->yoga_node, YGEdgeTop, value);
            break;
        case TUI_YOGA_PADDING_BOTTOM:
            YGNodeStyleSetPadding(node->yoga_node, YGEdgeBottom, value);
            break;
        case TUI_YOGA_PADDING_LEFT:
            YGNodeStyleSetPadding(node->yoga_node, YGEdgeLeft, value);
            break;
        case TUI_YOGA_PADDING_RIGHT:
            YGNodeStyleSetPadding(node->yoga_node, YGEdgeRight, value);
            break;
        case TUI_YOGA_MARGIN_ALL:
            YGNodeStyleSetMargin(node->yoga_node, YGEdgeAll, value);
            break;
        case TUI_YOGA_MARGIN_TOP:
            YGNodeStyleSetMargin(node->yoga_node, YGEdgeTop, value);
            break;
        case TUI_YOGA_MARGIN_BOTTOM:
            YGNodeStyleSetMargin(node->yoga_node, YGEdgeBottom, value);
            break;
        case TUI_YOGA_MARGIN_LEFT:
            YGNodeStyleSetMargin(node->yoga_node, YGEdgeLeft, value);
            break;
        case TUI_YOGA_MARGIN_RIGHT:
            YGNodeStyleSetMargin(node->yoga_node, YGEdgeRight, value);
            break;
        case TUI_YOGA_GAP_ALL:
            YGNodeStyleSetGap(node->yoga_node, YGGutterAll, value);
            break;
        case TUI_YOGA_GAP_ROW:
            YGNodeStyleSetGap(node->yoga_node, YGGutterRow, value);
            break;
        case TUI_YOGA_GAP_COLUMN:
            YGNodeStyleSetGap(node->yoga_node, YGGutterColumn, value);
            break;
        case TUI_YOGA_BORDER_ALL:
            YGNodeStyleSetBorder(node->yoga_node, YGEdgeAll, value);
            break;
        case TUI_YOGA_BORDER_TOP:
            YGNodeStyleSetBorder(node->yoga_node, YGEdgeTop, value);
            break;
        case TUI_YOGA_BORDER_BOTTOM:
            YGNodeStyleSetBorder(node->yoga_node, YGEdgeBottom, value);
            break;
        case TUI_YOGA_BORDER_LEFT:
            YGNodeStyleSetBorder(node->yoga_node, YGEdgeLeft, value);
            break;
        case TUI_YOGA_BORDER_RIGHT:
            YGNodeStyleSetBorder(node->yoga_node, YGEdgeRight, value);
            break;
    }
}

void tui_node_calculate_layout(tui_node *root, float width, float height)
{
    if (!root) return;

    YGNodeCalculateLayout(root->yoga_node, width, height, YGDirectionLTR);

    /* Copy layout results to nodes */
    copy_layout_recursive(root);
}

/* Text measurement function for Yoga */
static YGSize text_measure_func(YGNodeConstRef yg_node, float width,
    YGMeasureMode widthMode, float height, YGMeasureMode heightMode)
{
    YGSize size = {0, 0};
    tui_node *node = (tui_node *)YGNodeGetContext(yg_node);

    if (!node || !node->text) {
        return size;
    }

    /* Calculate text width using our text measurement */
    int text_width = tui_string_width(node->text);

    /* Count lines for height */
    int lines = 1;
    const char *p = node->text;
    while (*p) {
        if (*p == '\n') lines++;
        p++;
    }

    /* Apply width constraints */
    if (widthMode == YGMeasureModeExactly) {
        size.width = width;
    } else if (widthMode == YGMeasureModeAtMost) {
        size.width = text_width < width ? text_width : width;
    } else {
        size.width = text_width;
    }

    /* Apply height constraints */
    if (heightMode == YGMeasureModeExactly) {
        size.height = height;
    } else if (heightMode == YGMeasureModeAtMost) {
        size.height = lines < height ? lines : height;
    } else {
        size.height = lines;
    }

    return size;
}

/*
 * Recursively copy layout results from Yoga to tui_node.
 * Only copies nodes that have new layout data (optimization).
 * Returns 1 if any node in subtree had new layout, 0 otherwise.
 */
static int copy_layout_recursive(tui_node *node)
{
    if (!node || !node->yoga_node) return 0;

    int had_changes = 0;

    /* Check if this node has new layout data */
    if (YGNodeGetHasNewLayout(node->yoga_node)) {
        /* Copy this node's layout */
        node->x = YGNodeLayoutGetLeft(node->yoga_node);
        node->y = YGNodeLayoutGetTop(node->yoga_node);
        node->width = YGNodeLayoutGetWidth(node->yoga_node);
        node->height = YGNodeLayoutGetHeight(node->yoga_node);

        /* Clear the flag so we don't copy again unnecessarily */
        YGNodeSetHasNewLayout(node->yoga_node, false);
        had_changes = 1;
    }

    /* Recurse to children (always, as children may have changed even if parent didn't) */
    for (int i = 0; i < node->child_count; i++) {
        if (copy_layout_recursive(node->children[i])) {
            had_changes = 1;
        }
    }

    return had_changes;
}
