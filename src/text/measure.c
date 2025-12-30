/*
  +----------------------------------------------------------------------+
  | ext-tui: Text measurement (Unicode width)                           |
  +----------------------------------------------------------------------+
  | Provides Unicode-aware text width calculation and UTF-8 encoding/   |
  | decoding utilities. Uses simplified wcwidth implementation that     |
  | covers common cases (ASCII, CJK, emoji) but not the full Unicode    |
  | East Asian Width standard.                                          |
  +----------------------------------------------------------------------+
*/

#include "measure.h"
#include <stdlib.h>
#include <string.h>

/**
 * Get display width of a Unicode codepoint.
 *
 * Uses simplified width tables covering:
 * - Control characters (width 0)
 * - ASCII (width 1)
 * - Combining marks (width 0)
 * - CJK characters (width 2)
 * - Emoji (width 2)
 *
 * @param codepoint Unicode codepoint
 * @return Display width (0, 1, or 2)
 */
/* Unicode constants for emoji handling */
#define UNICODE_ZWJ             0x200D   /* Zero-Width Joiner */
#define UNICODE_VS16            0xFE0F   /* Variation Selector 16 (emoji presentation) */
#define UNICODE_VS15            0xFE0E   /* Variation Selector 15 (text presentation) */

/**
 * Check if codepoint is an emoji modifier (skin tone).
 * Fitzpatrick skin tone modifiers: U+1F3FB to U+1F3FF
 */
static int is_emoji_modifier(uint32_t codepoint)
{
    return (codepoint >= 0x1F3FB && codepoint <= 0x1F3FF);
}

/**
 * Check if codepoint is part of an emoji sequence (ZWJ, VS, modifier).
 * Currently unused but kept for future emoji sequence detection improvements.
 */
static int __attribute__((unused)) is_emoji_sequence_part(uint32_t codepoint)
{
    return (codepoint == UNICODE_ZWJ ||
            codepoint == UNICODE_VS16 ||
            codepoint == UNICODE_VS15 ||
            is_emoji_modifier(codepoint) ||
            /* Regional indicator symbols (flags): U+1F1E0 to U+1F1FF */
            (codepoint >= 0x1F1E6 && codepoint <= 0x1F1FF) ||
            /* Keycap combining sequence: digit + U+FE0F + U+20E3 */
            codepoint == 0x20E3);
}

/**
 * Check if codepoint is an emoji base (can start an emoji sequence).
 */
static int is_emoji_base(uint32_t codepoint)
{
    /* Emoji ranges */
    return ((codepoint >= 0x1F300 && codepoint <= 0x1F9FF) ||  /* Misc Symbols & Pictographs */
            (codepoint >= 0x1FA00 && codepoint <= 0x1FAFF) ||  /* Extended-A */
            (codepoint >= 0x2600 && codepoint <= 0x26FF) ||    /* Misc Symbols */
            (codepoint >= 0x2700 && codepoint <= 0x27BF) ||    /* Dingbats */
            (codepoint >= 0x1F1E6 && codepoint <= 0x1F1FF) ||  /* Regional Indicators (flags) */
            (codepoint >= 0x1F600 && codepoint <= 0x1F64F));   /* Emoticons */
}

