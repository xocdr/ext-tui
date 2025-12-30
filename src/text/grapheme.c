/*
  +----------------------------------------------------------------------+
  | ext-tui: Grapheme cluster segmentation                              |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "grapheme.h"
#include "measure.h"
#include <string.h>

/* ----------------------------------------------------------------
 * Grapheme Break Property lookup
 * Based on Unicode 15.0 UAX #29
 * ---------------------------------------------------------------- */

tui_grapheme_break_property tui_get_grapheme_break_property(uint32_t cp)
{
    /* CR, LF */
    if (cp == 0x000D) return TUI_GBP_CR;
    if (cp == 0x000A) return TUI_GBP_LF;

    /* Control characters (C0, C1, DEL, and specific format chars) */
    if (cp < 0x0020 || (cp >= 0x007F && cp <= 0x009F)) {
        return TUI_GBP_CONTROL;
    }

    /* Zero-Width Joiner */
    if (cp == 0x200D) return TUI_GBP_ZWJ;

    /* Regional Indicators (flags): U+1F1E6 to U+1F1FF */
    if (cp >= 0x1F1E6 && cp <= 0x1F1FF) {
        return TUI_GBP_REGIONAL_INDICATOR;
    }

    /* Hangul syllable types */
    /* L: Hangul Jamo Leading */
    if ((cp >= 0x1100 && cp <= 0x115F) ||
        (cp >= 0xA960 && cp <= 0xA97C)) {
        return TUI_GBP_HANGUL_L;
    }

    /* V: Hangul Jamo Vowel */
    if ((cp >= 0x1160 && cp <= 0x11A7) ||
        (cp >= 0xD7B0 && cp <= 0xD7C6)) {
        return TUI_GBP_HANGUL_V;
    }

    /* T: Hangul Jamo Trailing */
    if ((cp >= 0x11A8 && cp <= 0x11FF) ||
        (cp >= 0xD7CB && cp <= 0xD7FB)) {
        return TUI_GBP_HANGUL_T;
    }

    /* LV: Hangul syllable blocks (precomposed LV syllables) */
    if (cp >= 0xAC00 && cp <= 0xD7A3) {
        /* In the Hangul Syllables block, LV syllables have (cp - 0xAC00) % 28 == 0 */
        if ((cp - 0xAC00) % 28 == 0) {
            return TUI_GBP_HANGUL_LV;
        }
        return TUI_GBP_HANGUL_LVT;
    }

    /* Extend: Combining marks, variation selectors, etc. */
    /* Combining Diacritical Marks */
    if (cp >= 0x0300 && cp <= 0x036F) return TUI_GBP_EXTEND;
    /* Combining Diacritical Marks Extended */
    if (cp >= 0x1AB0 && cp <= 0x1AFF) return TUI_GBP_EXTEND;
    /* Combining Diacritical Marks Supplement */
    if (cp >= 0x1DC0 && cp <= 0x1DFF) return TUI_GBP_EXTEND;
    /* Combining Half Marks */
    if (cp >= 0xFE20 && cp <= 0xFE2F) return TUI_GBP_EXTEND;
    /* Variation Selectors */
    if (cp >= 0xFE00 && cp <= 0xFE0F) return TUI_GBP_EXTEND;
    /* Variation Selectors Supplement */
    if (cp >= 0xE0100 && cp <= 0xE01EF) return TUI_GBP_EXTEND;
    /* Combining Diacritical Marks for Symbols */
    if (cp >= 0x20D0 && cp <= 0x20FF) return TUI_GBP_EXTEND;
    /* Arabic combining marks */
    if (cp >= 0x064B && cp <= 0x065F) return TUI_GBP_EXTEND;
    if (cp >= 0x0610 && cp <= 0x061A) return TUI_GBP_EXTEND;
    /* Hebrew combining marks */
    if (cp >= 0x0591 && cp <= 0x05BD) return TUI_GBP_EXTEND;
    if (cp == 0x05BF || cp == 0x05C1 || cp == 0x05C2 ||
        cp == 0x05C4 || cp == 0x05C5 || cp == 0x05C7) {
        return TUI_GBP_EXTEND;
    }
    /* Devanagari etc. */
    if (cp >= 0x0900 && cp <= 0x0903) return TUI_GBP_EXTEND;
    if (cp >= 0x093A && cp <= 0x094F) return TUI_GBP_EXTEND;
    /* Emoji modifiers (skin tones) */
    if (cp >= 0x1F3FB && cp <= 0x1F3FF) return TUI_GBP_EXTEND;
    /* Enclosing marks */
    if (cp == 0x20E3) return TUI_GBP_EXTEND;  /* Combining Enclosing Keycap */
    /* Format characters that extend */
    if (cp == 0x200C || cp == 0x200E || cp == 0x200F) return TUI_GBP_EXTEND;  /* ZWNJ, LRM, RLM */

    /* SpacingMark: Spacing combining marks (simplified) */
    /* Thai */
    if (cp >= 0x0E31 && cp <= 0x0E3A) return TUI_GBP_SPACINGMARK;
    /* Lao */
    if (cp >= 0x0EB1 && cp <= 0x0EBC) return TUI_GBP_SPACINGMARK;

    /* Extended_Pictographic: Emoji and pictographic symbols */
    /* Miscellaneous Symbols and Pictographs */
    if (cp >= 0x1F300 && cp <= 0x1F5FF) return TUI_GBP_EXTENDED_PICTOGRAPHIC;
    /* Emoticons */
    if (cp >= 0x1F600 && cp <= 0x1F64F) return TUI_GBP_EXTENDED_PICTOGRAPHIC;
    /* Transport and Map Symbols */
    if (cp >= 0x1F680 && cp <= 0x1F6FF) return TUI_GBP_EXTENDED_PICTOGRAPHIC;
    /* Supplemental Symbols and Pictographs */
    if (cp >= 0x1F900 && cp <= 0x1F9FF) return TUI_GBP_EXTENDED_PICTOGRAPHIC;
    /* Symbols and Pictographs Extended-A */
    if (cp >= 0x1FA00 && cp <= 0x1FA6F) return TUI_GBP_EXTENDED_PICTOGRAPHIC;
    if (cp >= 0x1FA70 && cp <= 0x1FAFF) return TUI_GBP_EXTENDED_PICTOGRAPHIC;
    /* Miscellaneous Symbols */
    if (cp >= 0x2600 && cp <= 0x26FF) return TUI_GBP_EXTENDED_PICTOGRAPHIC;
    /* Dingbats */
    if (cp >= 0x2700 && cp <= 0x27BF) return TUI_GBP_EXTENDED_PICTOGRAPHIC;
    /* Basic emoji (digits, symbols with emoji presentation) */
    if (cp == 0x00A9 || cp == 0x00AE) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* (C), (R) */
    if (cp == 0x203C || cp == 0x2049) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* !!, !? */
    if (cp >= 0x2122 && cp <= 0x2139) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* TM, info */
    if (cp >= 0x2194 && cp <= 0x21AA) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* Arrows */
    if (cp >= 0x231A && cp <= 0x231B) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* Watch, hourglass */
    if (cp == 0x2328 || cp == 0x23CF) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* Keyboard, eject */
    if (cp >= 0x23E9 && cp <= 0x23F3) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* Media controls */
    if (cp >= 0x23F8 && cp <= 0x23FA) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* More media */
    if (cp == 0x25AA || cp == 0x25AB) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* Squares */
    if (cp == 0x25B6 || cp == 0x25C0) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* Triangles */
    if (cp >= 0x25FB && cp <= 0x25FE) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* More squares */
    if (cp >= 0x2934 && cp <= 0x2935) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* Curved arrows */
    if (cp >= 0x2B05 && cp <= 0x2B07) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* Arrows */
    if (cp >= 0x2B1B && cp <= 0x2B1C) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* Squares */
    if (cp == 0x2B50 || cp == 0x2B55) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* Star, circle */
    if (cp == 0x3030 || cp == 0x303D) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* Wavy dash, part mark */
    if (cp == 0x3297 || cp == 0x3299) return TUI_GBP_EXTENDED_PICTOGRAPHIC;  /* CJK ideograph */

    /* Default: Other */
    return TUI_GBP_OTHER;
}

