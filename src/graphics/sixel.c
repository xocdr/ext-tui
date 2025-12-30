/*
  +----------------------------------------------------------------------+
  | ext-tui: Sixel graphics protocol support                             |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "sixel.h"
#include "../terminal/ansi.h"
#include "../terminal/capabilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Write all bytes to stdout.
 */
static void write_stdout(const char *buf, size_t len)
{
    size_t written = 0;
    while (written < len) {
        ssize_t n = write(STDOUT_FILENO, buf + written, len - written);
        if (n <= 0) break;
        written += (size_t)n;
    }
}

int tui_sixel_is_supported(void)
{
    const tui_capabilities *caps = tui_get_capabilities();
    return tui_has_capability(caps, TUI_CAP_SIXEL);
}

/* Color box for median cut */
typedef struct {
    int r_min, r_max;
    int g_min, g_max;
    int b_min, b_max;
    int count;
    int *pixels;  /* Indices into original pixel array */
    int pixel_count;
} color_box;

/* Find color distance squared */
static int color_dist_sq(const tui_rgb *a, const tui_rgb *b)
{
    int dr = (int)a->r - (int)b->r;
    int dg = (int)a->g - (int)b->g;
    int db = (int)a->b - (int)b->b;
    return dr * dr + dg * dg + db * db;
}

/* Find nearest palette color */
static int find_nearest_color(const tui_rgb *color, const tui_rgb *palette, int palette_size)
{
    int best = 0;
    int best_dist = color_dist_sq(color, &palette[0]);

    for (int i = 1; i < palette_size; i++) {
        int dist = color_dist_sq(color, &palette[i]);
        if (dist < best_dist) {
            best_dist = dist;
            best = i;
        }
    }
    return best;
}

/* Calculate average color of a box */
static void box_average_color(const color_box *box, const unsigned char *rgb_data,
                               tui_rgb *out)
{
    if (box->pixel_count == 0) {
        out->r = out->g = out->b = 0;
        return;
    }

    long r_sum = 0, g_sum = 0, b_sum = 0;
    for (int i = 0; i < box->pixel_count; i++) {
        int idx = box->pixels[i] * 3;
        r_sum += rgb_data[idx];
        g_sum += rgb_data[idx + 1];
        b_sum += rgb_data[idx + 2];
    }

    out->r = (uint8_t)(r_sum / box->pixel_count);
    out->g = (uint8_t)(g_sum / box->pixel_count);
    out->b = (uint8_t)(b_sum / box->pixel_count);
}

/* Calculate box bounds */
static void box_calc_bounds(color_box *box, const unsigned char *rgb_data)
{
    if (box->pixel_count == 0) {
        box->r_min = box->r_max = 0;
        box->g_min = box->g_max = 0;
        box->b_min = box->b_max = 0;
        return;
    }

    int idx = box->pixels[0] * 3;
    box->r_min = box->r_max = rgb_data[idx];
    box->g_min = box->g_max = rgb_data[idx + 1];
    box->b_min = box->b_max = rgb_data[idx + 2];

    for (int i = 1; i < box->pixel_count; i++) {
        idx = box->pixels[i] * 3;
        int r = rgb_data[idx];
        int g = rgb_data[idx + 1];
        int b = rgb_data[idx + 2];

        if (r < box->r_min) box->r_min = r;
        if (r > box->r_max) box->r_max = r;
        if (g < box->g_min) box->g_min = g;
        if (g > box->g_max) box->g_max = g;
        if (b < box->b_min) box->b_min = b;
        if (b > box->b_max) box->b_max = b;
    }
}

/* Comparison function for sorting by red */
static int compare_r;
static const unsigned char *sort_rgb_data;
static int cmp_pixels_r(const void *a, const void *b)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (int)sort_rgb_data[ia * 3] - (int)sort_rgb_data[ib * 3];
}
static int cmp_pixels_g(const void *a, const void *b)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (int)sort_rgb_data[ia * 3 + 1] - (int)sort_rgb_data[ib * 3 + 1];
}
static int cmp_pixels_b(const void *a, const void *b)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (int)sort_rgb_data[ia * 3 + 2] - (int)sort_rgb_data[ib * 3 + 2];
}

/* Split box along longest axis */
static int split_box(color_box *box, color_box *new_box, const unsigned char *rgb_data)
{
    int r_range = box->r_max - box->r_min;
    int g_range = box->g_max - box->g_min;
    int b_range = box->b_max - box->b_min;

    /* Sort pixels along longest axis */
    sort_rgb_data = rgb_data;
    if (r_range >= g_range && r_range >= b_range) {
        qsort(box->pixels, (size_t)box->pixel_count, sizeof(int), cmp_pixels_r);
    } else if (g_range >= b_range) {
        qsort(box->pixels, (size_t)box->pixel_count, sizeof(int), cmp_pixels_g);
    } else {
        qsort(box->pixels, (size_t)box->pixel_count, sizeof(int), cmp_pixels_b);
    }

    /* Split at median */
    int mid = box->pixel_count / 2;

    new_box->pixels = box->pixels + mid;
    new_box->pixel_count = box->pixel_count - mid;
    box->pixel_count = mid;

    /* Recalculate bounds */
    box_calc_bounds(box, rgb_data);
    box_calc_bounds(new_box, rgb_data);

    return 0;
}

