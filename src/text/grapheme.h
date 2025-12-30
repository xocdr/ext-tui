/*
  +----------------------------------------------------------------------+
  | ext-tui: Grapheme cluster segmentation                              |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
  | UAX #29 grapheme cluster boundaries:                                  |
  | - ZWJ sequences (emoji families, professions)                         |
  | - Regional indicator pairs (flag emoji)                               |
  | - Combining mark sequences                                            |
  | - Emoji modifier sequences (skin tones)                               |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_GRAPHEME_H
#define TUI_GRAPHEME_H

#include <stddef.h>
#include <stdint.h>

/**
 * Grapheme cluster iterator.
 * Iterates through grapheme clusters in a UTF-8 string.
 */
typedef struct {
    const char *text;    /* Original text pointer */
    size_t len;          /* Total length in bytes */
    size_t pos;          /* Current byte position */
} tui_grapheme_iter;

/**
 * Grapheme Break Property (simplified from UAX #29).
 */
typedef enum {
    TUI_GBP_OTHER = 0,
    TUI_GBP_CR,
    TUI_GBP_LF,
    TUI_GBP_CONTROL,
    TUI_GBP_EXTEND,          /* Combining marks, VS, etc. */
    TUI_GBP_ZWJ,
    TUI_GBP_REGIONAL_INDICATOR,
    TUI_GBP_PREPEND,
    TUI_GBP_SPACINGMARK,
    TUI_GBP_HANGUL_L,
    TUI_GBP_HANGUL_V,
    TUI_GBP_HANGUL_T,
    TUI_GBP_HANGUL_LV,
    TUI_GBP_HANGUL_LVT,
    TUI_GBP_EXTENDED_PICTOGRAPHIC
} tui_grapheme_break_property;

/**
 * Initialize a grapheme iterator.
 *
 * @param iter    Iterator to initialize
 * @param text    UTF-8 text to iterate
 * @param len     Length of text in bytes
 */
void tui_grapheme_iter_init(tui_grapheme_iter *iter, const char *text, size_t len);

/**
 * Get the next grapheme cluster.
 *
 * @param iter    Iterator
 * @param start   Output: pointer to start of grapheme cluster
 * @param len     Output: length of grapheme cluster in bytes
 * @return        1 if a grapheme was found, 0 if end of string
 */
int tui_grapheme_iter_next(tui_grapheme_iter *iter, const char **start, size_t *len);

/**
 * Get display width of a grapheme cluster.
 * Handles ZWJ sequences, modifiers, and regional indicators.
 *
 * @param grapheme  Pointer to grapheme cluster
 * @param len       Length in bytes
 * @return          Display width (typically 1 or 2)
 */
int tui_grapheme_width(const char *grapheme, size_t len);

/**
 * Count grapheme clusters in a string.
 *
 * @param str   UTF-8 string
 * @param len   Length in bytes (-1 for null-terminated)
 * @return      Number of grapheme clusters
 */
int tui_grapheme_count(const char *str, int len);

/**
 * Get grapheme break property for a codepoint.
 *
 * @param codepoint  Unicode codepoint
 * @return           Grapheme break property
 */
tui_grapheme_break_property tui_get_grapheme_break_property(uint32_t codepoint);

/**
 * Check if there's a grapheme boundary between two codepoints.
 * Implements UAX #29 grapheme cluster boundary rules.
 *
 * @param prev_gbp      Break property of previous codepoint
 * @param curr_gbp      Break property of current codepoint
 * @param ri_count      Number of regional indicators seen so far (for flags)
 * @param after_zwj     Whether previous codepoint was ZWJ
 * @return              1 if there's a boundary, 0 if not
 */
int tui_is_grapheme_boundary(tui_grapheme_break_property prev_gbp,
                              tui_grapheme_break_property curr_gbp,
                              int ri_count, int after_zwj);

#endif /* TUI_GRAPHEME_H */
