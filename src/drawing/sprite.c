/*
  +----------------------------------------------------------------------+
  | ext-tui: Sprite rendering and animation                             |
  +----------------------------------------------------------------------+
*/

#include "sprite.h"
#include "../text/measure.h"
#include <stdlib.h>
#include <string.h>

tui_sprite_frame tui_sprite_frame_create(const char **lines, int line_count,
                                          int duration)
{
    tui_sprite_frame frame = {0};

    if (!lines || line_count <= 0) return frame;

    frame.lines = calloc(line_count, sizeof(char*));
    if (!frame.lines) return frame;

    frame.height = line_count;
    frame.width = 0;
    frame.duration = duration > 0 ? duration : 100;

    for (int i = 0; i < line_count; i++) {
        if (lines[i]) {
            frame.lines[i] = strdup(lines[i]);
            int w = tui_string_width_n(lines[i], strlen(lines[i]));
            if (w > frame.width) frame.width = w;
        } else {
            frame.lines[i] = strdup("");
        }
    }

    return frame;
}

void tui_sprite_frame_free(tui_sprite_frame *frame)
{
    if (!frame) return;

    if (frame->lines) {
        for (int i = 0; i < frame->height; i++) {
            free(frame->lines[i]);
        }
        free(frame->lines);
        frame->lines = NULL;
    }
    frame->width = 0;
    frame->height = 0;
}

tui_sprite* tui_sprite_create(tui_sprite_frame *frames, int frame_count,
                              const char *name, int loop)
{
    tui_sprite *sprite = calloc(1, sizeof(tui_sprite));
    if (!sprite) return NULL;

    sprite->visible = 1;

    if (frames && frame_count > 0) {
        if (tui_sprite_add_animation(sprite, frames, frame_count, name, loop) != 0) {
            free(sprite);
            return NULL;
        }
    }

    return sprite;
}

void tui_sprite_free(tui_sprite *sprite)
{
    if (!sprite) return;

    for (int a = 0; a < sprite->animation_count; a++) {
        tui_sprite_animation *anim = &sprite->animations[a];
        free(anim->name);
        for (int f = 0; f < anim->frame_count; f++) {
            tui_sprite_frame_free(&anim->frames[f]);
        }
        free(anim->frames);
    }
    free(sprite->animations);
    free(sprite);
}

int tui_sprite_add_animation(tui_sprite *sprite, tui_sprite_frame *frames,
                             int frame_count, const char *name, int loop)
{
    if (!sprite || !frames || frame_count <= 0) return -1;

    /* Grow animations array */
    int new_count = sprite->animation_count + 1;
    tui_sprite_animation *new_anims = realloc(sprite->animations,
                                               new_count * sizeof(tui_sprite_animation));
    if (!new_anims) return -1;
    sprite->animations = new_anims;

    /* Initialize new animation */
    tui_sprite_animation *anim = &sprite->animations[sprite->animation_count];
    memset(anim, 0, sizeof(tui_sprite_animation));

    anim->name = name ? strdup(name) : strdup("default");
    anim->loop = loop;
    anim->frame_count = frame_count;
    anim->frames = calloc(frame_count, sizeof(tui_sprite_frame));

    if (!anim->frames) {
        free(anim->name);
        return -1;
    }

    /* Copy frames */
    for (int i = 0; i < frame_count; i++) {
        anim->frames[i] = frames[i];
        /* Deep copy lines */
        if (frames[i].lines) {
            anim->frames[i].lines = calloc(frames[i].height, sizeof(char*));
            for (int j = 0; j < frames[i].height; j++) {
                anim->frames[i].lines[j] = strdup(frames[i].lines[j]);
            }
        }
    }

    sprite->animation_count = new_count;
    return 0;
}

void tui_sprite_update(tui_sprite *sprite, int delta_ms)
{
    if (!sprite || sprite->animation_count == 0) return;

    tui_sprite_animation *anim = &sprite->animations[sprite->current_animation];
    if (anim->frame_count == 0) return;

    tui_sprite_frame *frame = &anim->frames[sprite->current_frame];

    sprite->frame_timer += delta_ms;

    /* Advance frame if duration elapsed */
    while (sprite->frame_timer >= frame->duration) {
        sprite->frame_timer -= frame->duration;
        sprite->current_frame++;

        if (sprite->current_frame >= anim->frame_count) {
            if (anim->loop) {
                sprite->current_frame = 0;
            } else {
                sprite->current_frame = anim->frame_count - 1;
                sprite->frame_timer = 0;
                break;
            }
        }

        frame = &anim->frames[sprite->current_frame];
    }
}

