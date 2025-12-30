/*
  +----------------------------------------------------------------------+
  | ext-tui: Screen recording and asciicast export                       |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "recorder.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

/* Get current time in seconds with microsecond precision */
static double get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
}

/* JSON-escape a string */
static char *json_escape_string(const char *str, size_t len)
{
    /* Worst case: every character needs escaping (\uXXXX = 6 chars) */
    size_t max_out = len * 6 + 1;
    char *out = malloc(max_out);
    if (!out) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)str[i];

        switch (c) {
            case '"':  out[j++] = '\\'; out[j++] = '"';  break;
            case '\\': out[j++] = '\\'; out[j++] = '\\'; break;
            case '\b': out[j++] = '\\'; out[j++] = 'b';  break;
            case '\f': out[j++] = '\\'; out[j++] = 'f';  break;
            case '\n': out[j++] = '\\'; out[j++] = 'n';  break;
            case '\r': out[j++] = '\\'; out[j++] = 'r';  break;
            case '\t': out[j++] = '\\'; out[j++] = 't';  break;
            default:
                if (c < 0x20) {
                    /* Control character - use \u00XX */
                    j += (size_t)snprintf(out + j, max_out - j, "\\u%04x", c);
                } else {
                    out[j++] = (char)c;
                }
                break;
        }
    }
    out[j] = '\0';

    return out;
}

tui_recording *tui_recording_create(int width, int height, const char *title)
{
    tui_recording *rec = calloc(1, sizeof(tui_recording));
    if (!rec) return NULL;

    rec->width = width > 0 ? width : 80;
    rec->height = height > 0 ? height : 24;

    if (title) {
        rec->title = strdup(title);
    }

    /* Start with room for 1024 frames */
    rec->frame_capacity = 1024;
    rec->frames = malloc((size_t)rec->frame_capacity * sizeof(tui_frame));
    if (!rec->frames) {
        free(rec->title);
        free(rec);
        return NULL;
    }

    rec->frame_count = 0;
    rec->state = TUI_RECORDING_STOPPED;
    rec->start_time = 0;
    rec->pause_time = 0;
    rec->paused_duration = 0;

    return rec;
}

void tui_recording_free(tui_recording *rec)
{
    if (!rec) return;

    /* Free all frame data */
    for (int i = 0; i < rec->frame_count; i++) {
        free(rec->frames[i].data);
    }
    free(rec->frames);
    free(rec->title);
    free(rec);
}

int tui_recording_start(tui_recording *rec)
{
    if (!rec) return -1;

    if (rec->state == TUI_RECORDING_ACTIVE) {
        return 0;  /* Already recording */
    }

    if (rec->state == TUI_RECORDING_PAUSED) {
        return tui_recording_resume(rec);
    }

    /* Fresh start */
    rec->start_time = get_time();
    rec->paused_duration = 0;
    rec->state = TUI_RECORDING_ACTIVE;

    return 0;
}

int tui_recording_pause(tui_recording *rec)
{
    if (!rec || rec->state != TUI_RECORDING_ACTIVE) {
        return -1;
    }

    rec->pause_time = get_time();
    rec->state = TUI_RECORDING_PAUSED;

    return 0;
}

int tui_recording_resume(tui_recording *rec)
{
    if (!rec || rec->state != TUI_RECORDING_PAUSED) {
        return -1;
    }

    /* Add time spent paused to paused_duration */
    double now = get_time();
    rec->paused_duration += (now - rec->pause_time);
    rec->state = TUI_RECORDING_ACTIVE;

    return 0;
}

int tui_recording_stop(tui_recording *rec)
{
    if (!rec) return -1;

    if (rec->state == TUI_RECORDING_PAUSED) {
        /* If stopped while paused, add final pause time */
        double now = get_time();
        rec->paused_duration += (now - rec->pause_time);
    }

    rec->state = TUI_RECORDING_STOPPED;
    return 0;
}

double tui_recording_current_time(const tui_recording *rec)
{
    if (!rec || rec->state == TUI_RECORDING_STOPPED) {
        return 0;
    }

    double now = get_time();
    double elapsed = now - rec->start_time - rec->paused_duration;

    if (rec->state == TUI_RECORDING_PAUSED) {
        /* Don't count time since pause */
        elapsed -= (now - rec->pause_time);
    }

    return elapsed > 0 ? elapsed : 0;
}

