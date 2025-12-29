/*
  +----------------------------------------------------------------------+
  | ext-tui: Headless test renderer                                      |
  +----------------------------------------------------------------------+
*/

#include "renderer.h"
#include "../text/measure.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define INITIAL_INPUT_CAPACITY 256

tui_test_renderer* tui_test_renderer_create(int width, int height)
{
    if (width <= 0 || height <= 0) return NULL;
    if (width > 1000 || height > 1000) return NULL;

    tui_test_renderer *renderer = calloc(1, sizeof(tui_test_renderer));
    if (!renderer) return NULL;

    renderer->width = width;
    renderer->height = height;

    renderer->buffer = tui_buffer_create(width, height);
    if (!renderer->buffer) {
        free(renderer);
        return NULL;
    }

    renderer->input_queue_capacity = INITIAL_INPUT_CAPACITY;
    renderer->input_queue = calloc(renderer->input_queue_capacity, 1);
    if (!renderer->input_queue) {
        tui_buffer_destroy(renderer->buffer);
        free(renderer);
        return NULL;
    }

    return renderer;
}

void tui_test_renderer_destroy(tui_test_renderer *renderer)
{
    if (!renderer) return;

    tui_buffer_destroy(renderer->buffer);
    free(renderer->input_queue);

    if (renderer->root) {
        tui_node_destroy(renderer->root);
    }

    /* Note: app is owned externally, don't destroy here */

    free(renderer);
}

/* Forward declaration from app.c for rendering */
extern void tui_app_render_node_to_buffer(tui_buffer *buffer, tui_node *node,
                                           int offset_x, int offset_y,
                                           int clip_x, int clip_y,
                                           int clip_w, int clip_h);

void tui_test_renderer_render(tui_test_renderer *renderer, tui_node *root)
{
    if (!renderer || !root) return;

    /* Free previous root if any (prevents memory leak on re-render) */
    if (renderer->root) {
        tui_node_destroy(renderer->root);
    }

    /* Store root reference */
    renderer->root = root;

    /* Calculate layout */
    tui_node_calculate_layout(root, (float)renderer->width, (float)renderer->height);

    /* Clear buffer */
    tui_buffer_clear(renderer->buffer);

    /* Render tree to buffer */
    tui_app_render_node_to_buffer(renderer->buffer, root, 0, 0,
                                   0, 0, renderer->width, renderer->height);

    renderer->frame_count++;
}

char** tui_test_renderer_get_output(tui_test_renderer *renderer, int *line_count)
{
    if (!renderer || !renderer->buffer || !line_count) return NULL;

    *line_count = renderer->height;

    char **lines = calloc(renderer->height, sizeof(char*));
    if (!lines) return NULL;

    for (int y = 0; y < renderer->height; y++) {
        /* Allocate worst case: 4 bytes per char (UTF-8) + null */
        lines[y] = calloc(renderer->width * 4 + 1, 1);
        if (!lines[y]) {
            tui_test_renderer_free_output(lines, y);
            *line_count = 0;
            return NULL;
        }

        int pos = 0;
        for (int x = 0; x < renderer->width; x++) {
            tui_cell *cell = tui_buffer_get_cell(renderer->buffer, x, y);
            if (cell && cell->codepoint != 0) {
                /* Skip continuation cells (wide chars) */
                if (cell->codepoint == (uint32_t)-1) continue;

                /* Encode codepoint to UTF-8 */
                uint32_t cp = cell->codepoint;
                if (cp < 0x80) {
                    lines[y][pos++] = (char)cp;
                } else if (cp < 0x800) {
                    lines[y][pos++] = (char)(0xC0 | (cp >> 6));
                    lines[y][pos++] = (char)(0x80 | (cp & 0x3F));
                } else if (cp < 0x10000) {
                    lines[y][pos++] = (char)(0xE0 | (cp >> 12));
                    lines[y][pos++] = (char)(0x80 | ((cp >> 6) & 0x3F));
                    lines[y][pos++] = (char)(0x80 | (cp & 0x3F));
                } else {
                    lines[y][pos++] = (char)(0xF0 | (cp >> 18));
                    lines[y][pos++] = (char)(0x80 | ((cp >> 12) & 0x3F));
                    lines[y][pos++] = (char)(0x80 | ((cp >> 6) & 0x3F));
                    lines[y][pos++] = (char)(0x80 | (cp & 0x3F));
                }
            } else {
                lines[y][pos++] = ' ';
            }
        }

        /* Trim trailing spaces */
        while (pos > 0 && lines[y][pos - 1] == ' ') {
            pos--;
        }
        lines[y][pos] = '\0';
    }

    return lines;
}

void tui_test_renderer_free_output(char **lines, int line_count)
{
    if (!lines) return;

    for (int i = 0; i < line_count; i++) {
        free(lines[i]);
    }
    free(lines);
}

char* tui_test_renderer_to_string(tui_test_renderer *renderer)
{
    if (!renderer) return NULL;

    int line_count;
    char **lines = tui_test_renderer_get_output(renderer, &line_count);
    if (!lines) return NULL;

    /* Calculate total size needed */
    size_t total_size = 0;
    for (int i = 0; i < line_count; i++) {
        total_size += strlen(lines[i]) + 1;  /* +1 for newline */
    }

    char *result = malloc(total_size + 1);
    if (!result) {
        tui_test_renderer_free_output(lines, line_count);
        return NULL;
    }

    /* Concatenate lines with newlines */
    size_t pos = 0;
    for (int i = 0; i < line_count; i++) {
        size_t len = strlen(lines[i]);
        memcpy(result + pos, lines[i], len);
        pos += len;
        result[pos++] = '\n';
    }

    /* Trim trailing empty lines */
    while (pos > 0 && (result[pos - 1] == '\n' || result[pos - 1] == ' ')) {
        pos--;
    }
    result[pos] = '\0';

    tui_test_renderer_free_output(lines, line_count);
    return result;
}