int tui_sprite_set_animation(tui_sprite *sprite, const char *name)
{
    if (!sprite || !name) return -1;

    for (int i = 0; i < sprite->animation_count; i++) {
        if (sprite->animations[i].name &&
            strcmp(sprite->animations[i].name, name) == 0) {
            if (sprite->current_animation != i) {
                sprite->current_animation = i;
                sprite->current_frame = 0;
                sprite->frame_timer = 0;
            }
            return 0;
        }
    }

    return -1;
}

void tui_sprite_set_position(tui_sprite *sprite, int x, int y)
{
    if (!sprite) return;
    sprite->x = x;
    sprite->y = y;
}

void tui_sprite_flip(tui_sprite *sprite, int flipped)
{
    if (!sprite) return;
    sprite->flipped = flipped;
}

void tui_sprite_set_visible(tui_sprite *sprite, int visible)
{
    if (!sprite) return;
    sprite->visible = visible;
}

/* Reverse a UTF-8 string in-place, accounting for multi-byte characters */
static void reverse_utf8_line(const char *input, char *output, int max_len)
{
    int len = strlen(input);
    if (len == 0) {
        output[0] = '\0';
        return;
    }

    /* Collect character positions */
    int char_starts[256];
    int char_count = 0;
    int pos = 0;

    while (pos < len && char_count < 256) {
        char_starts[char_count++] = pos;
        unsigned char c = (unsigned char)input[pos];
        if ((c & 0x80) == 0) pos += 1;
        else if ((c & 0xE0) == 0xC0) pos += 2;
        else if ((c & 0xF0) == 0xE0) pos += 3;
        else if ((c & 0xF8) == 0xF0) pos += 4;
        else pos += 1;
    }

    /* Build reversed string */
    int out_pos = 0;
    for (int i = char_count - 1; i >= 0 && out_pos < max_len - 4; i--) {
        int start = char_starts[i];
        int end = (i < char_count - 1) ? char_starts[i + 1] : len;
        int char_len = end - start;
        memcpy(output + out_pos, input + start, char_len);
        out_pos += char_len;
    }
    output[out_pos] = '\0';
}

void tui_sprite_render(tui_buffer *buf, tui_sprite *sprite)
{
    if (!buf || !sprite || !sprite->visible) return;
    if (sprite->animation_count == 0) return;

    tui_sprite_animation *anim = &sprite->animations[sprite->current_animation];
    if (anim->frame_count == 0) return;

    tui_sprite_frame *frame = &anim->frames[sprite->current_frame];
    if (!frame->lines) return;

    /* Determine style */
    tui_style style = {0};
    if (frame->color.is_set) {
        style.fg = frame->color;
    } else if (sprite->default_color.is_set) {
        style.fg = sprite->default_color;
    }

    /* Render each line */
    for (int i = 0; i < frame->height; i++) {
        const char *line = frame->lines[i];
        if (!line) continue;

        if (sprite->flipped) {
            /* Horizontally flip the line */
            char reversed[512];
            reverse_utf8_line(line, reversed, sizeof(reversed));
            tui_buffer_write_text(buf, sprite->x, sprite->y + i, reversed, &style);
        } else {
            tui_buffer_write_text(buf, sprite->x, sprite->y + i, line, &style);
        }
    }
}

void tui_sprite_get_bounds(tui_sprite *sprite, int *x, int *y,
                           int *width, int *height)
{
    if (!sprite) {
        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = 0;
        if (height) *height = 0;
        return;
    }

    if (x) *x = sprite->x;
    if (y) *y = sprite->y;

    /* Get dimensions from current frame */
    if (sprite->animation_count > 0) {
        tui_sprite_animation *anim = &sprite->animations[sprite->current_animation];
        if (anim->frame_count > 0) {
            tui_sprite_frame *frame = &anim->frames[sprite->current_frame];
            if (width) *width = frame->width;
            if (height) *height = frame->height;
            return;
        }
    }

    if (width) *width = 0;
    if (height) *height = 0;
}

int tui_sprite_collides(tui_sprite *a, tui_sprite *b)
{
    if (!a || !b) return 0;
    if (!a->visible || !b->visible) return 0;

    int ax, ay, aw, ah;
    int bx, by, bw, bh;

    tui_sprite_get_bounds(a, &ax, &ay, &aw, &ah);
    tui_sprite_get_bounds(b, &bx, &by, &bw, &bh);

    /* AABB collision */
    return (ax < bx + bw &&
            ax + aw > bx &&
            ay < by + bh &&
            ay + ah > by);
}
