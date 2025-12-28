/*
  +----------------------------------------------------------------------+
  | ext-tui: Virtual DOM reconciler                                     |
  +----------------------------------------------------------------------+
*/

#include "reconciler.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_DIFF_CAPACITY 16

static tui_diff_result* diff_result_create(void)
{
    tui_diff_result *result = calloc(1, sizeof(tui_diff_result));
    if (!result) return NULL;

    result->capacity = INITIAL_DIFF_CAPACITY;
    result->ops = calloc(result->capacity, sizeof(tui_diff_op));
    return result;
}

static void diff_result_add(tui_diff_result *result, tui_diff_type type,
                            tui_node *old_node, tui_node *new_node, int index)
{
    if (result->count >= result->capacity) {
        int new_capacity = result->capacity * 2;
        tui_diff_op *new_ops = realloc(result->ops, new_capacity * sizeof(tui_diff_op));
        if (!new_ops) return;  /* Allocation failed */
        result->ops = new_ops;
        result->capacity = new_capacity;
    }

    tui_diff_op *op = &result->ops[result->count++];
    op->type = type;
    op->old_node = old_node;
    op->new_node = new_node;
    op->index = index;
}

static int nodes_same_type(tui_node *a, tui_node *b)
{
    if (!a || !b) return 0;
    return a->type == b->type;
}

static int nodes_same_key(tui_node *a, tui_node *b)
{
    if (!a || !b) return 0;
    if (!a->key && !b->key) return 1;
    if (!a->key || !b->key) return 0;
    return strcmp(a->key, b->key) == 0;
}

static void diff_children(tui_diff_result *result, tui_node *old_node, tui_node *new_node)
{
    int old_count = old_node ? old_node->child_count : 0;
    int new_count = new_node ? new_node->child_count : 0;

    /* Simple algorithm: compare by index */
    /* TODO: Implement key-based reconciliation for better performance */

    int max_count = old_count > new_count ? old_count : new_count;

    for (int i = 0; i < max_count; i++) {
        tui_node *old_child = i < old_count ? old_node->children[i] : NULL;
        tui_node *new_child = i < new_count ? new_node->children[i] : NULL;

        if (!old_child && new_child) {
            /* Create new node */
            diff_result_add(result, TUI_DIFF_CREATE, NULL, new_child, i);
        } else if (old_child && !new_child) {
            /* Delete old node */
            diff_result_add(result, TUI_DIFF_DELETE, old_child, NULL, i);
        } else if (!nodes_same_type(old_child, new_child)) {
            /* Replace with different type */
            diff_result_add(result, TUI_DIFF_REPLACE, old_child, new_child, i);
        } else {
            /* Same type, check for updates */
            /* TODO: Compare styles, text content, etc. */
            diff_result_add(result, TUI_DIFF_UPDATE, old_child, new_child, i);

            /* Recurse into children */
            diff_children(result, old_child, new_child);
        }
    }
}

tui_diff_result* tui_reconciler_diff(tui_node *old_tree, tui_node *new_tree)
{
    tui_diff_result *result = diff_result_create();
    if (!result) return NULL;

    if (!old_tree && new_tree) {
        diff_result_add(result, TUI_DIFF_CREATE, NULL, new_tree, 0);
    } else if (old_tree && !new_tree) {
        diff_result_add(result, TUI_DIFF_DELETE, old_tree, NULL, 0);
    } else if (old_tree && new_tree) {
        if (!nodes_same_type(old_tree, new_tree)) {
            diff_result_add(result, TUI_DIFF_REPLACE, old_tree, new_tree, 0);
        } else {
            diff_result_add(result, TUI_DIFF_UPDATE, old_tree, new_tree, 0);
            diff_children(result, old_tree, new_tree);
        }
    }

    return result;
}

void tui_reconciler_apply(tui_node *tree, tui_diff_result *diff)
{
    if (!diff) return;

    for (int i = 0; i < diff->count; i++) {
        tui_diff_op *op = &diff->ops[i];

        switch (op->type) {
            case TUI_DIFF_CREATE:
                /* Add new node to parent */
                if (op->new_node && op->new_node->parent) {
                    tui_node_append_child(op->new_node->parent, op->new_node);
                }
                break;

            case TUI_DIFF_DELETE:
                /* Remove node from parent and destroy it */
                if (op->old_node && op->old_node->parent) {
                    tui_node_remove_child(op->old_node->parent, op->old_node);
                    tui_node_destroy(op->old_node);
                }
                break;

            case TUI_DIFF_REPLACE:
                /* Replace old node with new node */
                if (op->old_node && op->new_node && op->old_node->parent) {
                    tui_node *parent = op->old_node->parent;
                    int index = op->index;

                    /* Remove old node */
                    tui_node_remove_child(parent, op->old_node);

                    /* Insert new node at same position */
                    if (index < parent->child_count) {
                        tui_node_insert_before(parent, op->new_node, parent->children[index]);
                    } else {
                        tui_node_append_child(parent, op->new_node);
                    }

                    tui_node_destroy(op->old_node);
                }
                break;

            case TUI_DIFF_UPDATE:
                /* Copy properties from new node to old node */
                if (op->old_node && op->new_node) {
                    /* Update style */
                    op->old_node->style = op->new_node->style;

                    /* Update text content for text nodes */
                    if (op->old_node->type == TUI_NODE_TEXT &&
                        op->new_node->type == TUI_NODE_TEXT) {
                        if (op->new_node->text) {
                            char *new_text = strdup(op->new_node->text);
                            if (new_text) {
                                free(op->old_node->text);
                                op->old_node->text = new_text;
                            }
                            /* On strdup failure, keep old text unchanged */
                        } else {
                            free(op->old_node->text);
                            op->old_node->text = NULL;
                        }
                    }
                }
                break;

            case TUI_DIFF_REORDER:
                /* Reorder children within parent */
                /* For now, handled by the combination of remove/insert */
                break;

            case TUI_DIFF_NONE:
            default:
                break;
        }
    }
}

void tui_reconciler_free_diff(tui_diff_result *diff)
{
    if (diff) {
        free(diff->ops);
        free(diff);
    }
}
