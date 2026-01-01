/*
  +----------------------------------------------------------------------+
  | ext-tui: Graphics protocol PHP bindings                              |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
  | Supports multiple graphics protocols:                                 |
  | - Kitty Graphics Protocol (preferred)                                |
  | - iTerm2 Inline Images Protocol                                       |
  | - Sixel Graphics Protocol (fallback)                                  |
  +----------------------------------------------------------------------+
*/

#include "tui_internal.h"
#include "src/graphics/kitty.h"
#include "src/graphics/iterm2.h"
#include "src/graphics/sixel.h"
#include <string.h>

/* Graphics protocol enumeration */
typedef enum {
    TUI_GRAPHICS_PROTO_NONE = 0,
    TUI_GRAPHICS_PROTO_KITTY,
    TUI_GRAPHICS_PROTO_ITERM2,
    TUI_GRAPHICS_PROTO_SIXEL
} tui_graphics_protocol;

/* Detect best available graphics protocol */
static tui_graphics_protocol detect_graphics_protocol(void)
{
    /* Priority order:
     * 1. Kitty - best quality, widest feature set
     * 2. iTerm2 - good support for PNG, simpler protocol
     * 3. Sixel - fallback for older terminals
     */
    if (tui_graphics_is_supported()) {
        return TUI_GRAPHICS_PROTO_KITTY;
    }
    if (tui_iterm2_is_supported()) {
        return TUI_GRAPHICS_PROTO_ITERM2;
    }
    if (tui_sixel_is_supported()) {
        return TUI_GRAPHICS_PROTO_SIXEL;
    }
    return TUI_GRAPHICS_PROTO_NONE;
}

/* {{{ Resource destructor */
void tui_image_dtor(zend_resource *res)
{
    tui_image *img = (tui_image *)res->ptr;
    if (img) {
        tui_image_free(img);
        free(img);
    }
}
/* }}} */

/* {{{ proto resource tui_image_load(string $path)
   Load image from file path */
PHP_FUNCTION(tui_image_load)
{
    char *path;
    size_t path_len;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STRING(path, path_len)
    ZEND_PARSE_PARAMETERS_END();

    tui_image *img = tui_image_alloc();
    if (!img) {
        zend_throw_exception(tui_resource_exception_ce,
            "Failed to allocate image", 0);
        RETURN_THROWS();
    }

    if (tui_image_load_file(img, path) < 0) {
        free(img);
        zend_throw_exception(tui_resource_exception_ce,
            "Failed to load image file", 0);
        RETURN_THROWS();
    }

    RETURN_RES(zend_register_resource(img, le_tui_image));
}
/* }}} */

/* {{{ proto resource tui_image_create(string $data, int $width, int $height, string $format = 'rgba')
   Create image from raw pixel data */
PHP_FUNCTION(tui_image_create)
{
    char *data;
    size_t data_len;
    zend_long width, height;
    char *format = "rgba";
    size_t format_len = 4;

    ZEND_PARSE_PARAMETERS_START(3, 4)
        Z_PARAM_STRING(data, data_len)
        Z_PARAM_LONG(width)
        Z_PARAM_LONG(height)
        Z_PARAM_OPTIONAL
        Z_PARAM_STRING(format, format_len)
    ZEND_PARSE_PARAMETERS_END();

    /* Validate dimensions */
    if (width <= 0 || height <= 0) {
        zend_throw_exception(tui_invalid_dimension_exception_ce,
            "Width and height must be positive", 0);
        RETURN_THROWS();
    }

    /* Determine format */
    tui_graphics_format fmt;
    size_t expected_len;
    if (strcasecmp(format, "rgba") == 0) {
        fmt = TUI_GRAPHICS_RGBA;
        expected_len = (size_t)(width * height * 4);
    } else if (strcasecmp(format, "rgb") == 0) {
        fmt = TUI_GRAPHICS_RGB;
        expected_len = (size_t)(width * height * 3);
    } else if (strcasecmp(format, "png") == 0) {
        fmt = TUI_GRAPHICS_PNG;
        expected_len = 0;  /* PNG is compressed, size varies */
    } else {
        zend_throw_exception(tui_validation_exception_ce,
            "Format must be 'rgba', 'rgb', or 'png'", 0);
        RETURN_THROWS();
    }

    /* Validate data length for raw formats */
    if (expected_len > 0 && data_len != expected_len) {
        zend_throw_exception(tui_validation_exception_ce,
            "Data length does not match dimensions and format", 0);
        RETURN_THROWS();
    }

    tui_image *img = tui_image_alloc();
    if (!img) {
        zend_throw_exception(tui_resource_exception_ce,
            "Failed to allocate image", 0);
        RETURN_THROWS();
    }

    if (tui_image_load_data(img, (const unsigned char *)data,
                            data_len, (int)width, (int)height, fmt) < 0) {
        free(img);
        zend_throw_exception(tui_resource_exception_ce,
            "Failed to create image from data", 0);
        RETURN_THROWS();
    }

    RETURN_RES(zend_register_resource(img, le_tui_image));
}
/* }}} */

