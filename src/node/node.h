/*
  +----------------------------------------------------------------------+
  | ext-tui: Virtual DOM node                                           |
  +----------------------------------------------------------------------+
  | Defines the node types and tree structure for TUI layouts.          |
  | Nodes form a tree with Yoga layout integration for flexbox.         |
  |                                                                      |
  | Memory Management: Nodes are allocated on the heap. Call            |
  | tui_node_destroy() to free a node and all its children recursively. |
  |                                                                      |
  | Thread Safety: NOT thread-safe. All operations must be on the       |
  | same thread.                                                        |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_NODE_H
#define TUI_NODE_H

#include <stdint.h>
#include <yoga/Yoga.h>
#include "../text/wrap.h"

/**
 * Node types in the virtual DOM tree.
 */
typedef enum {
    TUI_NODE_BOX,         /* Flexbox container */
    TUI_NODE_TEXT,        /* Text content */
    TUI_NODE_STATIC,      /* Static output (renders above dynamic content) */
    TUI_NODE_NEWLINE,     /* Blank line(s) */
    TUI_NODE_SPACER,      /* Flexible space (flexGrow: 1) */
    TUI_NODE_TRANSFORM    /* Text transformation wrapper */
} tui_node_type;

/**
 * RGB color with set flag.
 */
typedef struct {
    uint8_t r, g, b;      /* RGB components (0-255) */
    uint8_t is_set;       /* 1 if color is set, 0 for default/inherit */
} tui_color;

/**
 * Text styling attributes.
 */
typedef struct {
    tui_color fg;         /* Foreground color */
    tui_color bg;         /* Background color */
    uint8_t bold;         /* Bold text */
    uint8_t dim;          /* Dim/faint text */
    uint8_t italic;       /* Italic text */
    uint8_t underline;    /* Underlined text */
    uint8_t inverse;      /* Swap fg/bg colors */
    uint8_t strikethrough;/* Strikethrough text */
} tui_style;

/* Text wrap modes - defined in text/wrap.h */
/* TUI_WRAP_NONE, TUI_WRAP_CHAR, TUI_WRAP_WORD, TUI_WRAP_WORD_CHAR */

/**
 * Border drawing styles.
 */
typedef enum {
    TUI_BORDER_NONE,      /* No border */
    TUI_BORDER_SINGLE,    /* Single line: ─│┐└ */
    TUI_BORDER_DOUBLE,    /* Double line: ═║╗╚ */
    TUI_BORDER_ROUND,     /* Rounded corners: ─│╮╰ */
    TUI_BORDER_BOLD,      /* Bold line: ━┃┓┗ */
    TUI_BORDER_DASHED     /* Dashed line: ┄┆ */
} tui_border_style;

/**
 * Virtual DOM node.
 */
typedef struct tui_node {
    tui_node_type type;           /* Node type */
    char *key;                    /* Node identity for reconciler */
    char *id;                     /* ID for focus-by-id */
    tui_style style;              /* Text styling */

    /* For text nodes */
    char *text;                   /* Text content (NULL for box nodes) */
    tui_wrap_mode wrap_mode;      /* Text wrapping mode */

    /* For box nodes with borders */
    tui_border_style border_style;
    tui_color border_color;       /* Default border color */
    tui_color border_top_color;   /* Per-side border colors (override default) */
    tui_color border_right_color;
    tui_color border_bottom_color;
    tui_color border_left_color;

    /* Focus management */
    int focusable;                /* Whether node can receive focus */
    int focused;                  /* Currently focused */
    int tab_index;                /* -1 = skip, 0+ = explicit order */
    char *focus_group;            /* Group name for scoped tabbing */
    int auto_focus;               /* Focus on mount */
    int focus_trap;               /* Trap focus within this container */
    int show_cursor;              /* Show terminal cursor when focused */

    /* Hyperlink (OSC 8) */
    char *hyperlink_url;          /* Link URL */
    char *hyperlink_id;           /* Link ID (optional) */

    /* Yoga layout node */
    YGNodeRef yoga_node;          /* Associated Yoga node */

    /* Tree structure */
    struct tui_node *parent;      /* Parent node (NULL for root) */
    struct tui_node **children;   /* Child node array */
    int child_count;              /* Number of children */
    int child_capacity;           /* Allocated capacity */
    int children_from_pool;       /* 1 if children array is from pool */

    /* Computed layout (from Yoga) */
    float x, y, width, height;    /* Position and size in characters */

    /* Layout dirty flag */
    int layout_dirty;             /* Set by dirtied callback */

    /* For STATIC nodes */
    int static_items_rendered;    /* Track rendered items */

    /* For NEWLINE nodes */
    int newline_count;            /* Number of blank lines */
} tui_node;

/* ================================================================
 * Lifecycle
 * ================================================================ */

/**
 * Create a box node (flexbox container).
 * @return New node, or NULL on allocation failure
 */
tui_node* tui_node_create_box(void);

/**
 * Create a text node with content.
 * @param text Text content (copied internally)
 * @return New node, or NULL on allocation failure
 */
tui_node* tui_node_create_text(const char *text);

/**
 * Create a static node (renders above dynamic content).
 * @return New node, or NULL on allocation failure
 */
tui_node* tui_node_create_static(void);

/**
 * Create a newline node (blank lines).
 * @param count Number of blank lines (>= 1)
 * @return New node, or NULL on allocation failure
 */
tui_node* tui_node_create_newline(int count);

/**
 * Create a spacer node (flexible space).
 * @return New node, or NULL on allocation failure
 */
