/*
  +----------------------------------------------------------------------+
  | ext-tui: Smooth scrolling implementation                            |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "smooth.h"
#include <math.h>

/* Default spring parameters for a snappy, responsive feel */
#define DEFAULT_STIFFNESS 200.0f
#define DEFAULT_DAMPING 1.0f      /* Critically damped */
#define DEFAULT_SNAP_THRESHOLD 0.01f

void tui_scroll_anim_init(tui_scroll_animation *anim)
{
    if (!anim) return;

    anim->target_x = 0.0f;
    anim->target_y = 0.0f;
    anim->current_x = 0.0f;
    anim->current_y = 0.0f;
    anim->velocity_x = 0.0f;
    anim->velocity_y = 0.0f;
    anim->stiffness = DEFAULT_STIFFNESS;
    anim->damping = DEFAULT_DAMPING;
    anim->animating = 0;
    anim->snap_threshold = DEFAULT_SNAP_THRESHOLD;
}

void tui_scroll_anim_set_spring(tui_scroll_animation *anim,
                                 float stiffness, float damping)
{
    if (!anim) return;

    anim->stiffness = stiffness > 0.0f ? stiffness : DEFAULT_STIFFNESS;
    anim->damping = damping > 0.0f ? damping : DEFAULT_DAMPING;
}

void tui_scroll_anim_set_target(tui_scroll_animation *anim,
                                 float target_x, float target_y)
{
    if (!anim) return;

    anim->target_x = target_x;
    anim->target_y = target_y;

    /* Start animating if not at target */
    float dx = anim->target_x - anim->current_x;
    float dy = anim->target_y - anim->current_y;
    float dist = sqrtf(dx * dx + dy * dy);

    if (dist > anim->snap_threshold) {
        anim->animating = 1;
    }
}

void tui_scroll_anim_scroll_by(tui_scroll_animation *anim,
                                float delta_x, float delta_y)
{
    if (!anim) return;

    tui_scroll_anim_set_target(anim,
                                anim->target_x + delta_x,
                                anim->target_y + delta_y);
}

/**
 * Spring physics update using semi-implicit Euler integration.
 *
 * For a critically damped spring:
 *   acceleration = -stiffness * displacement - damping * velocity
 *
 * We use damping_coefficient = 2 * sqrt(stiffness) for critical damping
 * when damping = 1.0. For other values, we scale proportionally.
 */
int tui_scroll_anim_update(tui_scroll_animation *anim, float dt)
{
    if (!anim || !anim->animating) return 0;

    /* Calculate critical damping coefficient */
    float omega = sqrtf(anim->stiffness);
    float damping_coeff = 2.0f * omega * anim->damping;

    /* Spring force for X */
    float dx = anim->target_x - anim->current_x;
    float spring_force_x = anim->stiffness * dx;
    float damping_force_x = damping_coeff * anim->velocity_x;
    float accel_x = spring_force_x - damping_force_x;

    /* Spring force for Y */
    float dy = anim->target_y - anim->current_y;
    float spring_force_y = anim->stiffness * dy;
    float damping_force_y = damping_coeff * anim->velocity_y;
    float accel_y = spring_force_y - damping_force_y;

    /* Semi-implicit Euler: update velocity first, then position */
    anim->velocity_x += accel_x * dt;
    anim->velocity_y += accel_y * dt;

    anim->current_x += anim->velocity_x * dt;
    anim->current_y += anim->velocity_y * dt;

    /* Check if we've reached the target */
    float dist = sqrtf(dx * dx + dy * dy);
    float speed = sqrtf(anim->velocity_x * anim->velocity_x +
                        anim->velocity_y * anim->velocity_y);

    if (dist < anim->snap_threshold && speed < anim->snap_threshold) {
        /* Snap to target and stop */
        anim->current_x = anim->target_x;
        anim->current_y = anim->target_y;
        anim->velocity_x = 0.0f;
        anim->velocity_y = 0.0f;
        anim->animating = 0;
        return 0;
    }

    return 1;
}

void tui_scroll_anim_snap(tui_scroll_animation *anim)
{
    if (!anim) return;

    anim->current_x = anim->target_x;
    anim->current_y = anim->target_y;
    anim->velocity_x = 0.0f;
    anim->velocity_y = 0.0f;
    anim->animating = 0;
}

void tui_scroll_anim_get_position(const tui_scroll_animation *anim,
                                   float *x, float *y)
{
    if (!anim) {
        if (x) *x = 0.0f;
        if (y) *y = 0.0f;
        return;
    }

    if (x) *x = anim->current_x;
    if (y) *y = anim->current_y;
}

int tui_scroll_anim_is_animating(const tui_scroll_animation *anim)
{
    return anim ? anim->animating : 0;
}

float tui_scroll_anim_progress(const tui_scroll_animation *anim)
{
    if (!anim || !anim->animating) return 1.0f;

    /* Calculate distance from start to target */
    float dx = anim->target_x - anim->current_x;
    float dy = anim->target_y - anim->current_y;
    float remaining = sqrtf(dx * dx + dy * dy);

    /* Estimate total distance (current to target is remaining) */
    /* This is approximate since we don't track start position */
    if (remaining < anim->snap_threshold) {
        return 1.0f;
    }

    /* Use velocity to estimate how far along we are */
    float speed = sqrtf(anim->velocity_x * anim->velocity_x +
                        anim->velocity_y * anim->velocity_y);

    /* Rough estimate: higher speed = earlier in animation */
    /* This gives a reasonable approximation for progress */
    float max_speed = sqrtf(anim->stiffness) * remaining;
    if (max_speed < 0.01f) return 1.0f;

    float progress = 1.0f - (speed / max_speed);
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    return progress;
}
