/*
  +----------------------------------------------------------------------+
  | ext-tui: Sprite rendering and animation                             |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_SPRITE_H
#define TUI_SPRITE_H

#include "../render/buffer.h"
#include "../node/node.h"

/**
 * Sprite frame data.
 */
typedef struct {
    char **lines;           /* Array of UTF-8 strings */
    int width;              /* Max width in chars */
    int height;             /* Number of lines */
    tui_color color;        /* Frame color */
    int duration;           /* Hold time in milliseconds */
} tui_sprite_frame;

/**
 * Sprite animation.
 */
typedef struct {
    char *name;                     /* Animation name */
    tui_sprite_frame *frames;       /* Array of frames */
    int frame_count;                /* Number of frames */
    int loop;                       /* Whether to loop */
} tui_sprite_animation;

/**
 * Sprite instance.
 */
typedef struct {
    tui_sprite_animation *animations;   /* Array of animations */
    int animation_count;                 /* Number of animations */
    int current_animation;               /* Current animation index */
    int current_frame;                   /* Current frame in animation */
    int frame_timer;                     /* Milliseconds since frame start */
    int x, y;                            /* Position */
    int visible;                         /* Whether to render */
    int flipped;                         /* Horizontal flip */
    tui_color default_color;             /* Default color if frame has none */
} tui_sprite;

/**
 * Create a sprite with one animation.
 *
 * @param frames Array of frame data
 * @param frame_count Number of frames
 * @param name Animation name
 * @param loop Whether to loop
 * @return New sprite, or NULL on error
 */
tui_sprite* tui_sprite_create(tui_sprite_frame *frames, int frame_count,
                              const char *name, int loop);

/**
 * Free a sprite.
 */
void tui_sprite_free(tui_sprite *sprite);

/**
 * Add an animation to a sprite.
 *
 * @param sprite Sprite to modify
 * @param frames Array of frame data
 * @param frame_count Number of frames
 * @param name Animation name
 * @param loop Whether to loop
 * @return 0 on success, -1 on error
 */
int tui_sprite_add_animation(tui_sprite *sprite, tui_sprite_frame *frames,
                             int frame_count, const char *name, int loop);

/**
 * Create a sprite frame from lines of text.
 *
 * @param lines Array of strings (one per line)
 * @param line_count Number of lines
 * @param duration Frame duration in milliseconds
 * @return New frame structure
 */
tui_sprite_frame tui_sprite_frame_create(const char **lines, int line_count,
                                          int duration);

/**
 * Free a sprite frame's internal data.
 */
void tui_sprite_frame_free(tui_sprite_frame *frame);

/**
 * Update sprite animation state.
 *
 * @param sprite Sprite to update
 * @param delta_ms Milliseconds since last update
 */
void tui_sprite_update(tui_sprite *sprite, int delta_ms);

/**
 * Set current animation by name.
 *
 * @param sprite Sprite to modify
 * @param name Animation name
 * @return 0 on success, -1 if animation not found
 */
int tui_sprite_set_animation(tui_sprite *sprite, const char *name);

/**
 * Set sprite position.
 */
void tui_sprite_set_position(tui_sprite *sprite, int x, int y);

/**
 * Set horizontal flip.
 */
void tui_sprite_flip(tui_sprite *sprite, int flipped);

/**
 * Set sprite visibility.
 */
void tui_sprite_set_visible(tui_sprite *sprite, int visible);

/**
 * Render sprite to buffer.
 *
 * @param buf Buffer to render to
 * @param sprite Sprite to render
 */
void tui_sprite_render(tui_buffer *buf, tui_sprite *sprite);

/**
 * Get sprite bounding box.
 *
 * @param sprite Sprite to query
 * @param x Output: X position
 * @param y Output: Y position
 * @param width Output: Width
 * @param height Output: Height
 */
void tui_sprite_get_bounds(tui_sprite *sprite, int *x, int *y,
                           int *width, int *height);

/**
 * Check collision between two sprites (AABB).
 *
 * @param a First sprite
 * @param b Second sprite
 * @return 1 if colliding, 0 otherwise
 */
int tui_sprite_collides(tui_sprite *a, tui_sprite *b);

#endif /* TUI_SPRITE_H */
