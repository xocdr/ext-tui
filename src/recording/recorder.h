/*
  +----------------------------------------------------------------------+
  | ext-tui: Screen recording and asciicast export                       |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
  | Supports:                                                             |
  | - Frame-by-frame capture of terminal output                          |
  | - asciicast v2 format export (for asciinema playback)                |
  | - Pause/resume recording                                              |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_RECORDER_H
#define TUI_RECORDER_H

#include <stddef.h>
#include <stdint.h>

/* Maximum frames in a recording */
#define TUI_MAX_RECORDING_FRAMES 65536

/* Recording states */
typedef enum {
    TUI_RECORDING_STOPPED = 0,
    TUI_RECORDING_ACTIVE,
    TUI_RECORDING_PAUSED
} tui_recording_state;

/**
 * A single frame in a recording.
 */
typedef struct {
    double timestamp;      /* Time since recording start (seconds) */
    char *data;            /* Terminal output data */
    size_t data_len;       /* Length of data */
} tui_frame;

/**
 * Screen recording context.
 */
typedef struct {
    int width;                  /* Terminal width */
    int height;                 /* Terminal height */
    char *title;                /* Optional title */
    tui_frame *frames;          /* Array of frames */
    int frame_count;            /* Number of frames */
    int frame_capacity;         /* Allocated capacity */
    double start_time;          /* Recording start time */
    double pause_time;          /* Time when paused (for duration tracking) */
    double paused_duration;     /* Total time spent paused */
    tui_recording_state state;  /* Current state */
} tui_recording;

/**
 * Create a new recording context.
 *
 * @param width   Terminal width
 * @param height  Terminal height
 * @param title   Optional title (can be NULL)
 * @return        New recording or NULL on error
 */
tui_recording *tui_recording_create(int width, int height, const char *title);

/**
 * Free a recording context.
 */
void tui_recording_free(tui_recording *rec);

/**
 * Start or resume recording.
 *
 * @return 0 on success, -1 on error
 */
int tui_recording_start(tui_recording *rec);

/**
 * Pause recording.
 *
 * @return 0 on success, -1 on error
 */
int tui_recording_pause(tui_recording *rec);

/**
 * Resume paused recording.
 *
 * @return 0 on success, -1 on error
 */
int tui_recording_resume(tui_recording *rec);

/**
 * Stop recording.
 *
 * @return 0 on success, -1 on error
 */
int tui_recording_stop(tui_recording *rec);

/**
 * Capture a frame to the recording.
 *
 * @param rec   Recording context
 * @param data  Terminal output data
 * @param len   Length of data
 * @return      0 on success, -1 on error
 */
int tui_recording_capture_frame(tui_recording *rec, const char *data, size_t len);

/**
 * Get recording duration in seconds.
 */
double tui_recording_duration(const tui_recording *rec);

/**
 * Get current timestamp for a recording.
 */
double tui_recording_current_time(const tui_recording *rec);

/**
 * Export recording to asciicast v2 format.
 *
 * @param rec   Recording to export
 * @param out   Output buffer (allocated by function, caller must free)
 * @param len   Output length
 * @return      0 on success, -1 on error
 */
int tui_recording_to_asciicast(const tui_recording *rec, char **out, size_t *len);

/**
 * Save recording to asciicast file.
 *
 * @param rec   Recording to save
 * @param path  Output file path
 * @return      0 on success, -1 on error
 */
int tui_recording_save(const tui_recording *rec, const char *path);

#endif /* TUI_RECORDER_H */
