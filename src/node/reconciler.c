/*
  +----------------------------------------------------------------------+
  | ext-tui: Virtual DOM reconciler                                     |
  | Key-based reconciliation for efficient updates                      |
  +----------------------------------------------------------------------+
*/

#include "reconciler.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define INITIAL_DIFF_CAPACITY 16
#define KEY_MAP_INITIAL_SIZE 32

/* Simple hash map for key -> node lookups */
typedef struct {
    char *key;
    tui_node *node;
    int old_index;
    int matched;  /* 1 if matched with new node */
} key_map_entry;

typedef struct {
    key_map_entry *entries;
    int count;
    int capacity;
} key_map;

static key_map* key_map_create(int initial_capacity)
{
    key_map *map = calloc(1, sizeof(key_map));
    if (!map) return NULL;

    map->capacity = initial_capacity > 0 ? initial_capacity : KEY_MAP_INITIAL_SIZE;
    map->entries = calloc(map->capacity, sizeof(key_map_entry));
    if (!map->entries) {
        free(map);
        return NULL;
    }
    return map;
}

static void key_map_destroy(key_map *map)
{
    if (map) {
        free(map->entries);
        free(map);
    }
}

static int key_map_add(key_map *map, const char *key, tui_node *node, int index)
{
    if (!map || !key) return 0;

    /* Grow if needed */
    if (map->count >= map->capacity) {
        int new_capacity = map->capacity * 2;
        key_map_entry *new_entries = realloc(map->entries,
            new_capacity * sizeof(key_map_entry));
        if (!new_entries) return 0;
        map->entries = new_entries;
        map->capacity = new_capacity;
    }

    key_map_entry *entry = &map->entries[map->count++];
    entry->key = (char *)key;  /* Don't copy, just reference */
    entry->node = node;
    entry->old_index = index;
    entry->matched = 0;
    return 1;
}

static key_map_entry* key_map_find(key_map *map, const char *key)
{
    if (!map || !key) return NULL;

    for (int i = 0; i < map->count; i++) {
        if (map->entries[i].key && strcmp(map->entries[i].key, key) == 0) {
            return &map->entries[i];
        }
    }
    return NULL;
}

static tui_diff_result* diff_result_create(void)
{
    tui_diff_result *result = calloc(1, sizeof(tui_diff_result));
    if (!result) return NULL;

    result->capacity = INITIAL_DIFF_CAPACITY;
    result->ops = calloc(result->capacity, sizeof(tui_diff_op));
    if (!result->ops) {
        free(result);
        return NULL;
    }
    return result;
}

/**
 * Add a diff operation to the result.
 * @return 0 on success, -1 on allocation failure
 */
static int diff_result_add(tui_diff_result *result, tui_diff_type type,
                           tui_node *old_node, tui_node *new_node,
                           int old_index, int new_index)
{
    if (!result) return -1;

    if (result->count >= result->capacity) {
        /* Check for overflow before doubling */
        if (result->capacity > INT_MAX / 2) return -1;
        int new_capacity = result->capacity * 2;
        tui_diff_op *new_ops = realloc(result->ops, new_capacity * sizeof(tui_diff_op));
        if (!new_ops) return -1;  /* Allocation failed */
        result->ops = new_ops;
        result->capacity = new_capacity;
    }

    tui_diff_op *op = &result->ops[result->count++];
    op->type = type;
    op->old_node = old_node;
    op->new_node = new_node;
    op->old_index = old_index;
    op->new_index = new_index;
    return 0;
}

static int nodes_same_type(tui_node *a, tui_node *b)
{
    if (!a || !b) return 0;
    return a->type == b->type;
}

static int has_any_keys(tui_node **children, int count)
{
    for (int i = 0; i < count; i++) {
        if (children[i] && children[i]->key) {
            return 1;
        }
    }
    return 0;
}