int tui_char_width(uint32_t codepoint)
{
    /* Control characters */
    if (codepoint < 32 || codepoint == 127) {
        return 0;
    }

    /* ASCII */
    if (codepoint < 128) {
        return 1;
    }

    /* Zero-width characters */
    if (codepoint == UNICODE_ZWJ ||
        codepoint == UNICODE_VS16 ||
        codepoint == UNICODE_VS15) {
        return 0;
    }

    /* Emoji modifiers (skin tones) - zero width when modifying */
    if (is_emoji_modifier(codepoint)) {
        return 0;
    }

    /* Combining marks (approximate) */
    if (codepoint >= 0x0300 && codepoint <= 0x036F) {
        return 0;
    }

    /* Keycap combining mark */
    if (codepoint == 0x20E3) {
        return 0;
    }

    /* CJK characters (approximate ranges) */
    if ((codepoint >= 0x1100 && codepoint <= 0x115F) ||   /* Hangul Jamo */
        (codepoint >= 0x2E80 && codepoint <= 0x9FFF) ||   /* CJK */
        (codepoint >= 0xAC00 && codepoint <= 0xD7AF) ||   /* Hangul */
        (codepoint >= 0xF900 && codepoint <= 0xFAFF) ||   /* CJK Compat */
        (codepoint >= 0xFE10 && codepoint <= 0xFE1F) ||   /* Vertical forms */
        (codepoint >= 0xFE30 && codepoint <= 0xFE6F) ||   /* CJK Compat */
        (codepoint >= 0xFF00 && codepoint <= 0xFF60) ||   /* Fullwidth */
        (codepoint >= 0xFFE0 && codepoint <= 0xFFE6) ||   /* Fullwidth */
        (codepoint >= 0x20000 && codepoint <= 0x2FFFF)) { /* CJK Ext */
        return 2;
    }

    /* Emoji ranges - only characters that are reliably width 2 across terminals */
    if ((codepoint >= 0x1F300 && codepoint <= 0x1F9FF) ||  /* Misc Symbols & Pictographs, Emoticons, etc. */
        (codepoint >= 0x1FA00 && codepoint <= 0x1FAFF) ||  /* Chess, symbols, etc. */
        (codepoint >= 0x1F1E6 && codepoint <= 0x1F1FF)) {  /* Regional Indicators (flags) */
        return 2;
    }

    /* Note: Characters in 0x2300-0x2BFF (Misc Technical, Dingbats, etc.) are
     * "East Asian Ambiguous Width" and render as width 1 in most terminals.
     * We treat them as width 1 for better compatibility. */

    return 1;
}

/**
 * Decode a UTF-8 character from a string with length validation.
 *
 * @param str Input string (must not be NULL)
 * @param len Remaining bytes in string (must be > 0)
 * @param codepoint Output: decoded Unicode codepoint
 * @return Number of bytes consumed (1-4), or 1 for invalid sequences
 */
int tui_utf8_decode_n(const char *str, int len, uint32_t *codepoint)
{
    if (!str || len <= 0) {
        *codepoint = 0;
        return 0;
    }

    unsigned char c = (unsigned char)str[0];

    if ((c & 0x80) == 0) {
        /* ASCII */
        *codepoint = c;
        return 1;
    }

    if ((c & 0xE0) == 0xC0) {
        /* 2-byte sequence */
        if (len < 2 || ((unsigned char)str[1] & 0xC0) != 0x80) {
            *codepoint = c;  /* Invalid, treat as single byte */
            return 1;
        }
        uint32_t cp = ((c & 0x1F) << 6) | (str[1] & 0x3F);
        /* Reject overlong encodings: 2-byte must encode >= 0x80 */
        if (cp < 0x80) {
            *codepoint = c;
            return 1;
        }
        *codepoint = cp;
        return 2;
    }

    if ((c & 0xF0) == 0xE0) {
        /* 3-byte sequence */
        if (len < 3 ||
            ((unsigned char)str[1] & 0xC0) != 0x80 ||
            ((unsigned char)str[2] & 0xC0) != 0x80) {
            *codepoint = c;
            return 1;
        }
        uint32_t cp = ((c & 0x0F) << 12) |
                     ((str[1] & 0x3F) << 6) |
                     (str[2] & 0x3F);
        /* Reject overlong encodings: 3-byte must encode >= 0x800 */
        if (cp < 0x800) {
            *codepoint = c;
            return 1;
        }
        /* Reject UTF-16 surrogates (U+D800 to U+DFFF) - invalid in UTF-8 */
        if (cp >= 0xD800 && cp <= 0xDFFF) {
            *codepoint = c;
            return 1;
        }
        *codepoint = cp;
        return 3;
    }

    if ((c & 0xF8) == 0xF0) {
        /* 4-byte sequence */
        if (len < 4 ||
            ((unsigned char)str[1] & 0xC0) != 0x80 ||
            ((unsigned char)str[2] & 0xC0) != 0x80 ||
            ((unsigned char)str[3] & 0xC0) != 0x80) {
            *codepoint = c;
            return 1;
        }
        uint32_t cp = ((c & 0x07) << 18) |
                     ((str[1] & 0x3F) << 12) |
                     ((str[2] & 0x3F) << 6) |
                     (str[3] & 0x3F);
        /* Reject overlong encodings and out-of-range: 4-byte must encode 0x10000-0x10FFFF */
        if (cp < 0x10000 || cp > 0x10FFFF) {
            *codepoint = c;
            return 1;
        }
        *codepoint = cp;
        return 4;
    }

    /* Invalid, treat as single byte */
    *codepoint = c;
    return 1;
}

