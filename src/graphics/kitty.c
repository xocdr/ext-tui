/*
  +----------------------------------------------------------------------+
  | ext-tui: Kitty graphics protocol implementation                      |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "kitty.h"
#include "../terminal/ansi.h"
#include "../terminal/capabilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Global image ID counter */
static uint32_t g_next_image_id = 1;

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

uint32_t tui_graphics_next_image_id(void)
{
    return g_next_image_id++;
}

int tui_graphics_is_supported(void)
{
    return tui_has_capability(NULL, TUI_CAP_KITTY_GRAPHICS);
}

/* Check if data starts with PNG signature */
static int is_png_data(const unsigned char *data, size_t len)
{
    if (len < TUI_PNG_SIGNATURE_LEN) return 0;
    return memcmp(data, TUI_PNG_SIGNATURE, TUI_PNG_SIGNATURE_LEN) == 0;
}

/* Write escape sequence to stdout */
static int write_escape(const char *data, size_t len)
{
    size_t written = 0;
    while (written < len) {
        ssize_t n = write(STDOUT_FILENO, data + written, len - written);
        if (n < 0) return -1;
        written += (size_t)n;
    }
    return 0;
}

/* ============================================================================
 * Image Lifecycle Functions
 * ============================================================================ */

void tui_image_init(tui_image *img)
{
    if (!img) return;
    memset(img, 0, sizeof(*img));
    img->state = TUI_IMAGE_STATE_EMPTY;
    img->delete_on_free = 1;  /* Default: auto-delete from terminal */
}

tui_image *tui_image_alloc(void)
{
    tui_image *img = malloc(sizeof(tui_image));
    if (img) {
        tui_image_init(img);
    }
    return img;
}

int tui_image_load_file(tui_image *img, const char *path)
{
    if (!img || !path) return -1;

    FILE *fp = fopen(path, "rb");
    if (!fp) return -1;

    /* Get file size */
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return -1;
    }
    long size = ftell(fp);
    if (size <= 0 || size > 100 * 1024 * 1024) {  /* Max 100MB */
        fclose(fp);
        return -1;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return -1;
    }

    /* Allocate buffer */
    unsigned char *data = malloc((size_t)size);
    if (!data) {
        fclose(fp);
        return -1;
    }

    /* Read file */
    size_t read_len = fread(data, 1, (size_t)size, fp);
    fclose(fp);

    if (read_len != (size_t)size) {
        free(data);
        return -1;
    }

    /* Free existing data */
    if (img->data) {
        free(img->data);
    }

    /* Store data */
    img->data = data;
    img->data_len = read_len;

    /* Detect format from signature */
    if (is_png_data(data, read_len)) {
        img->format = TUI_GRAPHICS_PNG;
        /* PNG dimensions would require parsing IHDR chunk */
        /* For now, set to 0 (terminal will determine from PNG) */
        img->width = 0;
        img->height = 0;
    } else {
        /* Assume raw RGBA if not PNG */
        img->format = TUI_GRAPHICS_RGBA;
        img->width = 0;
        img->height = 0;
    }

    img->state = TUI_IMAGE_STATE_LOADED;
    return 0;
}

int tui_image_load_data(tui_image *img, const unsigned char *data,
                        size_t len, int width, int height,
                        tui_graphics_format format)
{
    if (!img || !data || len == 0) return -1;

    /* Validate dimensions for raw formats */
    if (format == TUI_GRAPHICS_RGB) {
        size_t expected = (size_t)width * (size_t)height * 3;
        if (len != expected) return -1;
    } else if (format == TUI_GRAPHICS_RGBA) {
        size_t expected = (size_t)width * (size_t)height * 4;
        if (len != expected) return -1;
    }
    /* PNG format: size varies, no validation needed */

    /* Free existing data */
    if (img->data) {
        free(img->data);
    }

    /* Copy data */
    img->data = malloc(len);
    if (!img->data) return -1;
    memcpy(img->data, data, len);

    img->data_len = len;
    img->width = width;
    img->height = height;
    img->format = format;
    img->state = TUI_IMAGE_STATE_LOADED;

    return 0;
}

void tui_image_free(tui_image *img)
{
    if (!img) return;

    /* Delete from terminal if requested and transmitted */
    if (img->delete_on_free && img->state >= TUI_IMAGE_STATE_TRANSMITTED) {
        tui_image_delete(img);
    }

    /* Free data buffer */
    if (img->data) {
        free(img->data);
        img->data = NULL;
    }

    img->data_len = 0;
    img->state = TUI_IMAGE_STATE_EMPTY;
}

/* ============================================================================
 * Transmission Functions
 * ============================================================================ */