int tui_quantize_image(const unsigned char *rgb_data,
                       int width, int height,
                       int max_colors,
                       tui_quantized_image *result)
{
    if (!rgb_data || width <= 0 || height <= 0 || max_colors < 2 || !result) {
        return -1;
    }

    if (max_colors > TUI_SIXEL_MAX_COLORS) {
        max_colors = TUI_SIXEL_MAX_COLORS;
    }

    int pixel_count = width * height;

    /* Allocate pixel indices array */
    int *all_pixels = malloc((size_t)pixel_count * sizeof(int));
    if (!all_pixels) {
        return -1;
    }

    for (int i = 0; i < pixel_count; i++) {
        all_pixels[i] = i;
    }

    /* Allocate boxes array */
    color_box *boxes = malloc((size_t)max_colors * sizeof(color_box));
    if (!boxes) {
        free(all_pixels);
        return -1;
    }

    /* Initialize first box with all pixels */
    boxes[0].pixels = all_pixels;
    boxes[0].pixel_count = pixel_count;
    box_calc_bounds(&boxes[0], rgb_data);
    int num_boxes = 1;

    /* Median cut: split boxes until we have max_colors */
    while (num_boxes < max_colors) {
        /* Find box with largest range to split */
        int best_box = -1;
        int best_range = 0;

        for (int i = 0; i < num_boxes; i++) {
            if (boxes[i].pixel_count < 2) continue;

            int r_range = boxes[i].r_max - boxes[i].r_min;
            int g_range = boxes[i].g_max - boxes[i].g_min;
            int b_range = boxes[i].b_max - boxes[i].b_min;
            int range = (r_range > g_range) ? r_range : g_range;
            range = (range > b_range) ? range : b_range;

            if (range > best_range) {
                best_range = range;
                best_box = i;
            }
        }

        if (best_box < 0 || best_range == 0) {
            break;  /* No more boxes to split */
        }

        /* Split the box */
        split_box(&boxes[best_box], &boxes[num_boxes], rgb_data);
        num_boxes++;
    }

    /* Build palette from box averages */
    result->palette = malloc((size_t)num_boxes * sizeof(tui_rgb));
    if (!result->palette) {
        free(boxes);
        free(all_pixels);
        return -1;
    }

    for (int i = 0; i < num_boxes; i++) {
        box_average_color(&boxes[i], rgb_data, &result->palette[i]);
    }
    result->palette_size = num_boxes;

    /* Map each pixel to nearest palette color */
    result->indices = malloc((size_t)pixel_count);
    if (!result->indices) {
        free(result->palette);
        free(boxes);
        free(all_pixels);
        return -1;
    }

    for (int i = 0; i < pixel_count; i++) {
        tui_rgb color;
        color.r = rgb_data[i * 3];
        color.g = rgb_data[i * 3 + 1];
        color.b = rgb_data[i * 3 + 2];
        result->indices[i] = (uint8_t)find_nearest_color(&color, result->palette,
                                                         result->palette_size);
    }

    result->width = width;
    result->height = height;

    free(boxes);
    free(all_pixels);
    return 0;
}

void tui_quantized_image_free(tui_quantized_image *img)
{
    if (img) {
        free(img->indices);
        free(img->palette);
        img->indices = NULL;
        img->palette = NULL;
        img->palette_size = 0;
    }
}