/*
 * Key-based reconciliation algorithm with React's lastPlacedIndex optimization:
 *
 * 1. Build a map of old children by key
 * 2. For each new child:
 *    - If it has a key, look up in map
 *    - If found and same type: UPDATE (and smart REORDER detection)
 *    - If found but different type: REPLACE
 *    - If not found: CREATE
 * 3. For unmatched old children: DELETE (batched)
 *
 * lastPlacedIndex optimization:
 * Instead of marking all position changes as reorders, we track the last
 * stable position. A node only needs to move if it appeared BEFORE the
 * last stable node in the old list. Nodes that maintain relative order
 * don't need explicit reorder operations.
 *
 * Example: old=[A,B,C,D] new=[A,C,D,B]
 * - A: matched at 0, lastPlacedIndex=0, no move
 * - C: matched at 2, 2 > lastPlacedIndex(0), lastPlacedIndex=2, no move
 * - D: matched at 3, 3 > lastPlacedIndex(2), lastPlacedIndex=3, no move
 * - B: matched at 1, 1 < lastPlacedIndex(3), NEEDS MOVE (only B moves!)
 */
static void diff_children_keyed(tui_diff_result *result,
                                 tui_node *old_node, tui_node *new_node)
{
    int old_count = old_node ? old_node->child_count : 0;
    int new_count = new_node ? new_node->child_count : 0;

    /* Build key map for old children */
    key_map *old_keys = key_map_create(old_count > 0 ? old_count : 1);
    if (!old_keys) return;

    for (int i = 0; i < old_count; i++) {
        tui_node *child = old_node->children[i];
        if (child && child->key) {
            key_map_add(old_keys, child->key, child, i);
        }
    }

    /* Track which old indices have been matched (for non-keyed fallback) */
    int *old_matched = calloc(old_count > 0 ? old_count : 1, sizeof(int));
    if (!old_matched) {
        key_map_destroy(old_keys);
        return;
    }

    /* React's lastPlacedIndex optimization:
     * Track the highest old index we've seen that doesn't need to move.
     * If a node's old index is less than this, it needs to be moved.
     */
    int last_placed_index = 0;

    /* Process new children */
    for (int new_idx = 0; new_idx < new_count; new_idx++) {
        tui_node *new_child = new_node->children[new_idx];
        if (!new_child) continue;

        tui_node *matched_old = NULL;
        int matched_old_idx = -1;

        if (new_child->key) {
            /* Keyed child: look up by key */
            key_map_entry *entry = key_map_find(old_keys, new_child->key);
            if (entry && !entry->matched) {
                matched_old = entry->node;
                matched_old_idx = entry->old_index;
                entry->matched = 1;
                old_matched[matched_old_idx] = 1;
            }
        } else {
            /* Non-keyed child: try to match by index if that old child is also non-keyed */
            if (new_idx < old_count) {
                tui_node *old_child = old_node->children[new_idx];
                if (old_child && !old_child->key && !old_matched[new_idx]) {
                    matched_old = old_child;
                    matched_old_idx = new_idx;
                    old_matched[new_idx] = 1;
                }
            }
        }

        if (matched_old) {
            /* Found a match */
            if (!nodes_same_type(matched_old, new_child)) {
                /* Different type: replace */
                diff_result_add(result, TUI_DIFF_REPLACE, matched_old, new_child,
                               matched_old_idx, new_idx);
            } else {
                /* Same type: check if reorder needed using lastPlacedIndex */
                tui_diff_type flags = TUI_DIFF_UPDATE;

                if (matched_old_idx < last_placed_index) {
                    /* Node appeared before the last stable position in old tree.
                     * It needs to be moved forward. */
                    flags |= TUI_DIFF_REORDER;
                } else {
                    /* Node maintains relative order, update lastPlacedIndex */
                    last_placed_index = matched_old_idx;
                }

                /* Add operation with combined flags */
                diff_result_add(result, flags, matched_old, new_child,
                               matched_old_idx, new_idx);

                /* Recurse into children */
                diff_children_keyed(result, matched_old, new_child);
            }
        } else {
            /* No match: create new */
            diff_result_add(result, TUI_DIFF_CREATE, NULL, new_child, -1, new_idx);
        }
    }

    /* Delete unmatched old children (collected, will be batched in apply) */
    for (int i = 0; i < old_count; i++) {
        if (!old_matched[i] && old_node->children[i]) {
            diff_result_add(result, TUI_DIFF_DELETE, old_node->children[i], NULL, i, -1);
        }
    }

    free(old_matched);
    key_map_destroy(old_keys);
}

