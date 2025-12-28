/*
  +----------------------------------------------------------------------+
  | ext-tui: Animation utilities (easing, interpolation)                |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_ANIMATION_H
#define TUI_ANIMATION_H

#include <stdint.h>
#include "../node/node.h"

/**
 * Easing function types.
 */
typedef enum {
    TUI_EASE_LINEAR,
    TUI_EASE_IN_QUAD,
    TUI_EASE_OUT_QUAD,
    TUI_EASE_IN_OUT_QUAD,
    TUI_EASE_IN_CUBIC,
    TUI_EASE_OUT_CUBIC,
    TUI_EASE_IN_OUT_CUBIC,
    TUI_EASE_IN_QUART,
    TUI_EASE_OUT_QUART,
    TUI_EASE_IN_OUT_QUART,
    TUI_EASE_IN_SINE,
    TUI_EASE_OUT_SINE,
    TUI_EASE_IN_OUT_SINE,
    TUI_EASE_IN_EXPO,
    TUI_EASE_OUT_EXPO,
    TUI_EASE_IN_OUT_EXPO,
    TUI_EASE_IN_CIRC,
    TUI_EASE_OUT_CIRC,
    TUI_EASE_IN_OUT_CIRC,
    TUI_EASE_IN_ELASTIC,
    TUI_EASE_OUT_ELASTIC,
    TUI_EASE_IN_OUT_ELASTIC,
    TUI_EASE_IN_BACK,
    TUI_EASE_OUT_BACK,
    TUI_EASE_IN_OUT_BACK,
    TUI_EASE_IN_BOUNCE,
    TUI_EASE_OUT_BOUNCE,
    TUI_EASE_IN_OUT_BOUNCE
} tui_easing;

/**
 * Apply easing function to progress value.
 *
 * @param t Progress value 0.0 to 1.0
 * @param easing Easing function
 * @return Eased value 0.0 to 1.0
 */
double tui_ease(double t, tui_easing easing);

/**
 * Parse easing name string to enum.
 *
 * @param name Easing name (e.g., "linear", "in-quad", "out-bounce")
 * @return Easing enum value, TUI_EASE_LINEAR if unknown
 */
tui_easing tui_ease_from_name(const char *name);

/**
 * Linear interpolation between two values.
 *
 * @param a Start value
 * @param b End value
 * @param t Progress (0.0 - 1.0)
 * @return Interpolated value
 */
double tui_lerp(double a, double b, double t);

/**
 * Interpolate between two colors.
 *
 * @param a Start color
 * @param b End color
 * @param t Progress (0.0 - 1.0)
 * @return Interpolated color
 */
tui_color tui_lerp_color(tui_color a, tui_color b, double t);

/**
 * Generate gradient colors.
 *
 * @param colors Input color array
 * @param color_count Number of input colors
 * @param output Output color array (must be pre-allocated)
 * @param steps Number of output colors to generate
 */
void tui_gradient(tui_color *colors, int color_count, tui_color *output, int steps);

/**
 * Convert hex color string to tui_color.
 *
 * @param hex Hex color string (e.g., "#ff0000" or "ff0000")
 * @return Parsed color
 */
tui_color tui_color_from_hex(const char *hex);

/**
 * Convert tui_color to hex string.
 *
 * @param color Color to convert
 * @param buf Output buffer (must be at least 8 bytes)
 * @param buf_size Size of output buffer
 */
void tui_color_to_hex(tui_color color, char *buf, size_t buf_size);

#endif /* TUI_ANIMATION_H */