void tui_test_renderer_send_input(tui_test_renderer *renderer, const char *input, int len)
{
    if (!renderer || !input || len <= 0) return;

    /* Grow queue if needed */
    if (renderer->input_queue_len + len >= renderer->input_queue_capacity) {
        /* Check for overflow before doubling */
        if (renderer->input_queue_capacity > INT_MAX / 2) return;
        int new_capacity = renderer->input_queue_capacity * 2;
        while (new_capacity < renderer->input_queue_len + len + 1) {
            if (new_capacity > INT_MAX / 2) return;
            new_capacity *= 2;
        }
        char *new_queue = realloc(renderer->input_queue, new_capacity);
        if (!new_queue) return;
        renderer->input_queue = new_queue;
        renderer->input_queue_capacity = new_capacity;
    }

    memcpy(renderer->input_queue + renderer->input_queue_len, input, len);
    renderer->input_queue_len += len;
}

void tui_test_renderer_send_key(tui_test_renderer *renderer, int key_code)
{
    if (!renderer) return;

    /* Convert key code to escape sequence */
    const char *seq = NULL;

    switch (key_code) {
        case TUI_KEY_ENTER:     seq = "\r"; break;
        case TUI_KEY_TAB:       seq = "\t"; break;
        case TUI_KEY_ESCAPE:    seq = "\x1b"; break;
        case TUI_KEY_BACKSPACE: seq = "\x7f"; break;
        case TUI_KEY_UP:        seq = "\x1b[A"; break;
        case TUI_KEY_DOWN:      seq = "\x1b[B"; break;
        case TUI_KEY_RIGHT:     seq = "\x1b[C"; break;
        case TUI_KEY_LEFT:      seq = "\x1b[D"; break;
        case TUI_KEY_HOME:      seq = "\x1b[H"; break;
        case TUI_KEY_END:       seq = "\x1b[F"; break;
        case TUI_KEY_PAGE_UP:   seq = "\x1b[5~"; break;
        case TUI_KEY_PAGE_DOWN: seq = "\x1b[6~"; break;
        case TUI_KEY_DELETE:    seq = "\x1b[3~"; break;
        case TUI_KEY_INSERT:    seq = "\x1b[2~"; break;
        case TUI_KEY_F1:        seq = "\x1bOP"; break;
        case TUI_KEY_F2:        seq = "\x1bOQ"; break;
        case TUI_KEY_F3:        seq = "\x1bOR"; break;
        case TUI_KEY_F4:        seq = "\x1bOS"; break;
        case TUI_KEY_F5:        seq = "\x1b[15~"; break;
        case TUI_KEY_F6:        seq = "\x1b[17~"; break;
        case TUI_KEY_F7:        seq = "\x1b[18~"; break;
        case TUI_KEY_F8:        seq = "\x1b[19~"; break;
        case TUI_KEY_F9:        seq = "\x1b[20~"; break;
        case TUI_KEY_F10:       seq = "\x1b[21~"; break;
        case TUI_KEY_F11:       seq = "\x1b[23~"; break;
        case TUI_KEY_F12:       seq = "\x1b[24~"; break;
        default:
            /* Ctrl+key combinations */
            if (key_code >= 1 && key_code <= 26) {
                char ctrl[2] = { (char)key_code, '\0' };
                tui_test_renderer_send_input(renderer, ctrl, 1);
                return;
            }
            return;
    }

    if (seq) {
        tui_test_renderer_send_input(renderer, seq, strlen(seq));
    }
}

void tui_test_renderer_advance_frame(tui_test_renderer *renderer)
{
    if (!renderer || !renderer->app) return;

    /* Process queued input */
    if (renderer->input_queue_len > 0) {
        /* Parse and dispatch input events */
        /* This will be handled by the app's input handler */
        tui_app_inject_input(renderer->app, renderer->input_queue, renderer->input_queue_len);

        /* Clear queue */
        renderer->input_queue_len = 0;
    }

    /* Trigger re-render if app has a component */
    if (renderer->app && renderer->app->root_node) {
        /* Re-calculate layout and render */
        tui_node_calculate_layout(renderer->app->root_node,
                                   (float)renderer->width, (float)renderer->height);
        tui_buffer_clear(renderer->buffer);
        tui_app_render_node_to_buffer(renderer->buffer, renderer->app->root_node,
                                       0, 0, 0, 0, renderer->width, renderer->height);
        renderer->frame_count++;
    }
}

void tui_test_renderer_run_timers(tui_test_renderer *renderer, int ms)
{
    if (!renderer || !renderer->app || ms <= 0) return;

    renderer->elapsed_ms += ms;

    /* Tick timers in the event loop */
    if (renderer->app->loop) {
        tui_loop_tick_timers(renderer->app->loop, ms);
    }

    /* Advance frame to process any timer callbacks */
    tui_test_renderer_advance_frame(renderer);
}

void tui_test_renderer_set_app(tui_test_renderer *renderer, tui_app *app)
{
    if (renderer) {
        renderer->app = app;
    }
}