/*
 * Simple index-based reconciliation for when no keys are present.
 * Faster than key-based when keys aren't used.
 */
static void diff_children_indexed(tui_diff_result *result,
                                   tui_node *old_node, tui_node *new_node)
{
    int old_count = old_node ? old_node->child_count : 0;
    int new_count = new_node ? new_node->child_count : 0;
    int max_count = old_count > new_count ? old_count : new_count;

    for (int i = 0; i < max_count; i++) {
        tui_node *old_child = i < old_count ? old_node->children[i] : NULL;
        tui_node *new_child = i < new_count ? new_node->children[i] : NULL;

        if (!old_child && new_child) {
            /* Create new node */
            diff_result_add(result, TUI_DIFF_CREATE, NULL, new_child, -1, i);
        } else if (old_child && !new_child) {
            /* Delete old node */
            diff_result_add(result, TUI_DIFF_DELETE, old_child, NULL, i, -1);
        } else if (!nodes_same_type(old_child, new_child)) {
            /* Replace with different type */
            diff_result_add(result, TUI_DIFF_REPLACE, old_child, new_child, i, i);
        } else {
            /* Same type: update */
            diff_result_add(result, TUI_DIFF_UPDATE, old_child, new_child, i, i);

            /* Recurse into children */
            diff_children_indexed(result, old_child, new_child);
        }
    }
}

/*
 * Choose reconciliation strategy based on whether keys are present.
 */
static void diff_children(tui_diff_result *result, tui_node *old_node, tui_node *new_node)
{
    int old_count = old_node ? old_node->child_count : 0;
    int new_count = new_node ? new_node->child_count : 0;

    /* Use key-based if any children have keys */
    int use_keyed = 0;
    if (old_count > 0 && has_any_keys(old_node->children, old_count)) {
        use_keyed = 1;
    }
    if (new_count > 0 && has_any_keys(new_node->children, new_count)) {
        use_keyed = 1;
    }

    if (use_keyed) {
        diff_children_keyed(result, old_node, new_node);
    } else {
        diff_children_indexed(result, old_node, new_node);
    }
}

tui_diff_result* tui_reconciler_diff(tui_node *old_tree, tui_node *new_tree)
{
    tui_diff_result *result = diff_result_create();
    if (!result) return NULL;

    if (!old_tree && new_tree) {
        diff_result_add(result, TUI_DIFF_CREATE, NULL, new_tree, -1, 0);
    } else if (old_tree && !new_tree) {
        diff_result_add(result, TUI_DIFF_DELETE, old_tree, NULL, 0, -1);
    } else if (old_tree && new_tree) {
        if (!nodes_same_type(old_tree, new_tree)) {
            diff_result_add(result, TUI_DIFF_REPLACE, old_tree, new_tree, 0, 0);
        } else {
            diff_result_add(result, TUI_DIFF_UPDATE, old_tree, new_tree, 0, 0);
            diff_children(result, old_tree, new_tree);
        }
    }

    return result;
}

