/*
  +----------------------------------------------------------------------+
  | ext-tui: Kitty graphics protocol support                             |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_KITTY_GRAPHICS_H
#define TUI_KITTY_GRAPHICS_H

#include <stddef.h>
#include <stdint.h>

/* Image format enumeration - values match Kitty protocol f= parameter */
typedef enum {
    TUI_GRAPHICS_PNG = 100,    /* Kitty f=100 - PNG format */
    TUI_GRAPHICS_RGB = 24,     /* Kitty f=24 - Raw RGB (3 bytes/pixel) */
    TUI_GRAPHICS_RGBA = 32     /* Kitty f=32 - Raw RGBA (4 bytes/pixel) */
} tui_graphics_format;

/* Image state tracking */
typedef enum {
    TUI_IMAGE_STATE_EMPTY = 0,      /* No data loaded */
    TUI_IMAGE_STATE_LOADED,         /* Data loaded, not transmitted */
    TUI_IMAGE_STATE_TRANSMITTED,    /* Sent to terminal memory */
    TUI_IMAGE_STATE_DISPLAYED       /* Actively displayed on screen */
} tui_image_state;

/* Image structure */
typedef struct {
    /* Image data */
    unsigned char *data;            /* Raw pixel or PNG data */
    size_t data_len;                /* Data length in bytes */

    /* Image dimensions (pixels) */
    int width;
    int height;

    /* Format */
    tui_graphics_format format;

    /* Terminal-assigned IDs */
    uint32_t image_id;              /* Unique image ID (i= parameter) */
    uint32_t placement_id;          /* Placement ID (p= parameter) */

    /* State tracking */
    tui_image_state state;

    /* Display info (when displayed) */
    int display_x;                  /* Terminal column position */
    int display_y;                  /* Terminal row position */
    int display_cols;               /* Cell width (c= parameter) */
    int display_rows;               /* Cell height (r= parameter) */

    /* Options */
    int z_index;                    /* Z-layer for stacking (z= parameter) */
    int delete_on_free;             /* Auto-delete from terminal on resource free */
} tui_image;

/* Chunk size for transmission (4096 bytes is Kitty's recommendation) */
#define TUI_GRAPHICS_CHUNK_SIZE 4096

/* Maximum escape sequence buffer size for a single chunk */
#define TUI_GRAPHICS_MAX_ESCAPE_SIZE 8192

/* PNG file signature (first 8 bytes) */
#define TUI_PNG_SIGNATURE "\x89PNG\r\n\x1a\n"
#define TUI_PNG_SIGNATURE_LEN 8

/* ============================================================================
 * Image Lifecycle Functions
 * ============================================================================ */

/**
 * Allocate and initialize an empty image structure.
 * @return Newly allocated tui_image or NULL on failure
 */
tui_image *tui_image_alloc(void);

/**
 * Initialize an image structure (for stack-allocated images).
 * @param img  Image structure to initialize
 */
void tui_image_init(tui_image *img);

/**
 * Load image from file path.
 * Automatically detects PNG format from file signature.
 * For PNG files, stores raw file data.
 * @param img   Image structure
 * @param path  File path to image
 * @return 0 on success, -1 on error
 */
int tui_image_load_file(tui_image *img, const char *path);

/**
 * Load image from raw pixel data.
 * @param img     Image structure
 * @param data    Raw pixel data (RGB or RGBA) or PNG data
 * @param len     Data length in bytes
 * @param width   Image width in pixels
 * @param height  Image height in pixels
 * @param format  TUI_GRAPHICS_RGB, TUI_GRAPHICS_RGBA, or TUI_GRAPHICS_PNG
 * @return 0 on success, -1 on error
 */
int tui_image_load_data(tui_image *img, const unsigned char *data,
                        size_t len, int width, int height,
                        tui_graphics_format format);

/**
 * Free image resources.
 * If delete_on_free is set and image was transmitted, sends delete command.
 * @param img  Image to free
 */
void tui_image_free(tui_image *img);

/* ============================================================================
 * Transmission Functions
 * ============================================================================ */

/**
 * Transmit image to terminal.
 * Uses chunked transmission for large images.
 * After success, img->image_id is set and state becomes TRANSMITTED.
 * @param img  Image to transmit
 * @return 0 on success, -1 on error
 */
int tui_image_transmit(tui_image *img);

/* ============================================================================
 * Display Functions
 * ============================================================================ */

/**
 * Display a transmitted image at cursor position.
 * Image must be in TRANSMITTED or DISPLAYED state.
 * @param img   Image to display
 * @param x     Terminal column (0-based)
 * @param y     Terminal row (0-based)
 * @param cols  Cell width (0 = auto from pixel dimensions)
 * @param rows  Cell height (0 = auto from pixel dimensions)
 * @return 0 on success, -1 on error
 */
int tui_image_display(tui_image *img, int x, int y, int cols, int rows);

/**
 * Display image at position (transmit first if needed).
 * Convenience function that combines transmit + display.
 * @param img   Image to display
 * @param x     Terminal column (0-based)
 * @param y     Terminal row (0-based)
 * @param cols  Cell width (0 = auto)
 * @param rows  Cell height (0 = auto)
 * @return 0 on success, -1 on error
 */
int tui_image_display_at(tui_image *img, int x, int y, int cols, int rows);

/* ============================================================================
 * Deletion Functions
 * ============================================================================ */

/**
 * Delete image from terminal memory.
 * @param img  Image to delete
 * @return 0 on success, -1 on error
 */
int tui_image_delete(tui_image *img);

/**
 * Delete image by ID.
 * @param image_id  Terminal image ID to delete
 * @return 0 on success, -1 on error
 */
int tui_image_delete_by_id(uint32_t image_id);

/**
 * Clear all images from terminal.
 * @return 0 on success, -1 on error
 */
int tui_graphics_clear_all(void);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * Check if Kitty graphics is supported by the terminal.
 * @return 1 if supported, 0 if not
 */
int tui_graphics_is_supported(void);

/**
 * Get next unique image ID.
 * Thread-safe incrementing counter.
 * @return Next available image ID
 */
uint32_t tui_graphics_next_image_id(void);

#endif /* TUI_KITTY_GRAPHICS_H */