/* ----------------------------------------------------------------
 * Grapheme boundary detection (UAX #29 simplified)
 * ---------------------------------------------------------------- */

int tui_is_grapheme_boundary(tui_grapheme_break_property prev_gbp,
                              tui_grapheme_break_property curr_gbp,
                              int ri_count, int after_zwj)
{
    /* GB1, GB2: Always break at start/end (handled by caller) */

    /* GB3: Do not break between CR and LF */
    if (prev_gbp == TUI_GBP_CR && curr_gbp == TUI_GBP_LF) {
        return 0;
    }

    /* GB4: Break after Control, CR, LF */
    if (prev_gbp == TUI_GBP_CONTROL || prev_gbp == TUI_GBP_CR ||
        prev_gbp == TUI_GBP_LF) {
        return 1;
    }

    /* GB5: Break before Control, CR, LF */
    if (curr_gbp == TUI_GBP_CONTROL || curr_gbp == TUI_GBP_CR ||
        curr_gbp == TUI_GBP_LF) {
        return 1;
    }

    /* GB6: Do not break Hangul L + (L|V|LV|LVT) */
    if (prev_gbp == TUI_GBP_HANGUL_L &&
        (curr_gbp == TUI_GBP_HANGUL_L || curr_gbp == TUI_GBP_HANGUL_V ||
         curr_gbp == TUI_GBP_HANGUL_LV || curr_gbp == TUI_GBP_HANGUL_LVT)) {
        return 0;
    }

    /* GB7: Do not break (LV|V) + (V|T) */
    if ((prev_gbp == TUI_GBP_HANGUL_LV || prev_gbp == TUI_GBP_HANGUL_V) &&
        (curr_gbp == TUI_GBP_HANGUL_V || curr_gbp == TUI_GBP_HANGUL_T)) {
        return 0;
    }

    /* GB8: Do not break (LVT|T) + T */
    if ((prev_gbp == TUI_GBP_HANGUL_LVT || prev_gbp == TUI_GBP_HANGUL_T) &&
        curr_gbp == TUI_GBP_HANGUL_T) {
        return 0;
    }

    /* GB9: Do not break before Extend or ZWJ */
    if (curr_gbp == TUI_GBP_EXTEND || curr_gbp == TUI_GBP_ZWJ) {
        return 0;
    }

    /* GB9a: Do not break before SpacingMark */
    if (curr_gbp == TUI_GBP_SPACINGMARK) {
        return 0;
    }

    /* GB9b: Do not break after Prepend */
    if (prev_gbp == TUI_GBP_PREPEND) {
        return 0;
    }

    /* GB11: Do not break within emoji ZWJ sequences */
    /* ZWJ + Extended_Pictographic → no break */
    if (after_zwj && curr_gbp == TUI_GBP_EXTENDED_PICTOGRAPHIC) {
        return 0;
    }

    /* GB12, GB13: Regional Indicator pairs */
    /* RI + RI → no break if we've seen odd number of RIs */
    if (prev_gbp == TUI_GBP_REGIONAL_INDICATOR &&
        curr_gbp == TUI_GBP_REGIONAL_INDICATOR) {
        /* ri_count counts RIs seen so far including prev */
        /* If ri_count is odd, this RI pairs with the previous one */
        return (ri_count % 2) == 0;
    }

    /* GB999: Otherwise, break */
    return 1;
}

