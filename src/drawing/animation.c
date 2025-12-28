/*
  +----------------------------------------------------------------------+
  | ext-tui: Animation utilities (easing, interpolation)                |
  +----------------------------------------------------------------------+
*/

#include "animation.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Easing helper functions */
static double ease_out_bounce(double t)
{
    if (t < 1.0 / 2.75) {
        return 7.5625 * t * t;
    } else if (t < 2.0 / 2.75) {
        t -= 1.5 / 2.75;
        return 7.5625 * t * t + 0.75;
    } else if (t < 2.5 / 2.75) {
        t -= 2.25 / 2.75;
        return 7.5625 * t * t + 0.9375;
    } else {
        t -= 2.625 / 2.75;
        return 7.5625 * t * t + 0.984375;
    }
}

static double ease_in_bounce(double t)
{
    return 1.0 - ease_out_bounce(1.0 - t);
}

double tui_ease(double t, tui_easing easing)
{
    /* Clamp t to [0, 1] */
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;

    switch (easing) {
        case TUI_EASE_LINEAR:
            return t;

        case TUI_EASE_IN_QUAD:
            return t * t;

        case TUI_EASE_OUT_QUAD:
            return t * (2.0 - t);

        case TUI_EASE_IN_OUT_QUAD:
            return t < 0.5 ? 2.0 * t * t : -1.0 + (4.0 - 2.0 * t) * t;

        case TUI_EASE_IN_CUBIC:
            return t * t * t;

        case TUI_EASE_OUT_CUBIC:
            {
                double t1 = t - 1.0;
                return t1 * t1 * t1 + 1.0;
            }

        case TUI_EASE_IN_OUT_CUBIC:
            return t < 0.5 ? 4.0 * t * t * t : (t - 1.0) * (2.0 * t - 2.0) * (2.0 * t - 2.0) + 1.0;

        case TUI_EASE_IN_QUART:
            return t * t * t * t;

        case TUI_EASE_OUT_QUART:
            {
                double t1 = t - 1.0;
                return 1.0 - t1 * t1 * t1 * t1;
            }

        case TUI_EASE_IN_OUT_QUART:
            {
                double t1 = t - 1.0;
                return t < 0.5 ? 8.0 * t * t * t * t : 1.0 - 8.0 * t1 * t1 * t1 * t1;
            }

        case TUI_EASE_IN_SINE:
            return 1.0 - cos(t * M_PI / 2.0);

        case TUI_EASE_OUT_SINE:
            return sin(t * M_PI / 2.0);

        case TUI_EASE_IN_OUT_SINE:
            return -(cos(M_PI * t) - 1.0) / 2.0;

        case TUI_EASE_IN_EXPO:
            return t == 0.0 ? 0.0 : pow(2.0, 10.0 * (t - 1.0));

        case TUI_EASE_OUT_EXPO:
            return t == 1.0 ? 1.0 : 1.0 - pow(2.0, -10.0 * t);

        case TUI_EASE_IN_OUT_EXPO:
            if (t == 0.0) return 0.0;
            if (t == 1.0) return 1.0;
            return t < 0.5
                ? pow(2.0, 20.0 * t - 10.0) / 2.0
                : (2.0 - pow(2.0, -20.0 * t + 10.0)) / 2.0;

        case TUI_EASE_IN_CIRC:
            return 1.0 - sqrt(1.0 - t * t);

        case TUI_EASE_OUT_CIRC:
            {
                double t1 = t - 1.0;
                return sqrt(1.0 - t1 * t1);
            }

        case TUI_EASE_IN_OUT_CIRC:
            if (t < 0.5) {
                return (1.0 - sqrt(1.0 - 4.0 * t * t)) / 2.0;
            } else {
                double t1 = -2.0 * t + 2.0;
                return (sqrt(1.0 - t1 * t1) + 1.0) / 2.0;
            }

        case TUI_EASE_IN_ELASTIC:
            if (t == 0.0) return 0.0;
            if (t == 1.0) return 1.0;
            return -pow(2.0, 10.0 * t - 10.0) * sin((t * 10.0 - 10.75) * (2.0 * M_PI / 3.0));

        case TUI_EASE_OUT_ELASTIC:
            if (t == 0.0) return 0.0;
            if (t == 1.0) return 1.0;
            return pow(2.0, -10.0 * t) * sin((t * 10.0 - 0.75) * (2.0 * M_PI / 3.0)) + 1.0;

        case TUI_EASE_IN_OUT_ELASTIC:
            if (t == 0.0) return 0.0;
            if (t == 1.0) return 1.0;
            if (t < 0.5) {
                return -(pow(2.0, 20.0 * t - 10.0) * sin((20.0 * t - 11.125) * (2.0 * M_PI / 4.5))) / 2.0;
            } else {
                return pow(2.0, -20.0 * t + 10.0) * sin((20.0 * t - 11.125) * (2.0 * M_PI / 4.5)) / 2.0 + 1.0;
            }

        case TUI_EASE_IN_BACK:
            {
                double c1 = 1.70158;
                double c3 = c1 + 1.0;
                return c3 * t * t * t - c1 * t * t;
            }

        case TUI_EASE_OUT_BACK:
            {
                double c1 = 1.70158;
                double c3 = c1 + 1.0;
                double t1 = t - 1.0;
                return 1.0 + c3 * t1 * t1 * t1 + c1 * t1 * t1;
            }

        case TUI_EASE_IN_OUT_BACK:
            {
                double c1 = 1.70158;
                double c2 = c1 * 1.525;
                if (t < 0.5) {
                    return (4.0 * t * t * ((c2 + 1.0) * 2.0 * t - c2)) / 2.0;
                } else {
                    double t1 = 2.0 * t - 2.0;
                    return (t1 * t1 * ((c2 + 1.0) * t1 + c2) + 2.0) / 2.0;
                }
            }

        case TUI_EASE_IN_BOUNCE:
            return ease_in_bounce(t);

        case TUI_EASE_OUT_BOUNCE:
            return ease_out_bounce(t);

        case TUI_EASE_IN_OUT_BOUNCE:
            return t < 0.5
                ? (1.0 - ease_out_bounce(1.0 - 2.0 * t)) / 2.0
                : (1.0 + ease_out_bounce(2.0 * t - 1.0)) / 2.0;

        default:
            return t;
    }
}

