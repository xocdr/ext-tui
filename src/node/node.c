/*
  +----------------------------------------------------------------------+
  | ext-tui: Virtual DOM node                                           |
  +----------------------------------------------------------------------+
  |                                                                      |
  | MEMORY ALLOCATION STRATEGY                                           |
  | ==========================                                           |
  |                                                                      |
  | This module uses a hybrid allocation strategy for children arrays:   |
  |                                                                      |
  | 1. POOL ALLOCATION (preferred):                                      |
  |    When TUI_G(pools) is available, children arrays are allocated     |
  |    from a pre-sized pool. This reduces malloc overhead during the    |
  |    hot path of reconciliation and tree building.                     |
  |                                                                      |
  | 2. STANDARD MALLOC (fallback):                                       |
  |    When pools aren't available (early init or pool exhausted),       |
  |    standard calloc/realloc is used.                                  |
  |                                                                      |
  | 3. TRANSITION HANDLING:                                              |
  |    - children_from_pool flag tracks the source of each array         |
  |    - When growing, we may transition from malloc to pool             |
  |    - Free operations check the flag to call the correct deallocator  |
  |                                                                      |
  | Node structs themselves always use standard malloc/free.             |
  | Text strings (node->text, node->key) use strdup/free.                |
  |                                                                      |
  +----------------------------------------------------------------------+
*/

#include "node.h"
#include "../text/measure.h"
#include "../pool/pool.h"
#include "php.h"
#include "php_tui.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#define INITIAL_CHILDREN_CAPACITY 4

/* Maximum recursion depth for tree traversal (prevents stack overflow) */
#define MAX_TREE_DEPTH 256

/* Forward declarations */
static int copy_layout_recursive(tui_node *node);
static YGSize text_measure_func(YGNodeConstRef yg_node, float width,
    YGMeasureMode widthMode, float height, YGMeasureMode heightMode);
static float text_baseline_func(YGNodeConstRef yg_node, float width, float height);
static void node_dirtied_func(YGNodeConstRef yg_node);

/*
 * Helper to grow a node's children array when needed.
 * Returns 0 on success, -1 on failure.
 */
static int ensure_children_capacity(tui_node *parent)
{
    if (parent->child_count < parent->child_capacity) {
        return 0;  /* Already have room */
    }

    /* Check for overflow before doubling */
    if (parent->child_capacity > INT_MAX / 2) return -1;
    int new_capacity = parent->child_capacity * 2;

    /* Check for size_t overflow in allocation */
    if ((size_t)new_capacity > SIZE_MAX / sizeof(tui_node*)) return -1;

    tui_node **new_children;
    int actual_capacity;

    if (parent->children_from_pool && TUI_G(pools)) {
        /* Get new array from pool, copy, release old */
        new_children = tui_children_pool_alloc(TUI_G(pools), new_capacity, &actual_capacity);
        if (!new_children) return -1;
        memcpy(new_children, parent->children, parent->child_count * sizeof(tui_node*));
        tui_children_pool_free(TUI_G(pools), parent->children, parent->child_capacity);
        new_capacity = actual_capacity;
        /* New array is also from pool */
    } else if (TUI_G(pools)) {
        /* Old array was malloc'd but pools now available - get from pool */
        new_children = tui_children_pool_alloc(TUI_G(pools), new_capacity, &actual_capacity);
        if (!new_children) return -1;
        memcpy(new_children, parent->children, parent->child_count * sizeof(tui_node*));
        free(parent->children);
        new_capacity = actual_capacity;
        parent->children_from_pool = 1;
    } else {
        /* No pools - use realloc */
        new_children = realloc(parent->children, (size_t)new_capacity * sizeof(tui_node*));
        if (!new_children) return -1;
        parent->children_from_pool = 0;
    }

    parent->children = new_children;
    parent->child_capacity = new_capacity;
    return 0;
}