tui_node* tui_node_create_spacer(void);

/**
 * Destroy node and all children recursively.
 * @param node Node to destroy (NULL-safe)
 */
void tui_node_destroy(tui_node *node);

/* ================================================================
 * ID and hyperlink management
 * ================================================================ */

/**
 * Set node ID for focus-by-id.
 * @param node Node to modify
 * @param id   ID string (copied internally)
 * @return 0 on success, -1 on allocation failure
 */
int tui_node_set_id(tui_node *node, const char *id);

/**
 * Set hyperlink (OSC 8) on node.
 * @param node Node to modify
 * @param url  Link URL (required)
 * @param id   Link ID (optional, may be NULL)
 * @return 0 on success, -1 on allocation failure
 */
int tui_node_set_hyperlink(tui_node *node, const char *url, const char *id);

/* ================================================================
 * Tree manipulation
 * ================================================================ */

/**
 * Append child to parent.
 * @param parent Parent node
 * @param child  Child node to append
 * @return 0 on success, -1 on failure
 */
int tui_node_append_child(tui_node *parent, tui_node *child);

/**
 * Remove child from parent.
 * Does not destroy the child node.
 * @param parent Parent node
 * @param child  Child node to remove
 */
void tui_node_remove_child(tui_node *parent, tui_node *child);

/**
 * Insert child before another child.
 * @param parent Parent node
 * @param child  Child node to insert
 * @param before Reference child (insert before this); if NULL, appends
 * @return 0 on success, -1 on failure (NULL params, 'before' not found, or OOM)
 */
int tui_node_insert_before(tui_node *parent, tui_node *child, tui_node *before);

/* ================================================================
 * Styling
 * ================================================================ */

/**
 * Set node text style.
 * @param node  Node to modify
 * @param style Style to apply (copied)
 */
void tui_node_set_style(tui_node *node, const tui_style *style);

/**
 * Yoga style property identifiers for tui_node_set_yoga_style().
 */
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

/**
 * Set a Yoga layout property on a node.
 * @param node     Node to modify
 * @param property Which property to set
 * @param value    Property value
 */
void tui_node_set_yoga_style(tui_node *node, tui_yoga_property property, float value);

/* ================================================================
 * Layout
 * ================================================================ */

/**
 * Calculate layout for node tree.
 * Updates x, y, width, height on all nodes.
 * @param root   Root node
 * @param width  Available width
 * @param height Available height
 */
void tui_node_calculate_layout(tui_node *root, float width, float height);

/**
 * Get the shared Yoga configuration.
 * @return Global Yoga config instance
 */
YGConfigRef tui_get_yoga_config(void);

/* ================================================================
 * Focus management
 * ================================================================ */

/**
 * Set focus group for scoped tabbing.
 * @param node  Node to modify
 * @param group Group name (copied internally)
 * @return 0 on success, -1 on allocation failure
 */
int tui_node_set_focus_group(tui_node *node, const char *group);

/**
 * Find next focusable node in tree.
 * @param root    Root of tree to search
 * @param current Currently focused node (NULL for first)
 * @return Next focusable node, or NULL if none
 */
tui_node* tui_focus_find_next(tui_node *root, tui_node *current);

/**
 * Find previous focusable node in tree.
 * @param root    Root of tree to search
 * @param current Currently focused node (NULL for last)
 * @return Previous focusable node, or NULL if none
 */
tui_node* tui_focus_find_prev(tui_node *root, tui_node *current);

/**
 * Find next focusable node within a focus group.
 * @param root    Root of tree to search
 * @param current Currently focused node
 * @param group   Focus group name
 * @return Next focusable node in group, or NULL if none
 */
tui_node* tui_focus_find_next_in_group(tui_node *root, tui_node *current, const char *group);

/**
 * Find node by ID.
 * @param root Root of tree to search
 * @param id   Node ID to find
 * @return Node with matching ID, or NULL if not found
 */
tui_node* tui_focus_find_by_id(tui_node *root, const char *id);

/**
 * Find first focusable node in tree.
 * @param root Root of tree to search
 * @return First focusable node, or NULL if none
 */
tui_node* tui_focus_find_first(tui_node *root);

/**
 * Find the focus trap container for a node.
 * @param node Node to start from
 * @return Nearest ancestor with focus_trap=1, or NULL if none
 */
tui_node* tui_focus_find_trap_container(tui_node *node);

/* ================================================================
 * Hit testing (for mouse events)
 * ================================================================ */

/**
 * Find the deepest node at coordinates.
 * @param root Root of tree to search
 * @param x    X coordinate (0-indexed)
 * @param y    Y coordinate (0-indexed)
 * @return Deepest node containing point, or NULL if none
 */
tui_node* tui_node_hit_test(tui_node *root, int x, int y);

/**
 * Find all nodes at coordinates (root to leaf order).
 * @param root  Root of tree to search
 * @param x     X coordinate (0-indexed)
 * @param y     Y coordinate (0-indexed)
 * @param count Output: number of nodes in returned array
 * @return Array of nodes (caller must free array, not nodes), or NULL if none
 */
tui_node** tui_node_hit_test_all(tui_node *root, int x, int y, int *count);

/**
 * Check if point is inside node's bounding box.
 * @param node Node to test
 * @param x    X coordinate
 * @param y    Y coordinate
 * @return 1 if point is inside, 0 otherwise
 */
int tui_node_contains_point(tui_node *node, int x, int y);

#endif /* TUI_NODE_H */