tui_easing tui_ease_from_name(const char *name)
{
    if (!name) return TUI_EASE_LINEAR;

    if (strcmp(name, "linear") == 0) return TUI_EASE_LINEAR;
    if (strcmp(name, "in-quad") == 0 || strcmp(name, "inQuad") == 0) return TUI_EASE_IN_QUAD;
    if (strcmp(name, "out-quad") == 0 || strcmp(name, "outQuad") == 0) return TUI_EASE_OUT_QUAD;
    if (strcmp(name, "in-out-quad") == 0 || strcmp(name, "inOutQuad") == 0) return TUI_EASE_IN_OUT_QUAD;
    if (strcmp(name, "in-cubic") == 0 || strcmp(name, "inCubic") == 0) return TUI_EASE_IN_CUBIC;
    if (strcmp(name, "out-cubic") == 0 || strcmp(name, "outCubic") == 0) return TUI_EASE_OUT_CUBIC;
    if (strcmp(name, "in-out-cubic") == 0 || strcmp(name, "inOutCubic") == 0) return TUI_EASE_IN_OUT_CUBIC;
    if (strcmp(name, "in-quart") == 0 || strcmp(name, "inQuart") == 0) return TUI_EASE_IN_QUART;
    if (strcmp(name, "out-quart") == 0 || strcmp(name, "outQuart") == 0) return TUI_EASE_OUT_QUART;
    if (strcmp(name, "in-out-quart") == 0 || strcmp(name, "inOutQuart") == 0) return TUI_EASE_IN_OUT_QUART;
    if (strcmp(name, "in-sine") == 0 || strcmp(name, "inSine") == 0) return TUI_EASE_IN_SINE;
    if (strcmp(name, "out-sine") == 0 || strcmp(name, "outSine") == 0) return TUI_EASE_OUT_SINE;
    if (strcmp(name, "in-out-sine") == 0 || strcmp(name, "inOutSine") == 0) return TUI_EASE_IN_OUT_SINE;
    if (strcmp(name, "in-expo") == 0 || strcmp(name, "inExpo") == 0) return TUI_EASE_IN_EXPO;
    if (strcmp(name, "out-expo") == 0 || strcmp(name, "outExpo") == 0) return TUI_EASE_OUT_EXPO;
    if (strcmp(name, "in-out-expo") == 0 || strcmp(name, "inOutExpo") == 0) return TUI_EASE_IN_OUT_EXPO;
    if (strcmp(name, "in-circ") == 0 || strcmp(name, "inCirc") == 0) return TUI_EASE_IN_CIRC;
    if (strcmp(name, "out-circ") == 0 || strcmp(name, "outCirc") == 0) return TUI_EASE_OUT_CIRC;
    if (strcmp(name, "in-out-circ") == 0 || strcmp(name, "inOutCirc") == 0) return TUI_EASE_IN_OUT_CIRC;
    if (strcmp(name, "in-elastic") == 0 || strcmp(name, "inElastic") == 0) return TUI_EASE_IN_ELASTIC;
    if (strcmp(name, "out-elastic") == 0 || strcmp(name, "outElastic") == 0) return TUI_EASE_OUT_ELASTIC;
    if (strcmp(name, "in-out-elastic") == 0 || strcmp(name, "inOutElastic") == 0) return TUI_EASE_IN_OUT_ELASTIC;
    if (strcmp(name, "in-back") == 0 || strcmp(name, "inBack") == 0) return TUI_EASE_IN_BACK;
    if (strcmp(name, "out-back") == 0 || strcmp(name, "outBack") == 0) return TUI_EASE_OUT_BACK;
    if (strcmp(name, "in-out-back") == 0 || strcmp(name, "inOutBack") == 0) return TUI_EASE_IN_OUT_BACK;
    if (strcmp(name, "in-bounce") == 0 || strcmp(name, "inBounce") == 0) return TUI_EASE_IN_BOUNCE;
    if (strcmp(name, "out-bounce") == 0 || strcmp(name, "outBounce") == 0) return TUI_EASE_OUT_BOUNCE;
    if (strcmp(name, "in-out-bounce") == 0 || strcmp(name, "inOutBounce") == 0) return TUI_EASE_IN_OUT_BOUNCE;

    return TUI_EASE_LINEAR;
}