tui_node* tui_node_create_box(void)
{
    tui_node *node = calloc(1, sizeof(tui_node));
    if (!node) return NULL;

    node->type = TUI_NODE_BOX;
    YGConfigRef config = tui_get_yoga_config();
    node->yoga_node = config ? YGNodeNewWithConfig(config) : YGNodeNew();
    if (!node->yoga_node) {
        free(node);
        return NULL;
    }

    /* Allocate children array from pool if available */
    int actual_capacity = INITIAL_CHILDREN_CAPACITY;
    if (TUI_G(pools)) {
        node->children = tui_children_pool_alloc(TUI_G(pools), INITIAL_CHILDREN_CAPACITY, &actual_capacity);
        node->children_from_pool = (node->children != NULL);
    } else {
        node->children = calloc(INITIAL_CHILDREN_CAPACITY, sizeof(tui_node*));
        node->children_from_pool = 0;
    }
    if (!node->children) {
        YGNodeFree(node->yoga_node);
        free(node);
        return NULL;
    }
    node->child_capacity = actual_capacity;

    /* Set up context and dirtied callback for incremental layout */
    YGNodeSetContext(node->yoga_node, node);
    YGNodeSetDirtiedFunc(node->yoga_node, node_dirtied_func);

    /* Track metrics */
    TUI_METRIC_INC(node_count);
    TUI_METRIC_INC(box_count);

    return node;
}

tui_node* tui_node_create_text(const char *text)
{
    tui_node *node = calloc(1, sizeof(tui_node));
    if (!node) return NULL;

    node->type = TUI_NODE_TEXT;
    YGConfigRef config = tui_get_yoga_config();
    node->yoga_node = config ? YGNodeNewWithConfig(config) : YGNodeNew();
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

    /* Set up text measurement, baseline, and dirtied callback for Yoga */
    YGNodeSetContext(node->yoga_node, node);
    YGNodeSetMeasureFunc(node->yoga_node, text_measure_func);
    YGNodeSetBaselineFunc(node->yoga_node, text_baseline_func);
    YGNodeSetDirtiedFunc(node->yoga_node, node_dirtied_func);

    /* Track metrics */
    TUI_METRIC_INC(node_count);
    TUI_METRIC_INC(text_count);

    return node;
}

tui_node* tui_node_create_static(void)
{
    tui_node *node = calloc(1, sizeof(tui_node));
    if (!node) return NULL;

    node->type = TUI_NODE_STATIC;
    YGConfigRef config = tui_get_yoga_config();
    node->yoga_node = config ? YGNodeNewWithConfig(config) : YGNodeNew();
    if (!node->yoga_node) {
        free(node);
        return NULL;
    }

    /* Allocate children array from pool if available */
    int actual_capacity = INITIAL_CHILDREN_CAPACITY;
    if (TUI_G(pools)) {
        node->children = tui_children_pool_alloc(TUI_G(pools), INITIAL_CHILDREN_CAPACITY, &actual_capacity);
        node->children_from_pool = (node->children != NULL);
    } else {
        node->children = calloc(INITIAL_CHILDREN_CAPACITY, sizeof(tui_node*));
        node->children_from_pool = 0;
    }
    if (!node->children) {
        YGNodeFree(node->yoga_node);
        free(node);
        return NULL;
    }
    node->child_capacity = actual_capacity;

    /* Set up context and dirtied callback */
    YGNodeSetContext(node->yoga_node, node);
    YGNodeSetDirtiedFunc(node->yoga_node, node_dirtied_func);

    /* Track metrics */
    TUI_METRIC_INC(node_count);
    TUI_METRIC_INC(static_count);

    return node;
}

tui_node* tui_node_create_newline(int count)
{
    tui_node *node = calloc(1, sizeof(tui_node));
    if (!node) return NULL;

    node->type = TUI_NODE_NEWLINE;
    node->newline_count = count > 0 ? count : 1;
    YGConfigRef config = tui_get_yoga_config();
    node->yoga_node = config ? YGNodeNewWithConfig(config) : YGNodeNew();
    if (!node->yoga_node) {
        free(node);
        return NULL;
    }

    /* Set height to number of lines */
    YGNodeStyleSetHeight(node->yoga_node, (float)node->newline_count);

    /* Set up context and dirtied callback */
    YGNodeSetContext(node->yoga_node, node);
    YGNodeSetDirtiedFunc(node->yoga_node, node_dirtied_func);

    /* Track metrics */
    TUI_METRIC_INC(node_count);

    return node;
}

