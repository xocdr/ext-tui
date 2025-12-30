/*
  +----------------------------------------------------------------------+
  | ext-tui: Progress bar and spinner rendering                         |
  +----------------------------------------------------------------------+
*/

#include "progress.h"
#include "animation.h"
#include <string.h>
#include <math.h>

/* Standard block characters */
#define CHAR_FULL_BLOCK  0x2588  /* █ */
#define CHAR_LIGHT_SHADE 0x2591  /* ░ */
#define CHAR_MEDIUM_SHADE 0x2592 /* ▒ */
#define CHAR_DARK_SHADE  0x2593  /* ▓ */

/* Partial block characters for sub-character precision */
static const uint32_t partial_blocks[] = {
    ' ',        /* 0/8 */
    0x258F,     /* 1/8 ▏ */
    0x258E,     /* 2/8 ▎ */
    0x258D,     /* 3/8 ▍ */
    0x258C,     /* 4/8 ▌ */
    0x258B,     /* 5/8 ▋ */
    0x258A,     /* 6/8 ▊ */
    0x2589,     /* 7/8 ▉ */
    0x2588      /* 8/8 █ */
};

/* Spinner frame definitions */
static const char* spinner_dots[] = {
    "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"
};

static const char* spinner_line[] = {
    "|", "/", "-", "\\"
};

static const char* spinner_circle[] = {
    "◐", "◓", "◑", "◒"
};

static const char* spinner_arrow[] = {
    "←", "↖", "↑", "↗", "→", "↘", "↓", "↙"
};

static const char* spinner_box[] = {
    "◰", "◳", "◲", "◱"
};

static const char* spinner_bounce[] = {
    "⠁", "⠂", "⠄", "⠂"
};

static const char* spinner_clock[] = {
    "🕐", "🕑", "🕒", "🕓", "🕔", "🕕",
    "🕖", "🕗", "🕘", "🕙", "🕚", "🕛"
};

static const char* spinner_moon[] = {
    "🌑", "🌒", "🌓", "🌔", "🌕", "🌖", "🌗", "🌘"
};

static const char* spinner_earth[] = {
    "🌍", "🌎", "🌏"
};

/* Get spinner frames and count */
static void get_spinner_frames(tui_spinner_type type,
                               const char ***frames, int *count)
{
    switch (type) {
        case TUI_SPINNER_DOTS:
            *frames = spinner_dots;
            *count = 10;
            break;
        case TUI_SPINNER_LINE:
            *frames = spinner_line;
            *count = 4;
            break;
        case TUI_SPINNER_CIRCLE:
            *frames = spinner_circle;
            *count = 4;
            break;
        case TUI_SPINNER_ARROW:
            *frames = spinner_arrow;
            *count = 8;
            break;
        case TUI_SPINNER_BOX:
            *frames = spinner_box;
            *count = 4;
            break;
        case TUI_SPINNER_BOUNCE:
            *frames = spinner_bounce;
            *count = 4;
            break;
        case TUI_SPINNER_CLOCK:
            *frames = spinner_clock;
            *count = 12;
            break;
        case TUI_SPINNER_MOON:
            *frames = spinner_moon;
            *count = 8;
            break;
        case TUI_SPINNER_EARTH:
            *frames = spinner_earth;
            *count = 3;
            break;
        default:
            *frames = spinner_dots;
            *count = 10;
            break;
    }
}

void tui_render_progress(tui_buffer *buf, int x, int y, int width,
                         double progress,
                         uint32_t filled_char, uint32_t empty_char,
                         const tui_style *filled_style,
                         const tui_style *empty_style)
{
    if (!buf || width <= 0) return;

    /* Clamp progress */
    if (progress < 0.0) progress = 0.0;
    if (progress > 1.0) progress = 1.0;

    /* Calculate filled portion with sub-character precision */
    double filled_exact = progress * width;
    int filled_full = (int)filled_exact;
    int partial_index = (int)((filled_exact - filled_full) * 8);

    /* Use default characters if not specified */
    if (filled_char == 0) filled_char = CHAR_FULL_BLOCK;
    if (empty_char == 0) empty_char = CHAR_LIGHT_SHADE;

    /* Render filled portion */
    for (int i = 0; i < filled_full && i < width; i++) {
        tui_buffer_set_cell(buf, x + i, y, filled_char, filled_style);
    }

    /* Render partial block if applicable */
    if (filled_full < width && partial_index > 0) {
        tui_buffer_set_cell(buf, x + filled_full, y,
                           partial_blocks[partial_index], filled_style);
        filled_full++;
    }

    /* Render empty portion */
    for (int i = filled_full; i < width; i++) {
        tui_buffer_set_cell(buf, x + i, y, empty_char, empty_style);
    }
}

