/*
  +----------------------------------------------------------------------+
  | ext-tui: Drag and drop support                                       |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
  | Provides:                                                             |
  | - Drag state tracking                                                 |
  | - Drag data transfer                                                  |
  | - Drop target detection                                               |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_DRAGDROP_H
#define TUI_DRAGDROP_H

#include <stddef.h>
#include <stdint.h>

/* Drag state */
typedef enum {
    TUI_DRAG_IDLE = 0,
    TUI_DRAG_STARTED,
    TUI_DRAG_DRAGGING,
    TUI_DRAG_ENDED
} tui_drag_state_type;

/* Drop effect */
typedef enum {
    TUI_DROP_NONE = 0,
    TUI_DROP_COPY,
    TUI_DROP_MOVE,
    TUI_DROP_LINK
} tui_drop_effect;

/* Forward declaration for node (defined elsewhere) */
struct tui_node;

/**
 * Drag data payload.
 */
typedef struct {
    char *type;         /* MIME type or custom type (e.g., "text/plain", "node") */
    char *data;         /* Serialized data */
    size_t data_len;    /* Data length */
} tui_drag_data;

/**
 * Drag state context.
 */
typedef struct {
    tui_drag_state_type state;  /* Current drag state */
    struct tui_node *source;    /* Source node being dragged */
    int start_x;                /* Drag start X position */
    int start_y;                /* Drag start Y position */
    int current_x;              /* Current X position */
    int current_y;              /* Current Y position */
    tui_drag_data *payload;     /* Data being dragged */
    tui_drop_effect allowed_effects;  /* Allowed drop effects */
    struct tui_node *drop_target;  /* Current drop target (if any) */
} tui_drag_context;

/**
 * Initialize drag context.
 */
void tui_drag_context_init(tui_drag_context *ctx);

/**
 * Free drag context resources.
 */
void tui_drag_context_free(tui_drag_context *ctx);

/**
 * Start a drag operation.
 *
 * @param ctx     Drag context
 * @param source  Source node
 * @param x       Start X position
 * @param y       Start Y position
 * @param type    Data type (will be copied)
 * @param data    Data payload (will be copied)
 * @param len     Data length
 * @return        0 on success, -1 on error
 */
int tui_drag_start(tui_drag_context *ctx, struct tui_node *source,
                   int x, int y, const char *type, const char *data, size_t len);

/**
 * Update drag position.
 *
 * @param ctx     Drag context
 * @param x       New X position
 * @param y       New Y position
 * @return        0 on success, -1 on error
 */
int tui_drag_move(tui_drag_context *ctx, int x, int y);

/**
 * End a drag operation (drop or cancel).
 *
 * @param ctx     Drag context
 * @param dropped Whether the drop was accepted
 * @return        0 on success, -1 on error
 */
int tui_drag_end(tui_drag_context *ctx, int dropped);

/**
 * Cancel drag operation.
 *
 * @param ctx     Drag context
 * @return        0 on success, -1 on error
 */
int tui_drag_cancel(tui_drag_context *ctx);

/**
 * Check if drag is in progress.
 */
int tui_drag_is_active(const tui_drag_context *ctx);

/**
 * Set current drop target.
 *
 * @param ctx     Drag context
 * @param target  Target node (or NULL if no valid target)
 */
void tui_drag_set_target(tui_drag_context *ctx, struct tui_node *target);

/**
 * Get drag data type.
 */
const char *tui_drag_get_type(const tui_drag_context *ctx);

/**
 * Get drag data.
 */
const char *tui_drag_get_data(const tui_drag_context *ctx, size_t *len);

/**
 * Create drag data.
 *
 * @param type    Data type
 * @param data    Data content
 * @param len     Data length
 * @return        New drag data (caller must free with tui_drag_data_free)
 */
tui_drag_data *tui_drag_data_create(const char *type, const char *data, size_t len);

/**
 * Free drag data.
 */
void tui_drag_data_free(tui_drag_data *payload);

/* Global drag context (for simple use cases) */
extern tui_drag_context tui_global_drag_ctx;

#endif /* TUI_DRAGDROP_H */
