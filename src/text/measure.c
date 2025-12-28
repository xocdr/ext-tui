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

    /* Combining marks (approximate) */
    if (codepoint >= 0x0300 && codepoint <= 0x036F) {
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
        (codepoint >= 0x1FA00 && codepoint <= 0x1FAFF)) {  /* Chess, symbols, etc. */
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
        *codepoint = ((c & 0x1F) << 6) | (str[1] & 0x3F);
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
        *codepoint = ((c & 0x0F) << 12) |
                     ((str[1] & 0x3F) << 6) |
                     (str[2] & 0x3F);
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
        *codepoint = ((c & 0x07) << 18) |
                     ((str[1] & 0x3F) << 12) |
                     ((str[2] & 0x3F) << 6) |
                     (str[3] & 0x3F);
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
        /* 2-byte - validate continuation byte exists */
        if (!str[1]) {
            *codepoint = c;
            return 1;
        }
        *codepoint = ((c & 0x1F) << 6) | (str[1] & 0x3F);
        return 2;
    }

    if ((c & 0xF0) == 0xE0) {
        /* 3-byte */
        if (!str[1] || !str[2]) {
            *codepoint = c;
            return 1;
        }
        *codepoint = ((c & 0x0F) << 12) |
                     ((str[1] & 0x3F) << 6) |
                     (str[2] & 0x3F);
        return 3;
    }

    if ((c & 0xF8) == 0xF0) {
        /* 4-byte */
        if (!str[1] || !str[2] || !str[3]) {
            *codepoint = c;
            return 1;
        }
        *codepoint = ((c & 0x07) << 18) |
                     ((str[1] & 0x3F) << 12) |
                     ((str[2] & 0x3F) << 6) |
                     (str[3] & 0x3F);
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
    pos++;  /* Skip ESC */

    if (pos >= len) return 1;  /* Just ESC at end */

    if (str[pos] == '[') {
        /* CSI sequence: ESC [ params letter */
        pos++;
        while (pos < len) {
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
        return pos - start;  /* Incomplete sequence */
    } else if (str[pos] == ']') {
        /* OSC sequence: ESC ] ... ST or BEL */
        pos++;
        while (pos < len) {
            if (str[pos] == '\007') {  /* BEL */
                return pos - start + 1;
            }
            if (str[pos] == '\033' && pos + 1 < len && str[pos + 1] == '\\') {
                /* ST = ESC \ */
                return pos - start + 2;
            }
            pos++;
        }
        return pos - start;
    } else {
        /* Single-character escape (e.g., ESC ( B) */
        return 2;
    }
}

/**
 * Get display width of a UTF-8 string with known length.
 *
 * ANSI escape sequences are ignored (they don't take display space).
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
        width += tui_char_width(codepoint);
        pos += bytes;
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
