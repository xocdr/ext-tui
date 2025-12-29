/*
  +----------------------------------------------------------------------+
  | ext-tui: Virtual DOM reconciler                                     |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_RECONCILER_H
#define TUI_RECONCILER_H

#include "node.h"

typedef enum {
    TUI_DIFF_NONE,
    TUI_DIFF_CREATE,
    TUI_DIFF_UPDATE,
    TUI_DIFF_DELETE,
    TUI_DIFF_REPLACE,
    TUI_DIFF_REORDER
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