void tui_dither_floyd_steinberg(tui_quantized_image *img,
                                 const unsigned char *original_rgb)
{
    if (!img || !img->indices || !img->palette || !original_rgb) {
        return;
    }

    int width = img->width;
    int height = img->height;

    /* Work buffer for error diffusion */
    int *error_r = calloc((size_t)(width + 2) * 2, sizeof(int));
    int *error_g = calloc((size_t)(width + 2) * 2, sizeof(int));
    int *error_b = calloc((size_t)(width + 2) * 2, sizeof(int));

    if (!error_r || !error_g || !error_b) {
        free(error_r);
        free(error_g);
        free(error_b);
        return;
    }

    int *curr_err_r = error_r + 1;
    int *next_err_r = error_r + width + 3;
    int *curr_err_g = error_g + 1;
    int *next_err_g = error_g + width + 3;
    int *curr_err_b = error_b + 1;
    int *next_err_b = error_b + width + 3;

    for (int y = 0; y < height; y++) {
        memset(next_err_r - 1, 0, (size_t)(width + 2) * sizeof(int));
        memset(next_err_g - 1, 0, (size_t)(width + 2) * sizeof(int));
        memset(next_err_b - 1, 0, (size_t)(width + 2) * sizeof(int));

        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            int rgb_idx = idx * 3;

            /* Get original color + accumulated error */
            int r = (int)original_rgb[rgb_idx] + curr_err_r[x];
            int g = (int)original_rgb[rgb_idx + 1] + curr_err_g[x];
            int b = (int)original_rgb[rgb_idx + 2] + curr_err_b[x];

            /* Clamp */
            if (r < 0) r = 0; else if (r > 255) r = 255;
            if (g < 0) g = 0; else if (g > 255) g = 255;
            if (b < 0) b = 0; else if (b > 255) b = 255;

            /* Find nearest palette color */
            tui_rgb color = { (uint8_t)r, (uint8_t)g, (uint8_t)b };
            int nearest = find_nearest_color(&color, img->palette, img->palette_size);
            img->indices[idx] = (uint8_t)nearest;

            /* Calculate error */
            int err_r = r - (int)img->palette[nearest].r;
            int err_g = g - (int)img->palette[nearest].g;
            int err_b = b - (int)img->palette[nearest].b;

            /* Distribute error (Floyd-Steinberg pattern) */
            /* Right: 7/16 */
            curr_err_r[x + 1] += err_r * 7 / 16;
            curr_err_g[x + 1] += err_g * 7 / 16;
            curr_err_b[x + 1] += err_b * 7 / 16;

            /* Bottom-left: 3/16 */
            next_err_r[x - 1] += err_r * 3 / 16;
            next_err_g[x - 1] += err_g * 3 / 16;
            next_err_b[x - 1] += err_b * 3 / 16;

            /* Bottom: 5/16 */
            next_err_r[x] += err_r * 5 / 16;
            next_err_g[x] += err_g * 5 / 16;
            next_err_b[x] += err_b * 5 / 16;

            /* Bottom-right: 1/16 */
            next_err_r[x + 1] += err_r * 1 / 16;
            next_err_g[x + 1] += err_g * 1 / 16;
            next_err_b[x + 1] += err_b * 1 / 16;
        }

        /* Swap rows */
        int *tmp;
        tmp = curr_err_r; curr_err_r = next_err_r; next_err_r = tmp;
        tmp = curr_err_g; curr_err_g = next_err_g; next_err_g = tmp;
        tmp = curr_err_b; curr_err_b = next_err_b; next_err_b = tmp;
    }

    free(error_r);
    free(error_g);
    free(error_b);
}

