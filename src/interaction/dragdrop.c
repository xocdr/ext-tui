/*
  +----------------------------------------------------------------------+
  | ext-tui: Drag and drop support                                       |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "dragdrop.h"
#include <stdlib.h>
#include <string.h>

/* Global drag context */
tui_drag_context tui_global_drag_ctx = {0};

void tui_drag_context_init(tui_drag_context *ctx)
{
    if (!ctx) return;

    ctx->state = TUI_DRAG_IDLE;
    ctx->source = NULL;
    ctx->start_x = 0;
    ctx->start_y = 0;
    ctx->current_x = 0;
    ctx->current_y = 0;
    ctx->payload = NULL;
    ctx->allowed_effects = TUI_DROP_NONE;
    ctx->drop_target = NULL;
}

void tui_drag_context_free(tui_drag_context *ctx)
{
    if (!ctx) return;

    if (ctx->payload) {
        tui_drag_data_free(ctx->payload);
        ctx->payload = NULL;
    }

    ctx->state = TUI_DRAG_IDLE;
    ctx->source = NULL;
    ctx->drop_target = NULL;
}

tui_drag_data *tui_drag_data_create(const char *type, const char *data, size_t len)
{
    tui_drag_data *payload = malloc(sizeof(tui_drag_data));
    if (!payload) return NULL;

    payload->type = NULL;
    payload->data = NULL;
    payload->data_len = 0;

    if (type) {
        payload->type = strdup(type);
        if (!payload->type) {
            free(payload);
            return NULL;
        }
    }

    if (data && len > 0) {
        payload->data = malloc(len + 1);
        if (!payload->data) {
            free(payload->type);
            free(payload);
            return NULL;
        }
        memcpy(payload->data, data, len);
        payload->data[len] = '\0';
        payload->data_len = len;
    }

    return payload;
}

void tui_drag_data_free(tui_drag_data *payload)
{
    if (!payload) return;

    free(payload->type);
    free(payload->data);
    free(payload);
}

int tui_drag_start(tui_drag_context *ctx, struct tui_node *source,
                   int x, int y, const char *type, const char *data, size_t len)
{
    if (!ctx) return -1;

    /* Cancel any existing drag */
    if (ctx->state != TUI_DRAG_IDLE) {
        tui_drag_context_free(ctx);
        tui_drag_context_init(ctx);
    }

    ctx->source = source;
    ctx->start_x = x;
    ctx->start_y = y;
    ctx->current_x = x;
    ctx->current_y = y;

    /* Create payload */
    if (type || data) {
        ctx->payload = tui_drag_data_create(type, data, len);
        if (!ctx->payload) {
            return -1;
        }
    }

    ctx->state = TUI_DRAG_STARTED;
    ctx->allowed_effects = TUI_DROP_MOVE;  /* Default to move */

    return 0;
}

int tui_drag_move(tui_drag_context *ctx, int x, int y)
{
    if (!ctx) return -1;

    if (ctx->state == TUI_DRAG_IDLE) {
        return -1;  /* No drag in progress */
    }

    ctx->current_x = x;
    ctx->current_y = y;

    if (ctx->state == TUI_DRAG_STARTED) {
        ctx->state = TUI_DRAG_DRAGGING;
    }

    return 0;
}

int tui_drag_end(tui_drag_context *ctx, int dropped)
{
    if (!ctx) return -1;

    if (ctx->state == TUI_DRAG_IDLE) {
        return -1;
    }

    ctx->state = TUI_DRAG_ENDED;

    /* If dropped, keep payload for retrieval by drop handler */
    /* Otherwise, we could mark it as cancelled */

    (void)dropped;  /* Used by caller to determine action */

    return 0;
}

int tui_drag_cancel(tui_drag_context *ctx)
{
    if (!ctx) return -1;

    tui_drag_context_free(ctx);
    tui_drag_context_init(ctx);

    return 0;
}

int tui_drag_is_active(const tui_drag_context *ctx)
{
    if (!ctx) return 0;

    return (ctx->state == TUI_DRAG_STARTED || ctx->state == TUI_DRAG_DRAGGING);
}

void tui_drag_set_target(tui_drag_context *ctx, struct tui_node *target)
{
    if (!ctx) return;

    ctx->drop_target = target;
}

const char *tui_drag_get_type(const tui_drag_context *ctx)
{
    if (!ctx || !ctx->payload) return NULL;

    return ctx->payload->type;
}

const char *tui_drag_get_data(const tui_drag_context *ctx, size_t *len)
{
    if (!ctx || !ctx->payload) {
        if (len) *len = 0;
        return NULL;
    }

    if (len) *len = ctx->payload->data_len;
    return ctx->payload->data;
}
