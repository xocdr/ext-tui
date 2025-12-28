/*
  +----------------------------------------------------------------------+
  | ext-tui: Progress bar and spinner rendering                         |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_PROGRESS_H
#define TUI_PROGRESS_H

#include "../render/buffer.h"
#include "../node/node.h"

/**
 * Busy/indeterminate bar styles.
 */
typedef enum {
    TUI_BUSY_PULSE,         /* Bouncing block */
    TUI_BUSY_SNAKE,         /* Growing snake */
    TUI_BUSY_GRADIENT,      /* Moving gradient */
    TUI_BUSY_WAVE,          /* Wave pattern */
    TUI_BUSY_SHIMMER,       /* Shimmer effect */
    TUI_BUSY_RAINBOW        /* Rainbow gradient */
} tui_busy_style;

/**
 * Standard spinner frame sets.
 */
typedef enum {
    TUI_SPINNER_DOTS,       /* ⠋ ⠙ ⠹ ⠸ ⠼ ⠴ ⠦ ⠧ ⠇ ⠏ */
    TUI_SPINNER_LINE,       /* | / - \ */
    TUI_SPINNER_CIRCLE,     /* ◐ ◓ ◑ ◒ */
    TUI_SPINNER_ARROW,      /* ← ↖ ↑ ↗ → ↘ ↓ ↙ */
    TUI_SPINNER_BOX,        /* ◰ ◳ ◲ ◱ */
    TUI_SPINNER_BOUNCE,     /* ⠁ ⠂ ⠄ ⠂ */
    TUI_SPINNER_CLOCK,      /* 🕐 🕑 🕒 ... */
    TUI_SPINNER_MOON,       /* 🌑 🌒 🌓 🌔 🌕 🌖 🌗 🌘 */
    TUI_SPINNER_EARTH       /* 🌍 🌎 🌏 */
} tui_spinner_type;

/**
 * Render a progress bar.
 *
 * @param buf Buffer to render to
 * @param x X position
 * @param y Y position
 * @param width Width in characters
 * @param progress Progress value (0.0 - 1.0)
 * @param filled_char Character for filled portion (default: █)
 * @param empty_char Character for empty portion (default: ░)
 * @param filled_style Style for filled portion
 * @param empty_style Style for empty portion
 */
void tui_render_progress(tui_buffer *buf, int x, int y, int width,
                         double progress,
                         uint32_t filled_char, uint32_t empty_char,
                         const tui_style *filled_style,
                         const tui_style *empty_style);

/**
 * Render an animated busy/indeterminate bar.
 *
 * @param buf Buffer to render to
 * @param x X position
 * @param y Y position
 * @param width Width in characters
 * @param frame Animation frame number
 * @param style Busy bar style
 * @param base_style Base style for rendering
 */
void tui_render_busy(tui_buffer *buf, int x, int y, int width,
                     int frame, tui_busy_style style,
                     const tui_style *base_style);

/**
 * Render a busy bar with gradient colors.
 *
 * @param buf Buffer to render to
 * @param x X position
 * @param y Y position
 * @param width Width in characters
 * @param frame Animation frame number
 * @param colors Gradient color array
 * @param color_count Number of colors in gradient
 * @param base_style Base style for rendering
 */
void tui_render_busy_gradient(tui_buffer *buf, int x, int y, int width,
                              int frame, tui_color *colors, int color_count,
                              const tui_style *base_style);

/**
 * Get spinner frame character for current time.
 *
 * @param spinner_type Type of spinner
 * @param frame Frame number
 * @param output Buffer to receive UTF-8 character (at least 8 bytes)
 * @return Number of bytes written to output
 */
int tui_spinner_frame(tui_spinner_type spinner_type, int frame, char *output);

/**
 * Get total frame count for a spinner type.
 *
 * @param spinner_type Type of spinner
 * @return Number of frames in animation
 */
int tui_spinner_frame_count(tui_spinner_type spinner_type);

/**
 * Render a spinner character.
 *
 * @param buf Buffer to render to
 * @param x X position
 * @param y Y position
 * @param spinner_type Type of spinner
 * @param frame Frame number
 * @param style Style for rendering
 */
void tui_render_spinner(tui_buffer *buf, int x, int y,
                        tui_spinner_type spinner_type, int frame,
                        const tui_style *style);

#endif /* TUI_PROGRESS_H */