/* ----------------------------------------------------------------
 * Grapheme iterator
 * ---------------------------------------------------------------- */

void tui_grapheme_iter_init(tui_grapheme_iter *iter, const char *text, size_t len)
{
    if (!iter) return;
    iter->text = text;
    iter->len = len;
    iter->pos = 0;
}

int tui_grapheme_iter_next(tui_grapheme_iter *iter, const char **start, size_t *len)
{
    if (!iter || !iter->text || iter->pos >= iter->len) {
        if (start) *start = NULL;
        if (len) *len = 0;
        return 0;
    }

    const char *text = iter->text;
    size_t text_len = iter->len;
    size_t pos = iter->pos;

    /* Start of this grapheme cluster */
    size_t cluster_start = pos;

    /* Decode first codepoint */
    uint32_t prev_cp;
    int bytes = tui_utf8_decode_n(text + pos, (int)(text_len - pos), &prev_cp);
    if (bytes <= 0) {
        if (start) *start = NULL;
        if (len) *len = 0;
        return 0;
    }
    pos += (size_t)bytes;

    tui_grapheme_break_property prev_gbp = tui_get_grapheme_break_property(prev_cp);
    int ri_count = (prev_gbp == TUI_GBP_REGIONAL_INDICATOR) ? 1 : 0;
    int after_zwj = (prev_gbp == TUI_GBP_ZWJ);

    /* Continue until we find a grapheme boundary */
    while (pos < text_len) {
        uint32_t curr_cp;
        bytes = tui_utf8_decode_n(text + pos, (int)(text_len - pos), &curr_cp);
        if (bytes <= 0) break;

        tui_grapheme_break_property curr_gbp = tui_get_grapheme_break_property(curr_cp);

        /* Check if there's a boundary */
        if (tui_is_grapheme_boundary(prev_gbp, curr_gbp, ri_count, after_zwj)) {
            break;  /* Found a boundary, stop here */
        }

        /* Extend the cluster */
        pos += (size_t)bytes;

        /* Update state for next iteration */
        if (curr_gbp == TUI_GBP_REGIONAL_INDICATOR) {
            ri_count++;
        } else if (prev_gbp != TUI_GBP_REGIONAL_INDICATOR) {
            ri_count = 0;  /* Reset RI count if not in RI sequence */
        }

        after_zwj = (curr_gbp == TUI_GBP_ZWJ);
        prev_gbp = curr_gbp;
    }

    /* Return the grapheme cluster */
    if (start) *start = text + cluster_start;
    if (len) *len = pos - cluster_start;

    iter->pos = pos;
    return 1;
}