/* {{{ proto bool tui_image_transmit(resource $image)
   Transmit image to terminal memory */
PHP_FUNCTION(tui_image_transmit)
{
    zval *zimage;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zimage)
    ZEND_PARSE_PARAMETERS_END();

    tui_image *img = (tui_image *)zend_fetch_resource(
        Z_RES_P(zimage), TUI_IMAGE_RES_NAME, le_tui_image);

    if (!img) {
        RETURN_FALSE;
    }

    RETURN_BOOL(tui_image_transmit(img) == 0);
}
/* }}} */

/* {{{ proto bool tui_image_display(resource $image, int $x, int $y, int $cols = 0, int $rows = 0)
   Display image at position using best available protocol */
PHP_FUNCTION(tui_image_display)
{
    zval *zimage;
    zend_long x, y;
    zend_long cols = 0, rows = 0;

    ZEND_PARSE_PARAMETERS_START(3, 5)
        Z_PARAM_RESOURCE(zimage)
        Z_PARAM_LONG(x)
        Z_PARAM_LONG(y)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(cols)
        Z_PARAM_LONG(rows)
    ZEND_PARSE_PARAMETERS_END();

    tui_image *img = (tui_image *)zend_fetch_resource(
        Z_RES_P(zimage), TUI_IMAGE_RES_NAME, le_tui_image);

    if (!img) {
        RETURN_FALSE;
    }

    /* Auto-detect best protocol and use it */
    tui_graphics_protocol proto = detect_graphics_protocol();
    int result = -1;

    switch (proto) {
        case TUI_GRAPHICS_PROTO_KITTY:
            result = tui_image_display_at(img, (int)x, (int)y,
                                          (int)cols, (int)rows);
            break;

        case TUI_GRAPHICS_PROTO_ITERM2:
            result = tui_iterm2_display(img, (int)x, (int)y,
                                        (int)cols, (int)rows);
            break;

        case TUI_GRAPHICS_PROTO_SIXEL:
            result = tui_sixel_display(img, (int)x, (int)y, NULL);
            break;

        case TUI_GRAPHICS_PROTO_NONE:
        default:
            result = -1;
            break;
    }

    RETURN_BOOL(result == 0);
}
/* }}} */

/* {{{ proto bool tui_image_delete(resource $image)
   Delete image from terminal memory */
PHP_FUNCTION(tui_image_delete)
{
    zval *zimage;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zimage)
    ZEND_PARSE_PARAMETERS_END();

    tui_image *img = (tui_image *)zend_fetch_resource(
        Z_RES_P(zimage), TUI_IMAGE_RES_NAME, le_tui_image);

    if (!img) {
        RETURN_FALSE;
    }

    RETURN_BOOL(tui_image_delete(img) == 0);
}
/* }}} */

/* {{{ proto void tui_image_clear()
   Clear all images from terminal */