void tui_render_busy(tui_buffer *buf, int x, int y, int width,
                     int frame, tui_busy_style style,
                     const tui_style *base_style)
{
    if (!buf || width <= 0) return;

    switch (style) {
        case TUI_BUSY_PULSE:
            {
                /* Bouncing block that moves back and forth */
                int pos = frame % (width * 2);
                if (pos >= width) pos = (width * 2) - pos - 1;

                for (int i = 0; i < width; i++) {
                    if (i == pos) {
                        tui_buffer_set_cell(buf, x + i, y, CHAR_FULL_BLOCK, base_style);
                    } else {
                        tui_buffer_set_cell(buf, x + i, y, CHAR_LIGHT_SHADE, base_style);
                    }
                }
            }
            break;

        case TUI_BUSY_SNAKE:
            {
                /* Growing/shrinking snake */
                int snake_len = 3;
                int cycle = width + snake_len;
                int pos = frame % cycle;

                for (int i = 0; i < width; i++) {
                    int dist = pos - i;
                    if (dist >= 0 && dist < snake_len) {
                        tui_buffer_set_cell(buf, x + i, y, CHAR_FULL_BLOCK, base_style);
                    } else {
                        tui_buffer_set_cell(buf, x + i, y, CHAR_LIGHT_SHADE, base_style);
                    }
                }
            }
            break;

        case TUI_BUSY_WAVE:
            {
                /* Wave pattern */
                for (int i = 0; i < width; i++) {
                    double phase = (double)(i + frame) / 3.0;
                    double val = (sin(phase) + 1.0) / 2.0;
                    uint32_t ch;
                    if (val > 0.75) ch = CHAR_FULL_BLOCK;
                    else if (val > 0.5) ch = CHAR_DARK_SHADE;
                    else if (val > 0.25) ch = CHAR_MEDIUM_SHADE;
                    else ch = CHAR_LIGHT_SHADE;
                    tui_buffer_set_cell(buf, x + i, y, ch, base_style);
                }
            }
            break;

        case TUI_BUSY_SHIMMER:
            {
                /* Shimmer effect - random-ish based on position and frame */
                for (int i = 0; i < width; i++) {
                    int v = ((i * 7 + frame * 3) % 11);
                    uint32_t ch;
                    if (v < 3) ch = CHAR_FULL_BLOCK;
                    else if (v < 5) ch = CHAR_DARK_SHADE;
                    else if (v < 7) ch = CHAR_MEDIUM_SHADE;
                    else ch = CHAR_LIGHT_SHADE;
                    tui_buffer_set_cell(buf, x + i, y, ch, base_style);
                }
            }
            break;

        case TUI_BUSY_GRADIENT:
        case TUI_BUSY_RAINBOW:
        default:
            {
                /* Moving gradient using shade characters */
                for (int i = 0; i < width; i++) {
                    int phase = (i + frame) % 4;
                    uint32_t ch;
                    switch (phase) {
                        case 0: ch = CHAR_FULL_BLOCK; break;
                        case 1: ch = CHAR_DARK_SHADE; break;
                        case 2: ch = CHAR_MEDIUM_SHADE; break;
                        default: ch = CHAR_LIGHT_SHADE; break;
                    }
                    tui_buffer_set_cell(buf, x + i, y, ch, base_style);
                }
            }
            break;
    }
}

void tui_render_busy_gradient(tui_buffer *buf, int x, int y, int width,
                              int frame, tui_color *colors, int color_count,
                              const tui_style *base_style)
{
    if (!buf || width <= 0 || !colors || color_count < 1) return;

    tui_style style = base_style ? *base_style : (tui_style){0};

    for (int i = 0; i < width; i++) {
        /* Calculate color based on position + frame offset */
        double t;
        if (width <= 1) {
            t = 0.0;
        } else {
            t = (double)((i + frame) % width) / (width - 1);
        }
        if (color_count == 1) {
            style.fg = colors[0];
        } else {
            double segment = t * (color_count - 1);
            int idx = (int)segment;
            if (idx >= color_count - 1) idx = color_count - 2;
            double local_t = segment - idx;
            style.fg = tui_lerp_color(colors[idx], colors[idx + 1], local_t);
        }

        tui_buffer_set_cell(buf, x + i, y, CHAR_FULL_BLOCK, &style);
    }
}

int tui_spinner_frame(tui_spinner_type spinner_type, int frame, char *output)
{
    if (!output) return 0;

    const char **frames;
    int count;
    get_spinner_frames(spinner_type, &frames, &count);

    int idx = frame % count;
    const char *ch = frames[idx];
    size_t len = strlen(ch);
    memcpy(output, ch, len);
    output[len] = '\0';

    return (int)len;
}

int tui_spinner_frame_count(tui_spinner_type spinner_type)
{
    const char **frames;
    int count;
    get_spinner_frames(spinner_type, &frames, &count);
    return count;
}

void tui_render_spinner(tui_buffer *buf, int x, int y,
                        tui_spinner_type spinner_type, int frame,
                        const tui_style *style)
{
    if (!buf) return;

    char ch[16];
    tui_spinner_frame(spinner_type, frame, ch);
    tui_buffer_write_text(buf, x, y, ch, style);
}