int tui_image_transmit(tui_image *img)
{
    if (!img || !img->data || img->data_len == 0) {
        return -1;
    }

    if (!tui_graphics_is_supported()) {
        return -1;
    }

    /* Assign unique ID if not already assigned */
    if (img->image_id == 0) {
        img->image_id = tui_graphics_next_image_id();
    }

    /* Calculate base64 size */
    size_t b64_len = tui_base64_encode_len(img->data_len);
    char *b64_data = malloc(b64_len + 1);
    if (!b64_data) return -1;

    /* Encode entire image to base64 */
    int encoded = tui_base64_encode((const char *)img->data, img->data_len,
                                     b64_data, b64_len + 1);
    if (encoded < 0) {
        free(b64_data);
        return -1;
    }

    /* Allocate buffer for escape sequence */
    char *escape_buf = malloc(TUI_GRAPHICS_MAX_ESCAPE_SIZE);
    if (!escape_buf) {
        free(b64_data);
        return -1;
    }

    /* Transmit in chunks */
    size_t offset = 0;
    size_t remaining = (size_t)encoded;
    int is_first = 1;
    int result = 0;

    while (remaining > 0) {
        size_t chunk_size = remaining > TUI_GRAPHICS_CHUNK_SIZE
                          ? TUI_GRAPHICS_CHUNK_SIZE : remaining;
        int is_last = (remaining - chunk_size == 0);
        int more = is_last ? 0 : 1;

        int escape_len;
        if (is_first) {
            /* First chunk includes metadata */
            if (img->format == TUI_GRAPHICS_PNG) {
                escape_len = snprintf(escape_buf, TUI_GRAPHICS_MAX_ESCAPE_SIZE,
                    "\x1b_Ga=T,f=%d,i=%u,q=2,m=%d;",
                    img->format, img->image_id, more);
            } else {
                /* RGB/RGBA requires dimensions */
                escape_len = snprintf(escape_buf, TUI_GRAPHICS_MAX_ESCAPE_SIZE,
                    "\x1b_Ga=T,f=%d,s=%d,v=%d,i=%u,q=2,m=%d;",
                    img->format, img->width, img->height, img->image_id, more);
            }
        } else {
            /* Subsequent chunks only have continuation flag */
            escape_len = snprintf(escape_buf, TUI_GRAPHICS_MAX_ESCAPE_SIZE,
                "\x1b_Gm=%d;", more);
        }

        if (escape_len < 0 || (size_t)escape_len >= TUI_GRAPHICS_MAX_ESCAPE_SIZE - chunk_size - 3) {
            result = -1;
            break;
        }

        /* Append chunk data */
        memcpy(escape_buf + escape_len, b64_data + offset, chunk_size);
        escape_len += (int)chunk_size;

        /* Append string terminator */
        escape_buf[escape_len++] = '\x1b';
        escape_buf[escape_len++] = '\\';

        /* Write to terminal */
        if (write_escape(escape_buf, (size_t)escape_len) < 0) {
            result = -1;
            break;
        }

        offset += chunk_size;
        remaining -= chunk_size;
        is_first = 0;
    }

    free(escape_buf);
    free(b64_data);

    if (result == 0) {
        img->state = TUI_IMAGE_STATE_TRANSMITTED;
    }

    return result;
}

/* ============================================================================
 * Display Functions
 * ============================================================================ */

int tui_image_display(tui_image *img, int x, int y, int cols, int rows)
{
    if (!img) return -1;

    /* Must be transmitted first */
    if (img->state < TUI_IMAGE_STATE_TRANSMITTED) {
        return -1;
    }

    if (!tui_graphics_is_supported()) {
        return -1;
    }

    char escape_buf[256];
    int len;

    /* Move cursor to position first */
    char cursor_buf[32];
    size_t cursor_len;
    tui_ansi_cursor_move(cursor_buf, &cursor_len, x, y);
    if (write_escape(cursor_buf, cursor_len) < 0) {
        return -1;
    }

    /* Send display command (placement) */
    if (cols > 0 && rows > 0) {
        len = snprintf(escape_buf, sizeof(escape_buf),
            "\x1b_Ga=p,i=%u,c=%d,r=%d,q=2;\x1b\\",
            img->image_id, cols, rows);
    } else if (cols > 0) {
        len = snprintf(escape_buf, sizeof(escape_buf),
            "\x1b_Ga=p,i=%u,c=%d,q=2;\x1b\\",
            img->image_id, cols);
    } else if (rows > 0) {
        len = snprintf(escape_buf, sizeof(escape_buf),
            "\x1b_Ga=p,i=%u,r=%d,q=2;\x1b\\",
            img->image_id, rows);
    } else {
        len = snprintf(escape_buf, sizeof(escape_buf),
            "\x1b_Ga=p,i=%u,q=2;\x1b\\",
            img->image_id);
    }

    if (len < 0 || (size_t)len >= sizeof(escape_buf)) {
        return -1;
    }

    if (write_escape(escape_buf, (size_t)len) < 0) {
        return -1;
    }

    /* Update display info */
    img->display_x = x;
    img->display_y = y;
    img->display_cols = cols;
    img->display_rows = rows;
    img->state = TUI_IMAGE_STATE_DISPLAYED;

    return 0;
}

int tui_image_display_at(tui_image *img, int x, int y, int cols, int rows)
{
    if (!img) return -1;

    /* Transmit if not already transmitted */
    if (img->state < TUI_IMAGE_STATE_TRANSMITTED) {
        if (tui_image_transmit(img) < 0) {
            return -1;
        }
    }

    return tui_image_display(img, x, y, cols, rows);
}

/* ============================================================================
 * Deletion Functions
 * ============================================================================ */

int tui_image_delete(tui_image *img)
{
    if (!img || img->image_id == 0) return -1;

    int result = tui_image_delete_by_id(img->image_id);

    if (result == 0) {
        img->state = TUI_IMAGE_STATE_LOADED;  /* Revert to loaded state */
    }

    return result;
}

int tui_image_delete_by_id(uint32_t image_id)
{
    if (image_id == 0) return -1;

    if (!tui_graphics_is_supported()) {
        return -1;
    }

    char escape_buf[64];
    int len = snprintf(escape_buf, sizeof(escape_buf),
        "\x1b_Ga=d,d=I,i=%u,q=2;\x1b\\", image_id);

    if (len < 0 || (size_t)len >= sizeof(escape_buf)) {
        return -1;
    }

    return write_escape(escape_buf, (size_t)len);
}

int tui_graphics_clear_all(void)
{
    if (!tui_graphics_is_supported()) {
        return -1;
    }

    /* Delete all images: a=d,d=a (all) */
    const char *escape = "\x1b_Ga=d,d=a,q=2;\x1b\\";
    return write_escape(escape, strlen(escape));
}
