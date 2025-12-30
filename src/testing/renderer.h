/*
  +----------------------------------------------------------------------+
  | ext-tui: Headless test renderer                                      |
  +----------------------------------------------------------------------+
  | Provides headless rendering for automated testing of TUI components. |
  | No terminal I/O - pure in-memory buffer rendering.                   |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_TESTING_RENDERER_H
#define TUI_TESTING_RENDERER_H

#include "../render/buffer.h"
#include "../node/node.h"
#include "../app/app.h"

/* Key codes for test input simulation
 * Start at 100 to avoid conflict with Ctrl+key combinations (1-26) */
#define TUI_KEY_ENTER     100
#define TUI_KEY_TAB       101
#define TUI_KEY_ESCAPE    102
#define TUI_KEY_BACKSPACE 103
#define TUI_KEY_UP        104
#define TUI_KEY_DOWN      105
#define TUI_KEY_RIGHT     106
#define TUI_KEY_LEFT      107
#define TUI_KEY_HOME      108
#define TUI_KEY_END       109
#define TUI_KEY_PAGE_UP   110
#define TUI_KEY_PAGE_DOWN 111
#define TUI_KEY_DELETE    112
#define TUI_KEY_INSERT    113
#define TUI_KEY_F1        114
#define TUI_KEY_F2        115
#define TUI_KEY_F3        116
#define TUI_KEY_F4        117
#define TUI_KEY_F5        118
#define TUI_KEY_F6        119
#define TUI_KEY_F7        120
#define TUI_KEY_F8        121
#define TUI_KEY_F9        122
#define TUI_KEY_F10       123
#define TUI_KEY_F11       124
#define TUI_KEY_F12       125

/* Test renderer structure */
typedef struct {
    int width;
    int height;
    tui_buffer *buffer;
    tui_node *root;
    tui_app *app;           /* App instance for state/hooks */
    int frame_count;        /* Number of frames rendered */

    /* Input queue for simulated input */
    char *input_queue;
    int input_queue_len;
    int input_queue_capacity;

    /* Timer simulation */
    int elapsed_ms;         /* Simulated elapsed time */
} tui_test_renderer;

/**
 * Create a new headless test renderer.
 *
 * @param width Buffer width in characters
 * @param height Buffer height in characters
 * @return New test renderer, or NULL on error
 */
tui_test_renderer* tui_test_renderer_create(int width, int height);

/**
 * Destroy a test renderer and free all resources.
 *
 * @param renderer The test renderer to destroy
 */
void tui_test_renderer_destroy(tui_test_renderer *renderer);

/**
 * Render a node tree to the test buffer.
 *
 * @param renderer The test renderer
 * @param root Root node of the tree to render
 */
void tui_test_renderer_render(tui_test_renderer *renderer, tui_node *root);

/**
 * Get the rendered output as a 2D array of strings.
 * Each string is one row of the buffer.
 *
 * @param renderer The test renderer
 * @param line_count Output: number of lines
 * @return Array of strings (caller must free with tui_test_renderer_free_output)
 */
char** tui_test_renderer_get_output(tui_test_renderer *renderer, int *line_count);

/**
 * Free output array returned by tui_test_renderer_get_output.
 *
 * @param lines The lines array to free
 * @param line_count Number of lines
 */
void tui_test_renderer_free_output(char **lines, int line_count);

/**
 * Get the rendered output as a single string with newlines.
 *
 * @param renderer The test renderer
 * @return String (caller must free)
 */
char* tui_test_renderer_to_string(tui_test_renderer *renderer);

/**
 * Queue input to be processed on next frame.
 *
 * @param renderer The test renderer
 * @param input Input string (characters or escape sequences)
 * @param len Length of input
 */
void tui_test_renderer_send_input(tui_test_renderer *renderer, const char *input, int len);

/**
 * Queue a special key to be processed on next frame.
 *
 * @param renderer The test renderer
 * @param key_code Key code (TUI_KEY_* constant)
 */
void tui_test_renderer_send_key(tui_test_renderer *renderer, int key_code);

/**
 * Advance one frame - process input and re-render.
 *
 * @param renderer The test renderer
 */
void tui_test_renderer_advance_frame(tui_test_renderer *renderer);

/**
 * Advance simulated time and run due timers.
 *
 * @param renderer The test renderer
 * @param ms Milliseconds to advance
 */
void tui_test_renderer_run_timers(tui_test_renderer *renderer, int ms);

/**
 * Set the app instance for the test renderer (for hooks/state).
 *
 * @param renderer The test renderer
 * @param app The app instance
 */
void tui_test_renderer_set_app(tui_test_renderer *renderer, tui_app *app);

#endif /* TUI_TESTING_RENDERER_H */
