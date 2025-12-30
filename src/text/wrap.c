/*
  +----------------------------------------------------------------------+
  | ext-tui: Text wrapping                                              |
  +----------------------------------------------------------------------+
  | Provides word/character wrapping and text truncation utilities.     |
  +----------------------------------------------------------------------+
*/

#include "wrap.h"
#include "measure.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>

#define INITIAL_LINES 8

/**
 * Add a line to the wrapped text result.
 * @return 0 on success, -1 on allocation failure
 */
static int add_line(tui_wrapped_text *wrapped, const char *start, int len)
{
    if (!wrapped) return -1;

    if (wrapped->count >= wrapped->capacity) {
        /* Check for overflow before doubling */
        if (wrapped->capacity > INT_MAX / 2) return -1;
        int new_capacity = wrapped->capacity * 2;

        /* Check for size_t overflow in allocation */
        if ((size_t)new_capacity > SIZE_MAX / sizeof(char*)) return -1;

        char **new_lines = realloc(wrapped->lines, (size_t)new_capacity * sizeof(char*));
        if (!new_lines) return -1;

        wrapped->lines = new_lines;
        wrapped->capacity = new_capacity;
    }

    char *line = malloc((size_t)len + 1);
    if (!line) return -1;

    memcpy(line, start, (size_t)len);
    line[len] = '\0';
    wrapped->lines[wrapped->count] = line;
    wrapped->count++;
    return 0;
}

/**
 * Check if character is a word break point.
 */
static int is_word_break(char c)
{
    return isspace((unsigned char)c) || c == '-' || c == '\0';
}

/**
 * Wrap text to specified width.
 *
 * @param text Input text to wrap
 * @param width Maximum display width per line
 * @param mode Wrapping mode (none, word, char, word+char)
 * @return Wrapped text structure, or NULL on error
 */
tui_wrapped_text* tui_wrap_text(const char *text, int width, tui_wrap_mode mode)
{
    if (!text || width <= 0) return NULL;

    tui_wrapped_text *wrapped = calloc(1, sizeof(tui_wrapped_text));
    if (!wrapped) return NULL;

    wrapped->capacity = INITIAL_LINES;
    wrapped->lines = calloc((size_t)wrapped->capacity, sizeof(char*));
    if (!wrapped->lines) {
        free(wrapped);
        return NULL;
    }

    if (mode == TUI_WRAP_NONE) {
        /* No wrapping, just copy */
        if (add_line(wrapped, text, (int)strlen(text)) < 0) {
            tui_wrapped_text_free(wrapped);
            return NULL;
        }
        return wrapped;
    }

    const char *line_start = text;
    const char *p = text;
    const char *last_break = NULL;
    int current_width = 0;

    while (*p) {
        /* Handle newlines */
        if (*p == '\n') {
            if (add_line(wrapped, line_start, (int)(p - line_start)) < 0) {
                tui_wrapped_text_free(wrapped);
                return NULL;
            }
            p++;
            line_start = p;
            last_break = NULL;
            current_width = 0;
            continue;
        }

        /* Track word breaks */
        if (is_word_break(*p)) {
            last_break = p;
        }

        /* Get character width */
        uint32_t codepoint;
        int bytes = tui_utf8_decode(p, &codepoint);
        if (bytes <= 0) bytes = 1;  /* Safety: advance at least 1 byte */
        int char_width = tui_char_width(codepoint);

        /* Check if we need to wrap */
        if (current_width + char_width > width) {
            if (mode == TUI_WRAP_WORD || mode == TUI_WRAP_WORD_CHAR) {
                if (last_break && last_break > line_start) {
                    /* Wrap at word boundary */
                    if (add_line(wrapped, line_start, (int)(last_break - line_start)) < 0) {
                        tui_wrapped_text_free(wrapped);
                        return NULL;
                    }
                    p = last_break;
                    while (*p && isspace((unsigned char)*p)) p++;
                    line_start = p;
                    last_break = NULL;
                    current_width = 0;
                    continue;
                }
            }

            /* Wrap at character (or fallback) */
            if (mode == TUI_WRAP_CHAR || mode == TUI_WRAP_WORD_CHAR) {
                if (p > line_start) {
                    if (add_line(wrapped, line_start, (int)(p - line_start)) < 0) {
                        tui_wrapped_text_free(wrapped);
                        return NULL;
                    }
                    line_start = p;
                    last_break = NULL;
                    current_width = 0;
                    continue;
                }
            }
        }

        current_width += char_width;
        p += bytes;
    }

    /* Add remaining text */
    if (p > line_start) {
        if (add_line(wrapped, line_start, (int)(p - line_start)) < 0) {
            tui_wrapped_text_free(wrapped);
            return NULL;
        }
    }

    return wrapped;
}

/**
 * Free wrapped text structure.
 */
void tui_wrapped_text_free(tui_wrapped_text *wrapped)
{
    if (wrapped) {
        if (wrapped->lines) {
            for (int i = 0; i < wrapped->count; i++) {
                free(wrapped->lines[i]);
            }
            free(wrapped->lines);
        }
        free(wrapped);
    }
}

/**
 * Truncate text at end: "Hello World" -> "Hello Wo..."
 */