/**
 * Decode a UTF-8 character from a null-terminated string.
 *
 * WARNING: This function does not check string bounds. Use tui_utf8_decode_n()
 * when the string length is known for safer operation.
 *
 * @param str Input null-terminated string
 * @param codepoint Output: decoded Unicode codepoint
 * @return Number of bytes consumed (1-4)
 */
int tui_utf8_decode(const char *str, uint32_t *codepoint)
{
    if (!str || !*str) {
        *codepoint = 0;
        return 0;
    }

    unsigned char c = (unsigned char)str[0];

    if ((c & 0x80) == 0) {
        /* ASCII */
        *codepoint = c;
        return 1;
    }

    if ((c & 0xE0) == 0xC0) {
        /* 2-byte - validate continuation byte exists and is valid (10xxxxxx) */
        if (!str[1] || ((unsigned char)str[1] & 0xC0) != 0x80) {
            *codepoint = c;
            return 1;
        }
        uint32_t cp = ((c & 0x1F) << 6) | (str[1] & 0x3F);
        /* Reject overlong encodings: 2-byte must encode >= 0x80 */
        if (cp < 0x80) {
            *codepoint = c;
            return 1;
        }
        *codepoint = cp;
        return 2;
    }

    if ((c & 0xF0) == 0xE0) {
        /* 3-byte - validate continuation bytes exist and are valid (10xxxxxx) */
        if (!str[1] || !str[2] ||
            ((unsigned char)str[1] & 0xC0) != 0x80 ||
            ((unsigned char)str[2] & 0xC0) != 0x80) {
            *codepoint = c;
            return 1;
        }
        uint32_t cp = ((c & 0x0F) << 12) |
                      ((str[1] & 0x3F) << 6) |
                      (str[2] & 0x3F);
        /* Reject overlong encodings: 3-byte must encode >= 0x800 */
        if (cp < 0x800) {
            *codepoint = c;
            return 1;
        }
        /* Reject UTF-16 surrogates (U+D800 to U+DFFF) - invalid in UTF-8 */
        if (cp >= 0xD800 && cp <= 0xDFFF) {
            *codepoint = c;
            return 1;
        }
        *codepoint = cp;
        return 3;
    }

    if ((c & 0xF8) == 0xF0) {
        /* 4-byte - validate continuation bytes exist and are valid (10xxxxxx) */
        if (!str[1] || !str[2] || !str[3] ||
            ((unsigned char)str[1] & 0xC0) != 0x80 ||
            ((unsigned char)str[2] & 0xC0) != 0x80 ||
            ((unsigned char)str[3] & 0xC0) != 0x80) {
            *codepoint = c;
            return 1;
        }
        uint32_t cp = ((c & 0x07) << 18) |
                      ((str[1] & 0x3F) << 12) |
                      ((str[2] & 0x3F) << 6) |
                      (str[3] & 0x3F);
        /* Reject overlong encodings: 4-byte must encode >= 0x10000 */
        if (cp < 0x10000 || cp > 0x10FFFF) {
            *codepoint = c;
            return 1;
        }
        *codepoint = cp;
        return 4;
    }

    /* Invalid, treat as single byte */
    *codepoint = c;
    return 1;
}

/**
 * Encode a Unicode codepoint to UTF-8.
 *
 * @param codepoint Unicode codepoint to encode
 * @param buf Output buffer (must have at least 4 bytes available)
 * @return Number of bytes written (1-4)
 */
