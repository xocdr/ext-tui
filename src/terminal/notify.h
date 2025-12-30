/*
  +----------------------------------------------------------------------+
  | ext-tui: Terminal notifications (bell, flash, desktop)              |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_NOTIFY_H
#define TUI_NOTIFY_H

#include <stddef.h>

/* Notification priority levels */
typedef enum {
    TUI_NOTIFY_NORMAL = 0,    /* Standard notification */
    TUI_NOTIFY_URGENT = 1     /* Urgent/critical notification */
} tui_notify_priority;

/**
 * Emit terminal bell (BEL character).
 * Produces audible bell on most terminals.
 */
void tui_bell(void);

/**
 * Emit visual bell (screen flash).
 * Uses reverse video mode briefly to flash the screen.
 */
void tui_flash(void);

/**
 * Send desktop notification via OSC escape sequence.
 * Uses terminal-specific protocol:
 * - Kitty: OSC 99
 * - iTerm2: OSC 9
 * - VTE (GNOME, Tilix): OSC 777
 *
 * @param title    Notification title
 * @param body     Notification body (can be NULL)
 * @param priority TUI_NOTIFY_NORMAL or TUI_NOTIFY_URGENT
 * @return 0 on success, -1 if notifications not supported
 */
int tui_notify(const char *title, const char *body, tui_notify_priority priority);

/**
 * Build bell sequence.
 * @param buf Output buffer
 * @param len Output length
 */
void tui_ansi_bell(char *buf, size_t *len);

/**
 * Build visual flash start sequence (reverse video on).
 * @param buf Output buffer
 * @param len Output length
 */
void tui_ansi_flash_start(char *buf, size_t *len);

/**
 * Build visual flash end sequence (reverse video off).
 * @param buf Output buffer
 * @param len Output length
 */
void tui_ansi_flash_end(char *buf, size_t *len);

/**
 * Build OSC 9 notification (iTerm2 style).
 * Format: ESC ] 9 ; message BEL
 *
 * @param buf       Output buffer
 * @param buf_size  Buffer size
 * @param len       Output length
 * @param message   Notification message
 */
void tui_ansi_notify_osc9(char *buf, size_t buf_size, size_t *len, const char *message);

/**
 * Build OSC 99 notification (Kitty style).
 * Format: ESC ] 99 ; i=1:d=0:p=body ; title BEL
 *
 * @param buf       Output buffer
 * @param buf_size  Buffer size
 * @param len       Output length
 * @param title     Notification title
 * @param body      Notification body (can be NULL)
 */
void tui_ansi_notify_osc99(char *buf, size_t buf_size, size_t *len,
                           const char *title, const char *body);

/**
 * Build OSC 777 notification (VTE/libnotify style).
 * Format: ESC ] 777 ; notify ; title ; body BEL
 *
 * @param buf       Output buffer
 * @param buf_size  Buffer size
 * @param len       Output length
 * @param title     Notification title
 * @param body      Notification body (can be NULL)
 */
void tui_ansi_notify_osc777(char *buf, size_t buf_size, size_t *len,
                            const char *title, const char *body);

#endif /* TUI_NOTIFY_H */
