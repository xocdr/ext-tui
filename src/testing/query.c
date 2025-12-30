/*
  +----------------------------------------------------------------------+
  | ext-tui: Test query functions                                        |
  +----------------------------------------------------------------------+
*/

#include "query.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_QUERY_DEPTH 256
#define INITIAL_RESULTS_CAPACITY 16

/* Internal: recursive search by ID with depth limit */
static tui_node* find_by_id_impl(tui_node *node, const char *id, int depth)
{
    if (!node || !id || depth > MAX_QUERY_DEPTH) return NULL;

    if (node->id && strcmp(node->id, id) == 0) {
        return node;
    }

    if (node->children) {
        for (int i = 0; i < node->child_count; i++) {
            tui_node *found = find_by_id_impl(node->children[i], id, depth + 1);
            if (found) return found;
        }
    }

    return NULL;
}

tui_node* tui_test_find_by_id(tui_node *root, const char *id)
{
    return find_by_id_impl(root, id, 0);
}

/* Internal: recursive search by text with depth limit */
static void find_by_text_impl(tui_node *node, const char *text,
                               tui_node ***results, int *count, int *capacity,
                               int depth)
{
    if (!node || !text || depth > MAX_QUERY_DEPTH) return;

    /* Check if this node contains the text */
    if (node->type == TUI_NODE_TEXT && node->text) {
        if (strstr(node->text, text) != NULL) {
            /* Add to results */
            if (*count >= *capacity) {
                /* Check for overflow before doubling */
                if (*capacity > INT_MAX / 2) return;
                int new_capacity = *capacity * 2;
                tui_node **new_results = realloc(*results, new_capacity * sizeof(tui_node*));
                if (!new_results) return;
                *results = new_results;
                *capacity = new_capacity;
            }
            (*results)[(*count)++] = node;
        }
    }

    /* Recurse into children */
    if (node->children) {
        for (int i = 0; i < node->child_count; i++) {
            find_by_text_impl(node->children[i], text, results, count, capacity, depth + 1);
        }
    }
}

tui_node** tui_test_find_by_text(tui_node *root, const char *text, int *count)
{
    if (!root || !text || !count) {
        if (count) *count = 0;
        return NULL;
    }

    *count = 0;
    int capacity = INITIAL_RESULTS_CAPACITY;
    tui_node **results = calloc(capacity, sizeof(tui_node*));
    if (!results) return NULL;

    find_by_text_impl(root, text, &results, count, &capacity, 0);

    if (*count == 0) {
        free(results);
        return NULL;
    }

    return results;
}

/* Internal: recursive search for focusable nodes with depth limit */
static void find_focusable_impl(tui_node *node,
                                 tui_node ***results, int *count, int *capacity,
                                 int depth)
{
    if (!node || depth > MAX_QUERY_DEPTH) return;

    if (node->focusable) {
        if (*count >= *capacity) {
            /* Check for overflow before doubling */
            if (*capacity > INT_MAX / 2) return;
            int new_capacity = *capacity * 2;
            tui_node **new_results = realloc(*results, new_capacity * sizeof(tui_node*));
            if (!new_results) return;
            *results = new_results;
            *capacity = new_capacity;
        }
        (*results)[(*count)++] = node;
    }

    if (node->children) {
        for (int i = 0; i < node->child_count; i++) {
            find_focusable_impl(node->children[i], results, count, capacity, depth + 1);
        }
    }
}

tui_node** tui_test_find_focusable(tui_node *root, int *count)
{
    if (!root || !count) {
        if (count) *count = 0;
        return NULL;
    }

    *count = 0;
    int capacity = INITIAL_RESULTS_CAPACITY;
    tui_node **results = calloc(capacity, sizeof(tui_node*));
    if (!results) return NULL;

    find_focusable_impl(root, &results, count, &capacity, 0);

    if (*count == 0) {
        free(results);
        return NULL;
    }

    return results;
}

/* Internal: recursive search for focused node with depth limit */
static tui_node* find_focused_impl(tui_node *node, int depth)
{
    if (!node || depth > MAX_QUERY_DEPTH) return NULL;

    if (node->focused) {
        return node;
    }

    if (node->children) {
        for (int i = 0; i < node->child_count; i++) {
            tui_node *found = find_focused_impl(node->children[i], depth + 1);
            if (found) return found;
        }
    }

    return NULL;
}

tui_node* tui_test_find_focused(tui_node *root)
{
    return find_focused_impl(root, 0);
}

void tui_test_get_node_info(tui_node *node, tui_test_node_info *info)
{
    if (!info) return;

    memset(info, 0, sizeof(tui_test_node_info));

    if (!node) return;

    info->id = node->id;
    info->text = node->text;
    info->x = (int)node->x;
    info->y = (int)node->y;
    info->width = (int)node->width;
    info->height = (int)node->height;
    info->focusable = node->focusable;
    info->focused = node->focused;

    switch (node->type) {
        case TUI_NODE_BOX:     info->type = "box"; break;
        case TUI_NODE_TEXT:    info->type = "text"; break;
        case TUI_NODE_STATIC:  info->type = "static"; break;
        case TUI_NODE_NEWLINE: info->type = "newline"; break;
        case TUI_NODE_SPACER:  info->type = "spacer"; break;
        default:               info->type = "unknown"; break;
    }
}
