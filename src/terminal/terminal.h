/*
  +----------------------------------------------------------------------+
  | ext-tui: Terminal handling                                          |
  +----------------------------------------------------------------------+
  | Manages terminal state including raw mode, mouse tracking, and      |
  | bracketed paste mode.                                               |
  |                                                                      |
  | Thread Safety: All functions in this module are MAIN-THREAD-ONLY.   |
  | Terminal state (termios, mouse mode) is process-global. Only call   |
  | these functions from the thread that owns the terminal.             |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_TERMINAL_H
#define TUI_TERMINAL_H

#include <termios.h>

/* ================================================================
 * Raw mode (for unbuffered input)
 * ================================================================ */

/**
 * Enable terminal raw mode.
 * Disables canonical mode, echo, and signal processing.
 * Must be paired with tui_terminal_disable_raw_mode().
 * @return 0 on success, -1 on failure
 */
int tui_terminal_enable_raw_mode(void);

/**
 * Disable terminal raw mode.
 * Restores original terminal settings.
 * @return 0 on success, -1 on failure
 */
int tui_terminal_disable_raw_mode(void);

/**
 * Check if terminal is in raw mode.
 * @return 1 if raw mode is enabled, 0 otherwise
 */
int tui_terminal_is_raw_mode(void);

/* ================================================================
 * Terminal information
 * ================================================================ */

/**
 * Get terminal dimensions.
 * @param width  Output: terminal width in columns
 * @param height Output: terminal height in rows
 * @return 0 on success, -1 on failure
 */
int tui_terminal_get_size(int *width, int *height);

/**
 * Check if stdin is a terminal.
 * @return 1 if stdin is a TTY, 0 otherwise
 */
int tui_terminal_is_tty(void);

/* ================================================================
 * Bracketed paste mode
 * ================================================================ */

/**
 * Enable bracketed paste mode.
 * Pasted text will be wrapped in escape sequences.
 * @return 0 on success, -1 on failure
 */
int tui_terminal_enable_bracketed_paste(void);

/**
 * Disable bracketed paste mode.
 * @return 0 on success, -1 on failure
 */
int tui_terminal_disable_bracketed_paste(void);

/**
 * Check if bracketed paste mode is enabled.
 * @return 1 if enabled, 0 otherwise
 */
int tui_terminal_is_bracketed_paste_enabled(void);

/* ================================================================
 * Mouse tracking
 * ================================================================ */

#include "ansi.h"  /* For tui_mouse_mode */

/**
 * Enable mouse tracking with specified mode.
 * @param mode Mouse tracking mode (see ansi.h)
 * @return 0 on success, -1 on failure
 */
int tui_terminal_enable_mouse(tui_mouse_mode mode);

/**
 * Disable mouse tracking.
 * @return 0 on success, -1 on failure
 */
int tui_terminal_disable_mouse(void);

/**
 * Get current mouse tracking mode.
 * @return Current mouse mode
 */
tui_mouse_mode tui_terminal_get_mouse_mode(void);

/* ================================================================
 * Internal state (for save/restore)
 * ================================================================ */

/** Original terminal settings (saved before raw mode) */
extern struct termios tui_original_termios;

/* ================================================================
 * Emergency restoration (for crash recovery)
 * ================================================================ */

/**
 * Emergency terminal restore for atexit() handler.
 * Restores terminal to cooked mode if in raw mode.
 * Safe to call multiple times or from signal handlers.
 */
void tui_terminal_emergency_restore(void);

/**
 * Register the emergency restore handler with atexit().
 * Should be called once during module initialization.
 */
void tui_terminal_register_emergency_handler(void);

#endif /* TUI_TERMINAL_H */
