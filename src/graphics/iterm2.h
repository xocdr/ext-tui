/*
  +----------------------------------------------------------------------+
  | ext-tui: iTerm2 Inline Images Protocol support                       |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
  | Protocol reference:                                                   |
  | https://iterm2.com/documentation-images.html                          |
  |                                                                        |
  | Format: ESC ] 1337 ; File=[args] : [base64] BEL                       |
  | Parameters:                                                            |
  |   name=<base64 filename>  - optional filename                          |
  |   size=<bytes>            - file size in bytes                         |
  |   width=<n>               - width (cells, px, or %)                    |
  |   height=<n>              - height (cells, px, or %)                   |
  |   preserveAspectRatio=1   - maintain aspect ratio                      |
  |   inline=1                - display inline (required for images)       |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_ITERM2_GRAPHICS_H
#define TUI_ITERM2_GRAPHICS_H

#include "kitty.h"  /* Reuse tui_image structure */
#include <stddef.h>

/**
 * Check if iTerm2 graphics is supported by the terminal.
 * Checks TERM_PROGRAM and LC_TERMINAL environment variables.
 * @return 1 if supported, 0 if not
 */
int tui_iterm2_is_supported(void);

/**
 * Display image using iTerm2 protocol.
 * Writes the OSC 1337 escape sequence directly to stdout.
 *
 * @param img   Image to display (must be in LOADED state with PNG or raw data)
 * @param x     Terminal column (0-based) - cursor is moved here first
 * @param y     Terminal row (0-based)
 * @param cols  Cell width (0 = auto)
 * @param rows  Cell height (0 = auto)
 * @return 0 on success, -1 on error
 */
int tui_iterm2_display(tui_image *img, int x, int y, int cols, int rows);

/**
 * Display image at current cursor position using iTerm2 protocol.
 * @param img   Image to display
 * @param cols  Cell width (0 = auto)
 * @param rows  Cell height (0 = auto)
 * @return 0 on success, -1 on error
 */
int tui_iterm2_display_inline(tui_image *img, int cols, int rows);

/**
 * Build iTerm2 image escape sequence.
 * Caller must provide buffer large enough for base64 encoding.
 *
 * @param buf       Output buffer
 * @param buf_size  Buffer size
 * @param img       Image data
 * @param cols      Cell width (0 = auto)
 * @param rows      Cell height (0 = auto)
 * @return Number of bytes written, or -1 on error
 */
int tui_iterm2_build_escape(char *buf, size_t buf_size,
                            const tui_image *img, int cols, int rows);

#endif /* TUI_ITERM2_GRAPHICS_H */
