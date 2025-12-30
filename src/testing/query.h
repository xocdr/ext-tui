/*
  +----------------------------------------------------------------------+
  | ext-tui: Test query functions                                        |
  +----------------------------------------------------------------------+
  | Provides functions to query the rendered node tree for testing.      |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_TESTING_QUERY_H
#define TUI_TESTING_QUERY_H

#include "../node/node.h"

/**
 * Find a node by its ID.
 *
 * @param root Root node to search from
 * @param id The ID to search for
 * @return Found node, or NULL if not found
 */
tui_node* tui_test_find_by_id(tui_node *root, const char *id);

/**
 * Find all nodes containing specific text.
 *
 * @param root Root node to search from
 * @param text Text to search for (substring match)
 * @param count Output: number of matches found
 * @return Array of matching nodes (caller must free array, not nodes)
 */
tui_node** tui_test_find_by_text(tui_node *root, const char *text, int *count);

/**
 * Find all focusable nodes.
 *
 * @param root Root node to search from
 * @param count Output: number of focusable nodes found
 * @return Array of focusable nodes (caller must free array, not nodes)
 */
tui_node** tui_test_find_focusable(tui_node *root, int *count);

/**
 * Find the currently focused node.
 *
 * @param root Root node to search from
 * @return Focused node, or NULL if none focused
 */
tui_node* tui_test_find_focused(tui_node *root);

/**
 * Get node info as a structure suitable for PHP.
 * Used to return node data to PHP code.
 */
typedef struct {
    const char *id;
    const char *type;      /* "box", "text", "static", "newline", "spacer" */
    const char *text;      /* For text nodes */
    int x, y;
    int width, height;
    int focusable;
    int focused;
} tui_test_node_info;

/**
 * Get info about a node.
 *
 * @param node The node to inspect
 * @param info Output: node information
 */
void tui_test_get_node_info(tui_node *node, tui_test_node_info *info);

#endif /* TUI_TESTING_QUERY_H */
