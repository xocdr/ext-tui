/*
  +----------------------------------------------------------------------+
  | ext-tui: Sixel graphics protocol support                             |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
  | Sixel Protocol:                                                        |
  | - Start: ESC P q (DCS with Sixel mode)                                 |
  | - Color: # n ; 2 ; r ; g ; b (define color n as RGB 0-100%)            |
  | - Data: Characters 63-126 represent 6 vertical pixels                  |
  | - Row: $ = CR (carriage return), - = NL (next 6 rows)                  |
  | - End: ESC \ (ST - String Terminator)                                  |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_SIXEL_GRAPHICS_H
#define TUI_SIXEL_GRAPHICS_H

#include "kitty.h"  /* Reuse tui_image structure */
#include <stddef.h>
#include <stdint.h>

/* Maximum colors in Sixel palette */
#define TUI_SIXEL_MAX_COLORS 256

/* Dithering methods */
typedef enum {
    TUI_DITHER_NONE = 0,
    TUI_DITHER_FLOYD_STEINBERG
} tui_dither_method;

/* RGB color */
typedef struct {
    uint8_t r, g, b;
} tui_rgb;

/* Quantized image (palette-indexed) */
typedef struct {
    uint8_t *indices;       /* Palette index per pixel [height * width] */
    tui_rgb *palette;       /* Color palette */
    int palette_size;       /* Number of colors in palette */
    int width;
    int height;
} tui_quantized_image;

/* Sixel encoding options */
typedef struct {
    int max_colors;         /* 2-256, default 256 */
    tui_dither_method dither;
} tui_sixel_options;

/**
 * Check if Sixel graphics is supported by the terminal.
 * @return 1 if supported, 0 if not
 */
int tui_sixel_is_supported(void);

/**
 * Encode RGB image to Sixel format.
 *
 * @param rgb_data   Raw RGB pixel data (3 bytes per pixel)
 * @param width      Image width in pixels
 * @param height     Image height in pixels
 * @param options    Encoding options (NULL for defaults)
 * @param sixel_out  Output buffer (allocated by function, caller must free)
 * @param sixel_len  Output length
 * @return 0 on success, -1 on error
 */
int tui_image_to_sixel(const unsigned char *rgb_data,
                       int width, int height,
                       const tui_sixel_options *options,
                       char **sixel_out, size_t *sixel_len);

/**
 * Display image using Sixel protocol.
 *
 * @param img     Image to display
 * @param x       Terminal column (0-based)
 * @param y       Terminal row (0-based)
 * @param options Encoding options (NULL for defaults)
 * @return 0 on success, -1 on error
 */
int tui_sixel_display(tui_image *img, int x, int y,
                      const tui_sixel_options *options);

/**
 * Display Sixel data at current cursor position.
 * @param sixel_data  Pre-encoded Sixel data
 * @param len         Data length
 * @return 0 on success, -1 on error
 */
int tui_sixel_output(const char *sixel_data, size_t len);

/**
 * Free Sixel-encoded data.
 */
void tui_sixel_free(char *sixel_data);

/* Color quantization functions */

/**
 * Quantize RGB image to palette.
 * Uses median-cut algorithm.
 *
 * @param rgb_data    Raw RGB data (3 bytes per pixel)
 * @param width       Image width
 * @param height      Image height
 * @param max_colors  Maximum palette size (2-256)
 * @param result      Output quantized image
 * @return 0 on success, -1 on error
 */
int tui_quantize_image(const unsigned char *rgb_data,
                       int width, int height,
                       int max_colors,
                       tui_quantized_image *result);

/**
 * Free quantized image resources.
 */
void tui_quantized_image_free(tui_quantized_image *img);

/**
 * Apply Floyd-Steinberg dithering.
 * Modifies indices in place based on original RGB data.
 */
void tui_dither_floyd_steinberg(tui_quantized_image *img,
                                 const unsigned char *original_rgb);

#endif /* TUI_SIXEL_GRAPHICS_H */
