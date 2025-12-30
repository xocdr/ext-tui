/*
  +----------------------------------------------------------------------+
  | ext-tui: Accessibility support                                       |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
  | Provides:                                                             |
  | - Screen reader announcements via OSC escape sequences               |
  | - ARIA properties for nodes                                          |
  | - Motion preference detection                                         |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_ACCESSIBILITY_H
#define TUI_ACCESSIBILITY_H

#include <stddef.h>

/* Announcement priority levels */
typedef enum {
    TUI_ANNOUNCE_POLITE = 0,   /* Wait for idle */
    TUI_ANNOUNCE_ASSERTIVE     /* Interrupt immediately */
} tui_announce_priority;

/* Common ARIA roles */
typedef enum {
    TUI_ARIA_ROLE_NONE = 0,
    TUI_ARIA_ROLE_BUTTON,
    TUI_ARIA_ROLE_CHECKBOX,
    TUI_ARIA_ROLE_DIALOG,
    TUI_ARIA_ROLE_GRID,
    TUI_ARIA_ROLE_GRIDCELL,
    TUI_ARIA_ROLE_GROUP,
    TUI_ARIA_ROLE_HEADING,
    TUI_ARIA_ROLE_IMG,
    TUI_ARIA_ROLE_LINK,
    TUI_ARIA_ROLE_LIST,
    TUI_ARIA_ROLE_LISTITEM,
    TUI_ARIA_ROLE_MENU,
    TUI_ARIA_ROLE_MENUITEM,
    TUI_ARIA_ROLE_NAVIGATION,
    TUI_ARIA_ROLE_PROGRESSBAR,
    TUI_ARIA_ROLE_RADIO,
    TUI_ARIA_ROLE_REGION,
    TUI_ARIA_ROLE_SCROLLBAR,
    TUI_ARIA_ROLE_SEARCH,
    TUI_ARIA_ROLE_SLIDER,
    TUI_ARIA_ROLE_SPINBUTTON,
    TUI_ARIA_ROLE_STATUS,
    TUI_ARIA_ROLE_TAB,
    TUI_ARIA_ROLE_TABLIST,
    TUI_ARIA_ROLE_TABPANEL,
    TUI_ARIA_ROLE_TEXTBOX,
    TUI_ARIA_ROLE_TREE,
    TUI_ARIA_ROLE_TREEITEM,
    TUI_ARIA_ROLE_ALERT,
    TUI_ARIA_ROLE_ALERTDIALOG,
    TUI_ARIA_ROLE_TOOLTIP,
    TUI_ARIA_ROLE_CUSTOM
} tui_aria_role;

/* Tristate for boolean ARIA properties that can be unset */
typedef enum {
    TUI_ARIA_UNSET = -1,
    TUI_ARIA_FALSE = 0,
    TUI_ARIA_TRUE = 1
} tui_aria_tristate;

/* ARIA properties container */
typedef struct {
    char *label;           /* aria-label */
    char *description;     /* aria-description */
    tui_aria_role role;    /* role attribute */
    char *custom_role;     /* For ARIA_ROLE_CUSTOM */
    tui_aria_tristate hidden;    /* aria-hidden */
    tui_aria_tristate expanded;  /* aria-expanded */
    tui_aria_tristate checked;   /* aria-checked */
    tui_aria_tristate disabled;  /* aria-disabled */
    tui_aria_tristate selected;  /* aria-selected */
    int level;             /* aria-level (for headings, 0 = unset) */
    int value_now;         /* aria-valuenow */
    int value_min;         /* aria-valuemin */
    int value_max;         /* aria-valuemax */
    char *live;            /* aria-live: off, polite, assertive */
} tui_aria_props;

/**
 * Initialize ARIA properties to defaults.
 */
void tui_aria_props_init(tui_aria_props *props);

/**
 * Free ARIA properties (strings only).
 */
void tui_aria_props_free(tui_aria_props *props);

/**
 * Set ARIA label.
 * @return 0 on success, -1 on error
 */
int tui_aria_set_label(tui_aria_props *props, const char *label);

/**
 * Set ARIA description.
 * @return 0 on success, -1 on error
 */
int tui_aria_set_description(tui_aria_props *props, const char *description);

/**
 * Set ARIA role.
 * @param role Role enum or TUI_ARIA_ROLE_CUSTOM
 * @param custom_role Custom role string (only used if role is CUSTOM)
 * @return 0 on success, -1 on error
 */
int tui_aria_set_role(tui_aria_props *props, tui_aria_role role, const char *custom_role);

/**
 * Get role name as string.
 */
const char *tui_aria_role_to_string(tui_aria_role role);

/**
 * Parse role name string to enum.
 * @return Role enum or TUI_ARIA_ROLE_NONE if not recognized
 */
tui_aria_role tui_aria_role_from_string(const char *role_str);

/**
 * Send announcement to screen reader.
 * Uses OSC sequences (may not be widely supported).
 *
 * @param message Text to announce
 * @param priority POLITE or ASSERTIVE
 * @return 0 on success, -1 on error
 */
int tui_announce(const char *message, tui_announce_priority priority);

/**
 * Check if user prefers reduced motion.
 * Checks environment variable: REDUCE_MOTION=1, NO_MOTION=1, or
 * prefers-reduced-motion in accessibility settings.
 *
 * @return 1 if reduced motion preferred, 0 otherwise
 */
int tui_prefers_reduced_motion(void);

/**
 * Check if user prefers high contrast.
 * Checks environment variables: HIGH_CONTRAST=1, COLORTERM=none
 *
 * @return 1 if high contrast preferred, 0 otherwise
 */
int tui_prefers_high_contrast(void);

/**
 * Get system accessibility features.
 * Returns bitmask of detected features.
 */
int tui_get_accessibility_features(void);

/* Accessibility feature flags */
#define TUI_A11Y_REDUCED_MOTION  0x01
#define TUI_A11Y_HIGH_CONTRAST   0x02
#define TUI_A11Y_SCREEN_READER   0x04

#endif /* TUI_ACCESSIBILITY_H */
