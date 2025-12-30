/*
  +----------------------------------------------------------------------+
  | ext-tui: Smooth scrolling with spring physics                       |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_SMOOTH_SCROLL_H
#define TUI_SMOOTH_SCROLL_H

/**
 * Scroll animation state using spring physics.
 *
 * Uses a critically-damped spring for smooth, natural-feeling scrolling.
 * The animation converges to target without oscillation.
 */
typedef struct {
    /* Target values */
    float target_x;
    float target_y;

    /* Current values */
    float current_x;
    float current_y;

    /* Velocity (for spring physics) */
    float velocity_x;
    float velocity_y;

    /* Spring parameters */
    float stiffness;     /* Spring constant (higher = faster) */
    float damping;       /* Damping ratio (1.0 = critically damped) */

    /* State */
    int animating;       /* 1 if animation is in progress */

    /* Snap threshold (stop animating when close enough) */
    float snap_threshold;
} tui_scroll_animation;

/**
 * Initialize scroll animation with default spring parameters.
 */
void tui_scroll_anim_init(tui_scroll_animation *anim);

/**
 * Set spring parameters.
 * @param stiffness  Spring constant (default: 200.0, range: 50-500)
 * @param damping    Damping ratio (default: 1.0 for critical damping)
 *                   < 1.0 = underdamped (oscillates)
 *                   = 1.0 = critically damped (fastest without oscillation)
 *                   > 1.0 = overdamped (slower convergence)
 */
void tui_scroll_anim_set_spring(tui_scroll_animation *anim,
                                 float stiffness, float damping);

/**
 * Set scroll target position.
 * Animation will begin if not already at target.
 */
void tui_scroll_anim_set_target(tui_scroll_animation *anim,
                                 float target_x, float target_y);

/**
 * Add to current scroll target (for incremental scrolling).
 */
void tui_scroll_anim_scroll_by(tui_scroll_animation *anim,
                                float delta_x, float delta_y);

/**
 * Update animation for one frame.
 * @param dt  Time delta in seconds (e.g., 1.0/60.0 for 60fps)
 * @return 1 if animation is still in progress, 0 if complete
 */
int tui_scroll_anim_update(tui_scroll_animation *anim, float dt);

/**
 * Immediately snap to target (cancel animation).
 */
void tui_scroll_anim_snap(tui_scroll_animation *anim);

/**
 * Get current scroll position.
 */
void tui_scroll_anim_get_position(const tui_scroll_animation *anim,
                                   float *x, float *y);

/**
 * Check if animation is in progress.
 */
int tui_scroll_anim_is_animating(const tui_scroll_animation *anim);

/**
 * Get animation progress (0.0 to 1.0).
 * Returns 1.0 if not animating.
 */
float tui_scroll_anim_progress(const tui_scroll_animation *anim);

#endif /* TUI_SMOOTH_SCROLL_H */