double tui_recording_duration(const tui_recording *rec)
{
    if (!rec || rec->frame_count == 0) {
        return 0;
    }

    /* Return timestamp of last frame */
    return rec->frames[rec->frame_count - 1].timestamp;
}

int tui_recording_capture_frame(tui_recording *rec, const char *data, size_t len)
{
    if (!rec || !data || rec->state != TUI_RECORDING_ACTIVE) {
        return -1;
    }

    /* Check frame limit */
    if (rec->frame_count >= TUI_MAX_RECORDING_FRAMES) {
        return -1;
    }

    /* Grow frames array if needed */
    if (rec->frame_count >= rec->frame_capacity) {
        int new_capacity = rec->frame_capacity * 2;
        if (new_capacity > TUI_MAX_RECORDING_FRAMES) {
            new_capacity = TUI_MAX_RECORDING_FRAMES;
        }

        tui_frame *new_frames = realloc(rec->frames,
            (size_t)new_capacity * sizeof(tui_frame));
        if (!new_frames) return -1;

        rec->frames = new_frames;
        rec->frame_capacity = new_capacity;
    }

    /* Allocate and copy frame data */
    char *frame_data = malloc(len + 1);
    if (!frame_data) return -1;
    memcpy(frame_data, data, len);
    frame_data[len] = '\0';

    /* Add frame */
    tui_frame *frame = &rec->frames[rec->frame_count];
    frame->timestamp = tui_recording_current_time(rec);
    frame->data = frame_data;
    frame->data_len = len;

    rec->frame_count++;
    return 0;
}

int tui_recording_to_asciicast(const tui_recording *rec, char **out, size_t *len)
{
    if (!rec || !out || !len) return -1;

    /* Estimate output size */
    size_t est_size = 1024;  /* Header */
    for (int i = 0; i < rec->frame_count; i++) {
        /* Each frame: [timestamp, "o", "data"]\n */
        est_size += rec->frames[i].data_len * 6 + 64;
    }

    char *buf = malloc(est_size);
    if (!buf) return -1;

    size_t pos = 0;

    /* Write header (asciicast v2 format) */
    /* Get current time for timestamp */
    time_t now = time(NULL);

    char *escaped_title = rec->title ?
        json_escape_string(rec->title, strlen(rec->title)) :
        strdup("Terminal Recording");

    pos += (size_t)snprintf(buf + pos, est_size - pos,
        "{\"version\": 2, \"width\": %d, \"height\": %d, "
        "\"timestamp\": %ld, \"title\": \"%s\"}\n",
        rec->width, rec->height, (long)now,
        escaped_title ? escaped_title : "Terminal Recording");

    free(escaped_title);

    /* Write frames */
    for (int i = 0; i < rec->frame_count; i++) {
        tui_frame *frame = &rec->frames[i];

        char *escaped_data = json_escape_string(frame->data, frame->data_len);
        if (!escaped_data) {
            free(buf);
            return -1;
        }

        /* Check if we need more space */
        size_t frame_len = strlen(escaped_data) + 64;
        if (pos + frame_len >= est_size) {
            est_size = est_size * 2 + frame_len;
            char *new_buf = realloc(buf, est_size);
            if (!new_buf) {
                free(escaped_data);
                free(buf);
                return -1;
            }
            buf = new_buf;
        }

        pos += (size_t)snprintf(buf + pos, est_size - pos,
            "[%.6f, \"o\", \"%s\"]\n",
            frame->timestamp, escaped_data);

        free(escaped_data);
    }

    *out = buf;
    *len = pos;

    return 0;
}

int tui_recording_save(const tui_recording *rec, const char *path)
{
    if (!rec || !path) return -1;

    char *data;
    size_t len;

    if (tui_recording_to_asciicast(rec, &data, &len) < 0) {
        return -1;
    }

    FILE *f = fopen(path, "w");
    if (!f) {
        free(data);
        return -1;
    }

    size_t written = fwrite(data, 1, len, f);
    fclose(f);
    free(data);

    return (written == len) ? 0 : -1;
}
