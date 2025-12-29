/*
  +----------------------------------------------------------------------+
  | ext-tui: Virtual DOM reconciler                                     |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_RECONCILER_H
#define TUI_RECONCILER_H

#include "node.h"

/*
 * Diff operation types as bit flags for composable effects.
 * A single operation can combine multiple flags (e.g., UPDATE | REORDER).
 * This is more memory efficient than separate array entries.
 */
typedef enum {
    TUI_DIFF_NONE    = 0,
    TUI_DIFF_CREATE  = 1 << 0,  /* Node needs to be created */
    TUI_DIFF_UPDATE  = 1 << 1,  /* Node properties need update */
    TUI_DIFF_DELETE  = 1 << 2,  /* Node needs to be deleted */
    TUI_DIFF_REPLACE = 1 << 3,  /* Node needs replacement (type change) */
    TUI_DIFF_REORDER = 1 << 4   /* Node needs repositioning */
} tui_diff_type;

typedef struct {
    tui_diff_type type;
    tui_node *old_node;
    tui_node *new_node;
    int old_index;   /* Position in old tree (-1 if N/A) */
    int new_index;   /* Position in new tree (-1 if N/A) */
} tui_diff_op;

typedef struct {
    tui_diff_op *ops;
    int count;
    int capacity;
} tui_diff_result;

/* Diff two trees */
tui_diff_result* tui_reconciler_diff(tui_node *old_tree, tui_node *new_tree);

/* Apply diff to tree */
void tui_reconciler_apply(tui_node *tree, tui_diff_result *diff);

/* Free diff result */
void tui_reconciler_free_diff(tui_diff_result *diff);

#endif /* TUI_RECONCILER_H */