PHP_FUNCTION(tui_image_clear)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_graphics_clear_all();
}
/* }}} */

/* {{{ proto void tui_image_destroy(resource $image)
   Destroy image resource and free memory */
PHP_FUNCTION(tui_image_destroy)
{
    zval *zimage;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zimage)
    ZEND_PARSE_PARAMETERS_END();

    tui_image *img = (tui_image *)zend_fetch_resource(
        Z_RES_P(zimage), TUI_IMAGE_RES_NAME, le_tui_image);

    if (img) {
        zend_list_close(Z_RES_P(zimage));
    }
}
/* }}} */

/* {{{ proto array tui_image_get_info(resource $image)
   Get image metadata */
PHP_FUNCTION(tui_image_get_info)
{
    zval *zimage;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_RESOURCE(zimage)
    ZEND_PARSE_PARAMETERS_END();

    tui_image *img = (tui_image *)zend_fetch_resource(
        Z_RES_P(zimage), TUI_IMAGE_RES_NAME, le_tui_image);

    if (!img) {
        RETURN_NULL();
    }

    array_init(return_value);
    add_assoc_long(return_value, "width", img->width);
    add_assoc_long(return_value, "height", img->height);
    add_assoc_long(return_value, "data_size", (zend_long)img->data_len);
    add_assoc_long(return_value, "image_id", (zend_long)img->image_id);

    const char *state_str;
    switch (img->state) {
        case TUI_IMAGE_STATE_EMPTY: state_str = "empty"; break;
        case TUI_IMAGE_STATE_LOADED: state_str = "loaded"; break;
        case TUI_IMAGE_STATE_TRANSMITTED: state_str = "transmitted"; break;
        case TUI_IMAGE_STATE_DISPLAYED: state_str = "displayed"; break;
        default: state_str = "unknown";
    }
    add_assoc_string(return_value, "state", state_str);

    const char *format_str;
    switch (img->format) {
        case TUI_GRAPHICS_PNG: format_str = "png"; break;
        case TUI_GRAPHICS_RGB: format_str = "rgb"; break;
        case TUI_GRAPHICS_RGBA: format_str = "rgba"; break;
        default: format_str = "unknown";
    }
    add_assoc_string(return_value, "format", format_str);
}
/* }}} */

/* {{{ proto bool tui_graphics_supported()
   Check if Kitty graphics is supported by the terminal */
PHP_FUNCTION(tui_graphics_supported)
{
    ZEND_PARSE_PARAMETERS_NONE();

    RETURN_BOOL(tui_graphics_is_supported());
}
/* }}} */

/* {{{ proto string tui_graphics_protocol()
   Get the best available graphics protocol name */
PHP_FUNCTION(tui_graphics_protocol)
{
    ZEND_PARSE_PARAMETERS_NONE();

    tui_graphics_protocol proto = detect_graphics_protocol();

    switch (proto) {
        case TUI_GRAPHICS_PROTO_KITTY:
            RETURN_STRING("kitty");
        case TUI_GRAPHICS_PROTO_ITERM2:
            RETURN_STRING("iterm2");
        case TUI_GRAPHICS_PROTO_SIXEL:
            RETURN_STRING("sixel");
        case TUI_GRAPHICS_PROTO_NONE:
        default:
            RETURN_STRING("none");
    }
}
/* }}} */

/* {{{ proto bool tui_iterm2_supported()
   Check if iTerm2 graphics is supported by the terminal */
PHP_FUNCTION(tui_iterm2_supported)
{
    ZEND_PARSE_PARAMETERS_NONE();

    RETURN_BOOL(tui_iterm2_is_supported());
}
/* }}} */

/* {{{ proto bool tui_sixel_supported()
   Check if Sixel graphics is supported by the terminal */
PHP_FUNCTION(tui_sixel_supported)
{
    ZEND_PARSE_PARAMETERS_NONE();

    RETURN_BOOL(tui_sixel_is_supported());
}
/* }}} */
