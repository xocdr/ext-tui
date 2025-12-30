/*
  +----------------------------------------------------------------------+
  | ext-tui: Text measurement (Unicode width)                           |
  +----------------------------------------------------------------------+
  | Provides Unicode-aware text width calculation and UTF-8 encoding/   |
  | decoding utilities.                                                 |
  |                                                                      |
  | Thread Safety: All functions in this module are THREAD-SAFE and     |
  | REENTRANT. They operate only on their input parameters and have no  |
  | global state.                                                        |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_MEASURE_H
#define TUI_MEASURE_H

#include <stdint.h>
#include <stddef.h>

/**
 * Get display width of a Unicode codepoint.
 * Returns 0 for control/combining, 1 for most chars, 2 for CJK/emoji.
 *
 * Thread Safety: THREAD-SAFE, REENTRANT (pure function)
 */
int tui_char_width(uint32_t codepoint);

/**
 * Get display width of a null-terminated UTF-8 string.
 */
int tui_string_width(const char *str);

/**
 * Get display width of a UTF-8 string with known byte length.
 */
int tui_string_width_n(const char *str, int len);

/**
 * Decode UTF-8 character from null-terminated string.
 * @return Bytes consumed (1-4), or 0 if str is NULL/empty
 */
int tui_utf8_decode(const char *str, uint32_t *codepoint);

/**
 * Decode UTF-8 character with bounds checking.
 * @param str Input string
 * @param len Remaining bytes in string
 * @param codepoint Output codepoint
 * @return Bytes consumed (1-4), 0 on error
 */
int tui_utf8_decode_n(const char *str, int len, uint32_t *codepoint);

/**
 * Encode codepoint to UTF-8.
 * @param codepoint Unicode codepoint
 * @param buf Output buffer (must have at least 4 bytes)
 * @return Bytes written (1-4)
 */
int tui_utf8_encode(uint32_t codepoint, char *buf);

/**
 * Pad string to width with bounds checking.
 * @param text Input text (NULL treated as empty)
 * @param width Target display width
 * @param align 'l'=left, 'r'=right, 'c'=center
 * @param pad_char Padding character
 * @param output Output buffer
 * @param output_size Size of output buffer
 * @return Bytes written, or -1 on error
 */
int tui_pad_n(const char *text, int width, char align, char pad_char,
              char *output, size_t output_size);

/* Note: tui_pad() removed - use tui_pad_n() for buffer overflow protection */

/* ----------------------------------------------------------------
 * ANSI escape code handling
 * ---------------------------------------------------------------- */

/**
 * Strip all ANSI escape codes from string.
 * @param str Input string (may contain ANSI codes)
 * @return Newly allocated string without ANSI codes (caller must free)
 */
char* tui_strip_ansi(const char *str);

/**
 * Get display width of string, ignoring ANSI escape codes.
 * @param str Input string (may contain ANSI codes)
 * @return Display width (not counting ANSI codes)
 */
int tui_string_width_ansi(const char *str);

/**
 * Extract substring while preserving ANSI codes.
 * Extracts characters from display position `start` to `end` (exclusive),
 * but includes any ANSI codes that affect those characters.
 * @param str Input string (may contain ANSI codes)
 * @param start Start display position (0-indexed)
 * @param end End display position (exclusive)
 * @return Newly allocated substring (caller must free)
 */
char* tui_slice_ansi(const char *str, int start, int end);

#endif /* TUI_MEASURE_H */