int tui_utf8_encode(uint32_t codepoint, char *buf)
{
    if (codepoint < 0x80) {
        buf[0] = (char)codepoint;
        return 1;
    }

    if (codepoint < 0x800) {
        buf[0] = (char)(0xC0 | (codepoint >> 6));
        buf[1] = (char)(0x80 | (codepoint & 0x3F));
        return 2;
    }

    if (codepoint < 0x10000) {
        buf[0] = (char)(0xE0 | (codepoint >> 12));
        buf[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        buf[2] = (char)(0x80 | (codepoint & 0x3F));
        return 3;
    }

    if (codepoint <= 0x10FFFF) {
        buf[0] = (char)(0xF0 | (codepoint >> 18));
        buf[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        buf[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        buf[3] = (char)(0x80 | (codepoint & 0x3F));
        return 4;
    }

    /* Invalid codepoint, encode as replacement character */
    buf[0] = (char)0xEF;
    buf[1] = (char)0xBF;
    buf[2] = (char)0xBD;
    return 3;
}

/**
 * Get display width of a null-terminated UTF-8 string.
 *
 * @param str Input string
 * @return Total display width
 */
int tui_string_width(const char *str)
{
    if (!str) return 0;
    return tui_string_width_n(str, (int)strlen(str));
}

/**
 * Maximum length for ANSI escape sequences.
 * Prevents excessive looping on pathological input.
 * Most real sequences are < 20 bytes; 64 is very generous.
 */
#define MAX_ANSI_SEQUENCE_LENGTH 64

/**
 * Skip ANSI escape sequence starting at position.
 *
 * ANSI sequences start with ESC (0x1B) followed by:
 * - '[' then parameters and a letter (CSI sequences, e.g., \033[31m)
 * - Other single characters (e.g., \033(B)
 *
 * @param str Input string
 * @param pos Current position
 * @param len Total string length
 * @return Number of bytes to skip (0 if not an escape sequence)
 */
static int skip_ansi_sequence(const char *str, int pos, int len)
{
    if (pos >= len || str[pos] != '\033') return 0;

    int start = pos;
    int max_end = pos + MAX_ANSI_SEQUENCE_LENGTH;
    if (max_end > len) max_end = len;

    pos++;  /* Skip ESC */

    if (pos >= len) return 1;  /* Just ESC at end */

    if (str[pos] == '[') {
        /* CSI sequence: ESC [ params letter */
        pos++;
        while (pos < max_end) {
            unsigned char c = (unsigned char)str[pos];
            if (c >= 0x40 && c <= 0x7E) {
                /* Final byte found (letter) */
                return pos - start + 1;
            }
            if (c < 0x20 || c > 0x3F) {
                /* Invalid character in sequence */
                break;
            }
            pos++;
        }
        return pos - start;  /* Incomplete or too-long sequence */
    } else if (str[pos] == ']') {
        /* OSC sequence: ESC ] ... ST or BEL */
        pos++;
        while (pos < max_end) {
            if (str[pos] == '\007') {  /* BEL */
                return pos - start + 1;
            }
            if (str[pos] == '\033' && pos + 1 < len && str[pos + 1] == '\\') {
                /* ST = ESC \ */
                return pos - start + 2;
            }
            pos++;
        }
        return pos - start;  /* Incomplete or too-long sequence */
    } else {
        /* Single-character escape (e.g., ESC ( B) */
        return 2;
    }
}

/**
 * Get display width of a UTF-8 string with known length.
 *
 * ANSI escape sequences are ignored (they don't take display space).
 * ZWJ (Zero-Width Joiner) sequences are handled: characters after ZWJ
 * don't add width (they combine with the previous character).
 * Flag sequences (regional indicators) are handled: two regional indicators
 * combine to form one flag with width 2.
 *
 * @param str Input string
 * @param len Number of bytes to process
 * @return Total display width
 */
int tui_string_width_n(const char *str, int len)
{
    if (!str || len <= 0) return 0;

    int width = 0;
    int pos = 0;
    int after_zwj = 0;           /* Skip width after ZWJ */
    int regional_count = 0;      /* Count regional indicators for flag pairs */
    int prev_width = 0;          /* Width of previous character for VS16 handling */

    while (pos < len && str[pos]) {
        /* Skip ANSI escape sequences */
        int skip = skip_ansi_sequence(str, pos, len);
        if (skip > 0) {
            pos += skip;
            continue;
        }

        uint32_t codepoint;
        int bytes = tui_utf8_decode_n(str + pos, len - pos, &codepoint);
        if (bytes <= 0) break;

        /* Handle ZWJ sequences: character after ZWJ combines with previous */
        if (codepoint == UNICODE_ZWJ) {
            after_zwj = 1;
            pos += bytes;
            continue;
        }

        /* VS16 (emoji presentation): upgrades previous char to width 2 */
        if (codepoint == UNICODE_VS16) {
            if (prev_width == 1) {
                /* Previous char was text-width, upgrade to emoji-width */
                width += 1;  /* Add the extra width */
            }
            pos += bytes;
            continue;
        }

        /* VS15 (text presentation): no visual change for our purposes */
        if (codepoint == UNICODE_VS15) {
            pos += bytes;
            continue;
        }

        /* Regional indicators: pairs form flags */
        if (codepoint >= 0x1F1E6 && codepoint <= 0x1F1FF) {
            regional_count++;
            if (regional_count == 2) {
                /* Second indicator completes the flag, which has width 2 */
                width += 2;
                regional_count = 0;
            }
            pos += bytes;
            prev_width = 0;  /* Don't track width for regional indicators */
            continue;
        } else {
            /* Reset regional counter if non-regional character seen */
            if (regional_count == 1) {
                /* Lone regional indicator - add its width */
                width += 2;
            }
            regional_count = 0;
        }

        /* If we're after a ZWJ, the next emoji combines with previous */
        if (after_zwj && is_emoji_base(codepoint)) {
            after_zwj = 0;
            pos += bytes;
            prev_width = 0;  /* No width added */
            continue;
        }
        after_zwj = 0;

        int char_width = tui_char_width(codepoint);
        width += char_width;
        prev_width = char_width;
        pos += bytes;
    }

    /* Handle trailing lone regional indicator */
    if (regional_count == 1) {
        width += 2;
    }

    return width;
}

/**
 * Pad string to specified display width (accounting for Unicode width).
 *
 * @param text Input text (NULL treated as empty string)
 * @param width Target display width
 * @param align Alignment: 'l'=left, 'r'=right, 'c'=center
 * @param pad_char Padding character (should be single-width ASCII)
 * @param output Output buffer
 * @param output_size Size of output buffer in bytes
 * @return Number of bytes written (excluding null terminator), or -1 on error
 */
int tui_pad_n(const char *text, int width, char align, char pad_char,
              char *output, size_t output_size)
{
    if (!output || output_size == 0) return -1;
    if (width < 0) width = 0;

    const char *src = text ? text : "";
    size_t text_len = strlen(src);
    int text_width = tui_string_width_n(src, (int)text_len);

    /* Check if output buffer is large enough */
    size_t needed = text_len + (width > text_width ? (size_t)(width - text_width) : 0) + 1;
    if (needed > output_size) {
        output[0] = '\0';
        return -1;
    }

    /* If text is already wider than target, just copy it */
    if (text_width >= width) {
        if (text_len + 1 > output_size) {
            output[0] = '\0';
            return -1;
        }
        memcpy(output, src, text_len);
        output[text_len] = '\0';
        return (int)text_len;
    }

    int padding = width - text_width;
    int left_pad = 0;
    int right_pad = 0;

    switch (align) {
        case 'r':
        case 'R':
            left_pad = padding;
            break;
        case 'c':
        case 'C':
            left_pad = padding / 2;
            right_pad = padding - left_pad;
            break;
        case 'l':
        case 'L':
        default:
            right_pad = padding;
            break;
    }

    size_t pos = 0;

    /* Left padding */
    for (int i = 0; i < left_pad && pos < output_size - 1; i++) {
        output[pos++] = pad_char;
    }

    /* Text */
    if (pos + text_len < output_size) {
        memcpy(output + pos, src, text_len);
        pos += text_len;
    }

    /* Right padding */
    for (int i = 0; i < right_pad && pos < output_size - 1; i++) {
        output[pos++] = pad_char;
    }

    output[pos] = '\0';
    return (int)pos;
}

/**
 * Pad string to specified display width (legacy version without size check).
 *
 * WARNING: Caller must ensure output buffer is large enough.
 * Use tui_pad_n() for safer operation.
 *
 * @param text Input text
 * @param width Target display width
 * @param align Alignment: 'l'=left, 'r'=right, 'c'=center
 * @param pad_char Padding character
 * @param output Output buffer (must be at least width + strlen(text) + 1 bytes)
 * @return Number of bytes written
 */
int tui_pad(const char *text, int width, char align, char pad_char, char *output)
{
    if (!output) return 0;

    const char *src = text ? text : "";
    size_t text_len = strlen(src);
    int text_width = tui_string_width_n(src, (int)text_len);

    /* If text is already wider than target, just copy it */
    if (text_width >= width) {
        memcpy(output, src, text_len);
        output[text_len] = '\0';
        return (int)text_len;
    }

    int padding = width - text_width;
    int left_pad = 0;
    int right_pad = 0;

    switch (align) {
        case 'r':
        case 'R':
            left_pad = padding;
            break;
        case 'c':
        case 'C':
            left_pad = padding / 2;
            right_pad = padding - left_pad;
            break;
        case 'l':
        case 'L':
        default:
            right_pad = padding;
            break;
    }

    int pos = 0;

    /* Left padding */
    for (int i = 0; i < left_pad; i++) {
        output[pos++] = pad_char;
    }

    /* Text */
    memcpy(output + pos, src, text_len);
    pos += (int)text_len;

    /* Right padding */
    for (int i = 0; i < right_pad; i++) {
        output[pos++] = pad_char;
    }

    output[pos] = '\0';
    return pos;
}

/* ----------------------------------------------------------------
 * ANSI escape code handling
 * ---------------------------------------------------------------- */

/**
 * Strip all ANSI escape codes from string.
 *
 * @param str Input string (may contain ANSI codes)
 * @return Newly allocated string without ANSI codes (caller must free)
 */
char* tui_strip_ansi(const char *str)
{
    if (!str) return NULL;

    size_t len = strlen(str);
    char *result = malloc(len + 1);
    if (!result) return NULL;

    size_t out_pos = 0;
    int pos = 0;

    while (pos < (int)len && str[pos]) {
        /* Skip ANSI escape sequences */
        int skip = skip_ansi_sequence(str, pos, (int)len);
        if (skip > 0) {
            pos += skip;
            continue;
        }

        /* Copy non-ANSI byte */
        result[out_pos++] = str[pos++];
    }

    result[out_pos] = '\0';

    /* Shrink allocation to actual size */
    char *shrunk = realloc(result, out_pos + 1);
    return shrunk ? shrunk : result;
}

/**
 * Get display width of string, ignoring ANSI escape codes.
 *
 * This is identical to tui_string_width_n() since it already ignores ANSI codes.
 *
 * @param str Input string (may contain ANSI codes)
 * @return Display width (not counting ANSI codes)
 */
int tui_string_width_ansi(const char *str)
{
    if (!str) return 0;
    return tui_string_width_n(str, (int)strlen(str));
}

/**
 * Extract substring while preserving ANSI codes.
 *
 * Extracts characters from display position `start` to `end` (exclusive),
 * but includes any ANSI codes that appear within that range or that were
 * active at the start position.
 *
 * @param str Input string (may contain ANSI codes)
 * @param start Start display position (0-indexed)
 * @param end End display position (exclusive)
 * @return Newly allocated substring (caller must free)
 */
char* tui_slice_ansi(const char *str, int start, int end)
{
    if (!str || start < 0 || end <= start) return NULL;

    size_t len = strlen(str);
    /* Worst case: entire string fits in the slice */
    char *result = malloc(len + 1);
    if (!result) return NULL;

    size_t out_pos = 0;
    int pos = 0;
    int display_pos = 0;  /* Current display position (not counting ANSI) */

    /* Track ANSI codes encountered before the start position */
    /* These need to be included to maintain proper styling */
    size_t pending_ansi_start = 0;
    size_t pending_ansi_len = 0;

    while (pos < (int)len && str[pos]) {
        /* Check for ANSI escape sequence */
        int skip = skip_ansi_sequence(str, pos, (int)len);
        if (skip > 0) {
            if (display_pos < start) {
                /* Before our slice - track for inclusion */
                pending_ansi_start = (size_t)pos;
                pending_ansi_len = (size_t)skip;
            } else if (display_pos < end) {
                /* Within our slice - include it */
                memcpy(result + out_pos, str + pos, (size_t)skip);
                out_pos += (size_t)skip;
            }
            pos += skip;
            continue;
        }

        /* Regular character */
        uint32_t codepoint;
        int bytes = tui_utf8_decode_n(str + pos, (int)len - pos, &codepoint);
        if (bytes <= 0) break;

        int char_width = tui_char_width(codepoint);

        /* Check if this character is within our slice */
        if (display_pos >= start && display_pos < end) {
            /* First character - include any pending ANSI codes */
            if (out_pos == 0 && pending_ansi_len > 0) {
                memcpy(result, str + pending_ansi_start, pending_ansi_len);
                out_pos = pending_ansi_len;
            }
            /* Copy the character */
            memcpy(result + out_pos, str + pos, (size_t)bytes);
            out_pos += (size_t)bytes;
        }

        display_pos += char_width;
        pos += bytes;

        /* Stop if we've passed the end */
        if (display_pos >= end) break;
    }

    result[out_pos] = '\0';

    /* Shrink allocation to actual size */
    char *shrunk = realloc(result, out_pos + 1);
    return shrunk ? shrunk : result;
}
