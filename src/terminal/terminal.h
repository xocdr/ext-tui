/*
  +----------------------------------------------------------------------+
  | ext-tui: Terminal handling                                          |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_TERMINAL_H
#define TUI_TERMINAL_H

#include <termios.h>

/* Terminal mode management */
int tui_terminal_enable_raw_mode(void);
int tui_terminal_disable_raw_mode(void);
int tui_terminal_is_raw_mode(void);

/* Terminal info */
int tui_terminal_get_size(int *width, int *height);
int tui_terminal_is_tty(void);

/* Original termios storage */
extern struct termios tui_original_termios;

#endif /* TUI_TERMINAL_H */