double tui_lerp(double a, double b, double t)
{
    return a + (b - a) * t;
}

tui_color tui_lerp_color(tui_color a, tui_color b, double t)
{
    tui_color result;
    result.r = (uint8_t)(a.r + (b.r - a.r) * t);
    result.g = (uint8_t)(a.g + (b.g - a.g) * t);
    result.b = (uint8_t)(a.b + (b.b - a.b) * t);
    result.is_set = 1;
    return result;
}

void tui_gradient(tui_color *colors, int color_count, tui_color *output, int steps)
{
    if (!colors || !output || color_count < 1 || steps < 1) return;

    if (color_count == 1) {
        for (int i = 0; i < steps; i++) {
            output[i] = colors[0];
        }
        return;
    }

    for (int i = 0; i < steps; i++) {
        double t = (double)i / (steps - 1);
        double segment = t * (color_count - 1);
        int idx = (int)segment;
        if (idx >= color_count - 1) idx = color_count - 2;
        double local_t = segment - idx;
        output[i] = tui_lerp_color(colors[idx], colors[idx + 1], local_t);
    }
}

tui_color tui_color_from_hex(const char *hex)
{
    tui_color color = {0, 0, 0, 0};
    if (!hex) return color;

    /* Skip leading # if present */
    if (hex[0] == '#') hex++;

    unsigned int r, g, b;
    if (sscanf(hex, "%02x%02x%02x", &r, &g, &b) == 3) {
        color.r = (uint8_t)r;
        color.g = (uint8_t)g;
        color.b = (uint8_t)b;
        color.is_set = 1;
    }

    return color;
}

/**
 * Convert color to hex string (e.g., "#ff0000").
 *
 * @param color Color to convert
 * @param buf Output buffer (must be at least 8 bytes)
 * @param buf_size Size of output buffer
 */
void tui_color_to_hex(tui_color color, char *buf, size_t buf_size)
{
    if (!buf || buf_size < 8) return;
    snprintf(buf, buf_size, "#%02x%02x%02x", color.r, color.g, color.b);
}