tui_node* tui_node_create_spacer(void)
{
    tui_node *node = calloc(1, sizeof(tui_node));
    if (!node) return NULL;

    node->type = TUI_NODE_SPACER;
    YGConfigRef config = tui_get_yoga_config();
    node->yoga_node = config ? YGNodeNewWithConfig(config) : YGNodeNew();
    if (!node->yoga_node) {
        free(node);
        return NULL;
    }

    /* Spacer has flexGrow: 1 by default */
    YGNodeStyleSetFlexGrow(node->yoga_node, 1.0f);

    /* Set up context and dirtied callback */
    YGNodeSetContext(node->yoga_node, node);
    YGNodeSetDirtiedFunc(node->yoga_node, node_dirtied_func);

    /* Track metrics */
    TUI_METRIC_INC(node_count);

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

int tui_node_set_hyperlink(tui_node *node, const char *url, const char *id)
{
    if (!node) return -1;

    free(node->hyperlink_url);
    free(node->hyperlink_id);

    if (url) {
        node->hyperlink_url = strdup(url);
        if (!node->hyperlink_url) return -1;
    } else {
        node->hyperlink_url = NULL;
    }

    if (id) {
        node->hyperlink_id = strdup(id);
        if (!node->hyperlink_id) {
            free(node->hyperlink_url);
            node->hyperlink_url = NULL;
            return -1;
        }
    } else {
        node->hyperlink_id = NULL;
    }

    return 0;
}

void tui_node_destroy(tui_node *node)
{
    if (!node) return;

    /* Track metrics - decrement based on type */
    TUI_METRIC_DEC(node_count);
    switch (node->type) {
        case TUI_NODE_BOX:    TUI_METRIC_DEC(box_count); break;
        case TUI_NODE_TEXT:   TUI_METRIC_DEC(text_count); break;
        case TUI_NODE_STATIC: TUI_METRIC_DEC(static_count); break;
        default: break;
    }

    /* Destroy children recursively (child_count is 0 if children is NULL) */
    if (node->children) {
        for (int i = 0; i < node->child_count; i++) {
            tui_node_destroy(node->children[i]);
        }
    }

    /* Free resources */
    if (node->yoga_node) {
        YGNodeFree(node->yoga_node);
    }
    /* Return children array to pool or free based on origin */
    if (node->children) {
        if (node->children_from_pool && TUI_G(pools)) {
            tui_children_pool_free(TUI_G(pools), node->children, node->child_capacity);
        } else {
            free(node->children);
        }
    }
    free(node->text);
    free(node->key);
    free(node->id);
    free(node->hyperlink_url);
    free(node->hyperlink_id);
    free(node->focus_group);
    free(node);
}

int tui_node_append_child(tui_node *parent, tui_node *child)
{
    if (!parent || !child) return -1;

    if (ensure_children_capacity(parent) != 0) {
        return -1;
    }

    parent->children[parent->child_count++] = child;
    child->parent = parent;

    /* Update Yoga tree */
    YGNodeInsertChild(parent->yoga_node, child->yoga_node, parent->child_count - 1);
    return 0;
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
        if (ensure_children_capacity(parent) != 0) {
            return;
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

    /* Track layout run */
    TUI_METRIC_INC(layout_runs);

    /* Use direction from root node, defaulting to LTR if not set */
    YGDirection dir = YGNodeStyleGetDirection(root->yoga_node);
    if (dir == YGDirectionInherit) {
        dir = YGDirectionLTR;
    }

    YGNodeCalculateLayout(root->yoga_node, width, height, dir);

    /* Copy layout results to nodes */
    copy_layout_recursive(root);
}

/* Text measurement function for Yoga */
static YGSize text_measure_func(YGNodeConstRef yg_node, float width,
    YGMeasureMode widthMode, float height, YGMeasureMode heightMode)
{
    YGSize size = {0, 0};
    tui_node *node = (tui_node *)YGNodeGetContext(yg_node);

    /* Track measure call */
    TUI_METRIC_INC(measure_calls);

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
 * Baseline function for text nodes.
 * Returns the baseline position (distance from top to text baseline).
 * For terminal text, baseline is at the bottom of the first line.
 */
static float text_baseline_func(YGNodeConstRef yg_node, float width, float height)
{
    /* Track baseline call */
    TUI_METRIC_INC(baseline_calls);

    tui_node *node = (tui_node *)YGNodeGetContext(yg_node);
    if (!node || !node->text) {
        return height;
    }

    /* For multi-line text, baseline is at the bottom of the first line (1 char tall) */
    const char *nl = strchr(node->text, '\n');
    if (nl) {
        return 1.0f;
    }

    /* For single-line text, baseline is at the bottom */
    return height;
}

/*
 * Dirtied callback - called when a node's layout needs recalculation.
 * This enables incremental layout updates by tracking which nodes changed.
 */
static void node_dirtied_func(YGNodeConstRef yg_node)
{
    tui_node *node = (tui_node *)YGNodeGetContext(yg_node);
    if (node) {
        node->layout_dirty = 1;
    }
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

/* ----------------------------------------------------------------
 * Focus Management
 * ---------------------------------------------------------------- */

int tui_node_set_focus_group(tui_node *node, const char *group)
{
    if (!node) return -1;

    free(node->focus_group);
    if (group) {
        node->focus_group = strdup(group);
        if (!node->focus_group) return -1;
    } else {
        node->focus_group = NULL;
    }
    return 0;
}

/* Helper: collect all focusable nodes in tree order */
typedef struct {
    tui_node **nodes;
    int count;
    int capacity;
} focus_list;

static int focus_list_add(focus_list *list, tui_node *node)
{
    if (list->count >= list->capacity) {
        int new_cap = list->capacity * 2;
        if (new_cap < 16) new_cap = 16;
        /* Check for integer overflow in allocation size */
        if (new_cap > INT_MAX / (int)sizeof(tui_node*)) {
            return -1;  /* Would overflow */
        }
        tui_node **new_nodes = realloc(list->nodes, (size_t)new_cap * sizeof(tui_node*));
        if (!new_nodes) return -1;
        list->nodes = new_nodes;
        list->capacity = new_cap;
    }
    list->nodes[list->count++] = node;
    return 0;
}

/**
 * Recursively collect focusable nodes into a list.
 * Returns 0 on success, -1 on allocation failure.
 */
static int collect_focusable_nodes(tui_node *node, focus_list *list, const char *group, tui_node *trap_root)
{
    if (!node) return 0;

    /* Skip if we have a trap but this node is outside it */
    if (trap_root && node != trap_root) {
        /* Check if node is a descendant of trap_root */
        tui_node *p = node->parent;
        int inside_trap = 0;
        while (p) {
            if (p == trap_root) { inside_trap = 1; break; }
            p = p->parent;
        }
        if (!inside_trap) return 0;
    }

    /* If focusable and tab_index >= 0, add to list */
    if (node->focusable && node->tab_index >= 0) {
        /* If group filter specified, only add matching group */
        if (!group || (node->focus_group && strcmp(node->focus_group, group) == 0)) {
            if (focus_list_add(list, node) != 0) {
                return -1;  /* Allocation failed */
            }
        }
    }

    /* Recurse to children */
    for (int i = 0; i < node->child_count; i++) {
        if (collect_focusable_nodes(node->children[i], list, group, trap_root) != 0) {
            return -1;  /* Propagate error */
        }
    }

    return 0;
}

/* Compare function for sorting by tab_index */
static int compare_tab_index(const void *a, const void *b)
{
    tui_node *na = *(tui_node **)a;
    tui_node *nb = *(tui_node **)b;

    /* Nodes with tab_index 0 should come before explicit indices in DOM order */
    if (na->tab_index == 0 && nb->tab_index == 0) return 0;
    if (na->tab_index == 0) return 1;  /* DOM order items after explicit */
    if (nb->tab_index == 0) return -1;
    return na->tab_index - nb->tab_index;
}

tui_node* tui_focus_find_trap_container(tui_node *node)
{
    if (!node) return NULL;

    tui_node *p = node;
    while (p) {
        if (p->focus_trap) return p;
        p = p->parent;
    }
    return NULL;  /* No trap active */
}

tui_node* tui_focus_find_next(tui_node *root, tui_node *current)
{
    if (!root) return NULL;

    /* Check for focus trap */
    tui_node *trap = current ? tui_focus_find_trap_container(current) : NULL;
    tui_node *search_root = trap ? trap : root;

    /* Collect focusable nodes */
    focus_list list = {NULL, 0, 0};
    if (collect_focusable_nodes(search_root, &list, NULL, trap) != 0) {
        free(list.nodes);
        return NULL;  /* Allocation failed */
    }

    if (list.count == 0) {
        free(list.nodes);
        return NULL;
    }

    /* Sort by tab_index (stable for DOM order when tab_index == 0) */
    /* Note: using simple sort; for true stability would need merge sort */
    qsort(list.nodes, list.count, sizeof(tui_node*), compare_tab_index);

    /* Find current position and return next (or first if at end/not found) */
    int cur_idx = -1;
    for (int i = 0; i < list.count; i++) {
        if (list.nodes[i] == current) {
            cur_idx = i;
            break;
        }
    }

    tui_node *result;
    if (cur_idx < 0 || cur_idx >= list.count - 1) {
        result = list.nodes[0];  /* Wrap to first */
    } else {
        result = list.nodes[cur_idx + 1];
    }

    free(list.nodes);
    return result;
}

tui_node* tui_focus_find_prev(tui_node *root, tui_node *current)
{
    if (!root) return NULL;

    /* Check for focus trap */
    tui_node *trap = current ? tui_focus_find_trap_container(current) : NULL;
    tui_node *search_root = trap ? trap : root;

    /* Collect focusable nodes */
    focus_list list = {NULL, 0, 0};
    if (collect_focusable_nodes(search_root, &list, NULL, trap) != 0) {
        free(list.nodes);
        return NULL;  /* Allocation failed */
    }

    if (list.count == 0) {
        free(list.nodes);
        return NULL;
    }

    qsort(list.nodes, list.count, sizeof(tui_node*), compare_tab_index);

    /* Find current position and return prev (or last if at start/not found) */
    int cur_idx = -1;
    for (int i = 0; i < list.count; i++) {
        if (list.nodes[i] == current) {
            cur_idx = i;
            break;
        }
    }

    tui_node *result;
    if (cur_idx <= 0) {
        result = list.nodes[list.count - 1];  /* Wrap to last */
    } else {
        result = list.nodes[cur_idx - 1];
    }

    free(list.nodes);
    return result;
}

tui_node* tui_focus_find_next_in_group(tui_node *root, tui_node *current, const char *group)
{
    if (!root || !group) return NULL;

    focus_list list = {NULL, 0, 0};
    if (collect_focusable_nodes(root, &list, group, NULL) != 0) {
        free(list.nodes);
        return NULL;  /* Allocation failed */
    }

    if (list.count == 0) {
        free(list.nodes);
        return NULL;
    }

    qsort(list.nodes, list.count, sizeof(tui_node*), compare_tab_index);

    int cur_idx = -1;
    for (int i = 0; i < list.count; i++) {
        if (list.nodes[i] == current) {
            cur_idx = i;
            break;
        }
    }

    tui_node *result;
    if (cur_idx < 0 || cur_idx >= list.count - 1) {
        result = list.nodes[0];
    } else {
        result = list.nodes[cur_idx + 1];
    }

    free(list.nodes);
    return result;
}

static tui_node* find_by_id_recursive(tui_node *node, const char *id)
{
    if (!node || !id) return NULL;

    if (node->id && strcmp(node->id, id) == 0) {
        return node;
    }

    for (int i = 0; i < node->child_count; i++) {
        tui_node *found = find_by_id_recursive(node->children[i], id);
        if (found) return found;
    }

    return NULL;
}

tui_node* tui_focus_find_by_id(tui_node *root, const char *id)
{
    return find_by_id_recursive(root, id);
}

tui_node* tui_focus_find_first(tui_node *root)
{
    if (!root) return NULL;

    focus_list list = {NULL, 0, 0};
    if (collect_focusable_nodes(root, &list, NULL, NULL) != 0) {
        free(list.nodes);
        return NULL;  /* Allocation failed */
    }

    if (list.count == 0) {
        free(list.nodes);
        return NULL;
    }

    qsort(list.nodes, list.count, sizeof(tui_node*), compare_tab_index);

    tui_node *result = list.nodes[0];
    free(list.nodes);
    return result;
}

/* ----------------------------------------------------------------
 * Hit Testing for Mouse Events
 * ---------------------------------------------------------------- */

int tui_node_contains_point(tui_node *node, int x, int y)
{
    if (!node) return 0;

    /* Calculate absolute position by accumulating parent positions.
     * Start with the node's own position and add each ancestor's position. */
    float abs_x = node->x;
    float abs_y = node->y;
    tui_node *p = node->parent;
    while (p) {
        abs_x += p->x;
        abs_y += p->y;
        p = p->parent;
    }

    /* Check if point is within node's absolute bounds */
    return (x >= abs_x && x < abs_x + node->width &&
            y >= abs_y && y < abs_y + node->height);
}

/*
 * Recursive helper for hit testing.
 * Returns the deepest node containing the point, or NULL.
 * Depth parameter prevents stack overflow on deeply nested trees.
 */
static tui_node* hit_test_recursive(tui_node *node, int x, int y,
                                     float parent_x, float parent_y, int depth)
{
    if (!node || depth >= MAX_TREE_DEPTH) return NULL;

    /* Calculate absolute position of this node */
    float abs_x = parent_x + node->x;
    float abs_y = parent_y + node->y;

    /* Check if point is in this node's bounds */
    if (x < abs_x || x >= abs_x + node->width ||
        y < abs_y || y >= abs_y + node->height) {
        return NULL;  /* Point not in this node */
    }

    /* Point is in this node - check children (deeper first) */
    /* Children are rendered in order, so later children appear on top */
    /* Search in reverse order to find topmost child first */
    for (int i = node->child_count - 1; i >= 0; i--) {
        tui_node *hit = hit_test_recursive(node->children[i], x, y,
                                           abs_x, abs_y, depth + 1);
        if (hit) return hit;
    }

    /* No child contains the point, return this node */
    return node;
}

tui_node* tui_node_hit_test(tui_node *root, int x, int y)
{
    if (!root) return NULL;
    return hit_test_recursive(root, x, y, 0, 0, 0);
}

/*
 * Helper: collect all nodes containing a point (from root to leaf)
 * Depth parameter prevents stack overflow on deeply nested trees.
 */
static void collect_hit_nodes(tui_node *node, int x, int y, float parent_x, float parent_y,
                              tui_node ***nodes, int *count, int *capacity, int depth)
{
    if (!node || depth >= MAX_TREE_DEPTH) return;

    float abs_x = parent_x + node->x;
    float abs_y = parent_y + node->y;

    /* Check if point is in this node's bounds */
    if (x < abs_x || x >= abs_x + node->width ||
        y < abs_y || y >= abs_y + node->height) {
        return;  /* Point not in this node */
    }

    /* Add this node to the list (with overflow protection) */
    if (*count >= *capacity) {
        int new_cap = *capacity * 2;
        if (new_cap < 16) new_cap = 16;
        /* Check for overflow before allocation */
        if (new_cap > INT_MAX / (int)sizeof(tui_node*)) {
            return;  /* Would overflow, stop */
        }
        tui_node **new_nodes = realloc(*nodes, (size_t)new_cap * sizeof(tui_node*));
        if (!new_nodes) return;  /* Allocation failure, just stop */
        *nodes = new_nodes;
        *capacity = new_cap;
    }
    (*nodes)[(*count)++] = node;

    /* Recurse to children */
    for (int i = 0; i < node->child_count; i++) {
        collect_hit_nodes(node->children[i], x, y, abs_x, abs_y,
                          nodes, count, capacity, depth + 1);
    }
}

tui_node** tui_node_hit_test_all(tui_node *root, int x, int y, int *count)
{
    if (!root || !count) {
        if (count) *count = 0;
        return NULL;
    }

    tui_node **nodes = NULL;
    int capacity = 0;
    *count = 0;

    collect_hit_nodes(root, x, y, 0, 0, &nodes, count, &capacity, 0);

    if (*count == 0) {
        free(nodes);
        return NULL;
    }

    return nodes;
}