void tui_reconciler_apply(tui_node *tree, tui_diff_result *diff)
{
    if (!diff) return;

    /*
     * Process operations in order using bit flag checks:
     * 1. First pass: DELETE and REPLACE (remove old nodes)
     * 2. Second pass: UPDATE (copy properties, preserves Yoga nodes)
     * 3. Third pass: REORDER (move nodes to new positions)
     * 4. Fourth pass: CREATE (add new nodes)
     *
     * With bit flags, a single operation can have multiple effects
     * (e.g., UPDATE | REORDER). We check each flag independently.
     */

    /* Pass 1: Deletions (batched - all deletions collected before destroy) */
    for (int i = 0; i < diff->count; i++) {
        tui_diff_op *op = &diff->ops[i];

        if (op->type & TUI_DIFF_DELETE) {
            if (op->old_node && op->old_node->parent) {
                tui_node_remove_child(op->old_node->parent, op->old_node);
                tui_node_destroy(op->old_node);
            }
        }
    }

    /* Pass 2: Updates (property sync - preserves Yoga nodes!) */
    for (int i = 0; i < diff->count; i++) {
        tui_diff_op *op = &diff->ops[i];

        if (op->type & TUI_DIFF_UPDATE) {
            if (op->old_node && op->new_node) {
                /* Update style */
                op->old_node->style = op->new_node->style;

                /* Update border properties */
                op->old_node->border_style = op->new_node->border_style;
                op->old_node->border_color = op->new_node->border_color;
                op->old_node->border_top_color = op->new_node->border_top_color;
                op->old_node->border_right_color = op->new_node->border_right_color;
                op->old_node->border_bottom_color = op->new_node->border_bottom_color;
                op->old_node->border_left_color = op->new_node->border_left_color;

                /* Update focus state */
                op->old_node->focusable = op->new_node->focusable;
                /* Note: focused state is managed by the app, not copied */

                /* Update text content for text nodes */
                if (op->old_node->type == TUI_NODE_TEXT &&
                    op->new_node->type == TUI_NODE_TEXT) {
                    /* Only update if text actually changed */
                    int text_changed = 0;
                    if (op->old_node->text && op->new_node->text) {
                        text_changed = strcmp(op->old_node->text, op->new_node->text) != 0;
                    } else {
                        text_changed = op->old_node->text != op->new_node->text;
                    }

                    if (text_changed) {
                        if (op->new_node->text) {
                            char *new_text = strdup(op->new_node->text);
                            if (new_text) {
                                free(op->old_node->text);
                                op->old_node->text = new_text;
                                /* Mark Yoga node dirty for text measurement */
                                if (op->old_node->yoga_node) {
                                    YGNodeMarkDirty(op->old_node->yoga_node);
                                }
                            }
                        } else {
                            free(op->old_node->text);
                            op->old_node->text = NULL;
                            if (op->old_node->yoga_node) {
                                YGNodeMarkDirty(op->old_node->yoga_node);
                            }
                        }
                    }
                }

                /* Update wrap mode */
                op->old_node->wrap_mode = op->new_node->wrap_mode;
            }
        }
    }

    /* Pass 3: Replacements */
    for (int i = 0; i < diff->count; i++) {
        tui_diff_op *op = &diff->ops[i];

        if (op->type & TUI_DIFF_REPLACE) {
            if (op->old_node && op->new_node && op->old_node->parent) {
                tui_node *parent = op->old_node->parent;
                int insert_index = op->new_index >= 0 ? op->new_index : 0;

                /* Remove old node */
                tui_node_remove_child(parent, op->old_node);

                /* Insert new node at target position */
                if (insert_index < parent->child_count) {
                    tui_node_insert_before(parent, op->new_node, parent->children[insert_index]);
                } else {
                    tui_node_append_child(parent, op->new_node);
                }

                tui_node_destroy(op->old_node);
            }
        }
    }

    /* Pass 4: Reorders - move nodes to new positions */
    for (int i = 0; i < diff->count; i++) {
        tui_diff_op *op = &diff->ops[i];

        if (op->type & TUI_DIFF_REORDER) {
            if (op->old_node && op->old_node->parent) {
                tui_node *parent = op->old_node->parent;
                tui_node *node = op->old_node;
                int target_index = op->new_index;

                /* Find current position */
                int current_index = -1;
                for (int j = 0; j < parent->child_count; j++) {
                    if (parent->children[j] == node) {
                        current_index = j;
                        break;
                    }
                }

                if (current_index >= 0 && current_index != target_index) {
                    /* Remove from current position */
                    tui_node_remove_child(parent, node);

                    /* Insert at target position */
                    if (target_index < parent->child_count) {
                        tui_node_insert_before(parent, node, parent->children[target_index]);
                    } else {
                        tui_node_append_child(parent, node);
                    }
                }
            }
        }
    }

    /* Pass 5: Creations */
    for (int i = 0; i < diff->count; i++) {
        tui_diff_op *op = &diff->ops[i];

        if (op->type & TUI_DIFF_CREATE) {
            if (op->new_node && op->new_node->parent) {
                tui_node *parent = op->new_node->parent;
                int target_index = op->new_index >= 0 ? op->new_index : parent->child_count;

                if (target_index < parent->child_count) {
                    tui_node_insert_before(parent, op->new_node, parent->children[target_index]);
                } else {
                    tui_node_append_child(parent, op->new_node);
                }
            }
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