static char* truncate_end(const char *text, int width, const char *ellipsis)
{
    int ellipsis_width = tui_string_width(ellipsis);
    int target_width = width - ellipsis_width;

    if (target_width <= 0) {
        return strdup(ellipsis);
    }

    /* Find truncation point from start */
    const char *p = text;
    int current_width = 0;

    while (*p) {
        uint32_t codepoint;
        int bytes = tui_utf8_decode(p, &codepoint);
        if (bytes <= 0) bytes = 1;
        int char_width = tui_char_width(codepoint);

        if (current_width + char_width > target_width) {
            break;
        }

        current_width += char_width;
        p += bytes;
    }

    /* Build result */
    size_t prefix_len = (size_t)(p - text);
    size_t ellipsis_len = strlen(ellipsis);
    size_t result_len = prefix_len + ellipsis_len;

    char *result = malloc(result_len + 1);
    if (!result) return NULL;

    memcpy(result, text, prefix_len);
    memcpy(result + prefix_len, ellipsis, ellipsis_len + 1);

    return result;
}

/**
 * Truncate text at start: "Hello World" -> "...lo World"
 */
static char* truncate_start(const char *text, int width, const char *ellipsis)
{
    int ellipsis_width = tui_string_width(ellipsis);
    int target_width = width - ellipsis_width;

    if (target_width <= 0) {
        return strdup(ellipsis);
    }

    int text_width = tui_string_width(text);
    int skip_width = text_width - target_width;

    /* Find start point (skip skip_width characters) */
    const char *p = text;
    int current_width = 0;

    while (*p && current_width < skip_width) {
        uint32_t codepoint;
        int bytes = tui_utf8_decode(p, &codepoint);
        if (bytes <= 0) bytes = 1;
        current_width += tui_char_width(codepoint);
        p += bytes;
    }

    /* Build result */
    size_t ellipsis_len = strlen(ellipsis);
    size_t suffix_len = strlen(p);
    size_t result_len = ellipsis_len + suffix_len;

    char *result = malloc(result_len + 1);
    if (!result) return NULL;

    memcpy(result, ellipsis, ellipsis_len);
    memcpy(result + ellipsis_len, p, suffix_len + 1);

    return result;
}

/**
 * Truncate text in middle: "Hello World" -> "Hello...orld"
 */
static char* truncate_middle(const char *text, int width, const char *ellipsis)
{
    int ellipsis_width = tui_string_width(ellipsis);
    int target_width = width - ellipsis_width;

    if (target_width <= 0) {
        return strdup(ellipsis);
    }

    /* Split target_width in half for prefix and suffix */
    int prefix_target = target_width / 2;
    int suffix_target = target_width - prefix_target;

    /* Find prefix end */
    const char *p = text;
    int current_width = 0;

    while (*p) {
        uint32_t codepoint;
        int bytes = tui_utf8_decode(p, &codepoint);
        if (bytes <= 0) bytes = 1;
        int char_width = tui_char_width(codepoint);

        if (current_width + char_width > prefix_target) {
            break;
        }

        current_width += char_width;
        p += bytes;
    }
    const char *prefix_end = p;
    size_t prefix_len = (size_t)(prefix_end - text);

    /* Find suffix start (from the end) */
    int text_width = tui_string_width(text);
    int skip_width = text_width - suffix_target;
    p = text;
    current_width = 0;

    while (*p && current_width < skip_width) {
        uint32_t codepoint;
        int bytes = tui_utf8_decode(p, &codepoint);
        if (bytes <= 0) bytes = 1;
        current_width += tui_char_width(codepoint);
        p += bytes;
    }
    const char *suffix_start = p;
    size_t suffix_len = strlen(suffix_start);

    /* Build result */
    size_t ellipsis_len = strlen(ellipsis);
    size_t result_len = prefix_len + ellipsis_len + suffix_len;

    char *result = malloc(result_len + 1);
    if (!result) return NULL;

    memcpy(result, text, prefix_len);
    memcpy(result + prefix_len, ellipsis, ellipsis_len);
    memcpy(result + prefix_len + ellipsis_len, suffix_start, suffix_len + 1);

    return result;
}

/**
 * Truncate text to specified width with ellipsis and position control.
 *
 * @param text Input text
 * @param width Maximum display width
 * @param ellipsis Ellipsis string (default "...")
 * @param position Where to place ellipsis (start, middle, end)
 * @return Truncated string (caller must free), or NULL on error
 */
char* tui_truncate_text_ex(const char *text, int width, const char *ellipsis, tui_truncate_position position)
{
    if (!text) return NULL;
    if (!ellipsis) ellipsis = "...";

    int text_width = tui_string_width(text);
    if (text_width <= width) {
        return strdup(text);
    }

    switch (position) {
        case TUI_TRUNCATE_START:
            return truncate_start(text, width, ellipsis);
        case TUI_TRUNCATE_MIDDLE:
            return truncate_middle(text, width, ellipsis);
        case TUI_TRUNCATE_END:
        default:
            return truncate_end(text, width, ellipsis);
    }
}

/**
 * Truncate text to specified width with ellipsis.
 * Legacy function - calls tui_truncate_text_ex with TUI_TRUNCATE_END.
 *
 * @param text Input text
 * @param width Maximum display width
 * @param ellipsis Ellipsis string (default "...")
 * @return Truncated string (caller must free), or NULL on error
 */
char* tui_truncate_text(const char *text, int width, const char *ellipsis)
{
    return tui_truncate_text_ex(text, width, ellipsis, TUI_TRUNCATE_END);
}