/* ----------------------------------------------------------------
 * Grapheme utilities
 * ---------------------------------------------------------------- */

int tui_grapheme_width(const char *grapheme, size_t len)
{
    if (!grapheme || len == 0) return 0;

    size_t pos = 0;
    int width = 0;
    int base_width = 0;
    int is_first = 1;
    int in_zwj_sequence = 0;
    int ri_count = 0;

    while (pos < len) {
        uint32_t cp;
        int bytes = tui_utf8_decode_n(grapheme + pos, (int)(len - pos), &cp);
        if (bytes <= 0) break;
        pos += (size_t)bytes;

        tui_grapheme_break_property gbp = tui_get_grapheme_break_property(cp);

        if (gbp == TUI_GBP_REGIONAL_INDICATOR) {
            ri_count++;
            if (ri_count == 1) {
                /* First RI - will be paired with second */
                base_width = 2;  /* Flag pairs are width 2 */
            }
            /* Second RI doesn't add width */
            continue;
        }

        if (gbp == TUI_GBP_ZWJ) {
            in_zwj_sequence = 1;
            continue;  /* ZWJ is zero-width */
        }

        if (gbp == TUI_GBP_EXTEND) {
            continue;  /* Extending characters are zero-width */
        }

        if (in_zwj_sequence && gbp == TUI_GBP_EXTENDED_PICTOGRAPHIC) {
            /* This is a continuation of a ZWJ sequence */
            /* The combined sequence should typically be width 2 */
            in_zwj_sequence = 0;
            continue;
        }

        /* Base character */
        if (is_first) {
            base_width = tui_char_width(cp);
            is_first = 0;
        }

        in_zwj_sequence = 0;
    }

    /* Use base_width, or minimum width of 1 if we found something */
    width = base_width;
    if (width == 0 && !is_first) {
        width = 1;  /* At least show something */
    }

    return width;
}

int tui_grapheme_count(const char *str, int len)
{
    if (!str) return 0;

    size_t actual_len;
    if (len < 0) {
        actual_len = strlen(str);
    } else {
        actual_len = (size_t)len;
    }

    tui_grapheme_iter iter;
    tui_grapheme_iter_init(&iter, str, actual_len);

    int count = 0;
    const char *start;
    size_t grapheme_len;

    while (tui_grapheme_iter_next(&iter, &start, &grapheme_len)) {
        count++;
    }

    return count;
}