int tui_image_to_sixel(const unsigned char *rgb_data,
                       int width, int height,
                       const tui_sixel_options *options,
                       char **sixel_out, size_t *sixel_len)
{
    if (!rgb_data || width <= 0 || height <= 0 || !sixel_out || !sixel_len) {
        return -1;
    }

    int max_colors = options ? options->max_colors : 256;
    tui_dither_method dither = options ? options->dither : TUI_DITHER_FLOYD_STEINBERG;

    if (max_colors < 2) max_colors = 2;
    if (max_colors > 256) max_colors = 256;

    /* Quantize image */
    tui_quantized_image quant;
    if (tui_quantize_image(rgb_data, width, height, max_colors, &quant) < 0) {
        return -1;
    }

    /* Apply dithering if requested */
    if (dither == TUI_DITHER_FLOYD_STEINBERG) {
        tui_dither_floyd_steinberg(&quant, rgb_data);
    }

    /* Estimate output size:
     * Header + palette + data
     * Palette: ~20 bytes per color
     * Data: ~2 bytes per pixel (worst case, better with RLE)
     */
    size_t est_size = 64 + (size_t)quant.palette_size * 20 +
                      (size_t)width * (size_t)height * 2;
    char *out = malloc(est_size);
    if (!out) {
        tui_quantized_image_free(&quant);
        return -1;
    }

    size_t pos = 0;

    /* Sixel header: ESC P q */
    out[pos++] = '\x1b';
    out[pos++] = 'P';
    out[pos++] = 'q';

    /* Set raster attributes: "w;h */
    pos += (size_t)snprintf(out + pos, est_size - pos, "\"1;1;%d;%d", width, height);

    /* Define color palette */
    for (int i = 0; i < quant.palette_size; i++) {
        /* RGB values as 0-100% */
        int r = quant.palette[i].r * 100 / 255;
        int g = quant.palette[i].g * 100 / 255;
        int b = quant.palette[i].b * 100 / 255;
        pos += (size_t)snprintf(out + pos, est_size - pos, "#%d;2;%d;%d;%d", i, r, g, b);
    }

    /* Encode pixels as sixel data (6 rows at a time) */
    for (int band = 0; band < height; band += 6) {
        int band_height = (band + 6 <= height) ? 6 : (height - band);

        /* For each color in palette */
        for (int color = 0; color < quant.palette_size; color++) {
            /* Check if this color is used in this band */
            int used = 0;
            for (int y = band; y < band + band_height && !used; y++) {
                for (int x = 0; x < width && !used; x++) {
                    if (quant.indices[y * width + x] == color) {
                        used = 1;
                    }
                }
            }

            if (!used) continue;

            /* Select color */
            pos += (size_t)snprintf(out + pos, est_size - pos, "#%d", color);

            /* Build sixel row for this color */
            int run_len = 0;
            int run_char = -1;

            for (int x = 0; x < width; x++) {
                /* Build 6-bit sixel value for this column */
                int sixel = 0;
                for (int bit = 0; bit < band_height; bit++) {
                    int y = band + bit;
                    if (quant.indices[y * width + x] == color) {
                        sixel |= (1 << bit);
                    }
                }

                int ch = sixel + 63;  /* Sixel chars are 63-126 */

                /* RLE compression */
                if (ch == run_char) {
                    run_len++;
                } else {
                    /* Flush previous run */
                    if (run_len > 0) {
                        if (run_len >= 4) {
                            pos += (size_t)snprintf(out + pos, est_size - pos,
                                                    "!%d%c", run_len, run_char);
                        } else {
                            for (int i = 0; i < run_len; i++) {
                                out[pos++] = (char)run_char;
                            }
                        }
                    }
                    run_char = ch;
                    run_len = 1;
                }
            }

            /* Flush final run */
            if (run_len > 0) {
                if (run_len >= 4) {
                    pos += (size_t)snprintf(out + pos, est_size - pos,
                                            "!%d%c", run_len, run_char);
                } else {
                    for (int i = 0; i < run_len; i++) {
                        out[pos++] = (char)run_char;
                    }
                }
            }

            /* Carriage return (back to start of row) */
            out[pos++] = '$';
        }

        /* Graphics newline (next 6 rows) */
        out[pos++] = '-';
    }

    /* Sixel terminator: ESC \ */
    out[pos++] = '\x1b';
    out[pos++] = '\\';
    out[pos] = '\0';

    tui_quantized_image_free(&quant);

    *sixel_out = out;
    *sixel_len = pos;
    return 0;
}

int tui_sixel_output(const char *sixel_data, size_t len)
{
    if (!sixel_data || len == 0) {
        return -1;
    }
    write_stdout(sixel_data, len);
    return 0;
}

void tui_sixel_free(char *sixel_data)
{
    free(sixel_data);
}

int tui_sixel_display(tui_image *img, int x, int y,
                      const tui_sixel_options *options)
{
    if (!img || !img->data || img->data_len == 0) {
        return -1;
    }

    /* Move cursor to position */
    char cursor_buf[32];
    size_t cursor_len;
    tui_ansi_cursor_move(cursor_buf, &cursor_len, x, y);
    write_stdout(cursor_buf, cursor_len);

    /* For Sixel, we need RGB data (not PNG) */
    if (img->format == TUI_GRAPHICS_PNG) {
        /* PNG decoding would be needed here - for now, return error */
        /* A full implementation would decode the PNG first */
        return -1;
    }

    /* Get RGB data */
    const unsigned char *rgb_data;
    int width = img->width;
    int height = img->height;

    if (img->format == TUI_GRAPHICS_RGBA) {
        /* Convert RGBA to RGB */
        size_t rgb_size = (size_t)width * (size_t)height * 3;
        unsigned char *rgb_buf = malloc(rgb_size);
        if (!rgb_buf) return -1;

        for (int i = 0; i < width * height; i++) {
            rgb_buf[i * 3] = img->data[i * 4];
            rgb_buf[i * 3 + 1] = img->data[i * 4 + 1];
            rgb_buf[i * 3 + 2] = img->data[i * 4 + 2];
        }

        char *sixel;
        size_t sixel_len;
        int result = tui_image_to_sixel(rgb_buf, width, height, options,
                                         &sixel, &sixel_len);
        free(rgb_buf);

        if (result < 0) return -1;

        tui_sixel_output(sixel, sixel_len);
        tui_sixel_free(sixel);
    } else {
        /* Already RGB */
        rgb_data = img->data;

        char *sixel;
        size_t sixel_len;
        if (tui_image_to_sixel(rgb_data, width, height, options,
                                &sixel, &sixel_len) < 0) {
            return -1;
        }

        tui_sixel_output(sixel, sixel_len);
        tui_sixel_free(sixel);
    }

    img->state = TUI_IMAGE_STATE_DISPLAYED;
    img->display_x = x;
    img->display_y = y;

    return 0;
}
