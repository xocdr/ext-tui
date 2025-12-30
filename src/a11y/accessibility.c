/*
  +----------------------------------------------------------------------+
  | ext-tui: Accessibility support                                       |
  +----------------------------------------------------------------------+
  | Copyright (c) The Exocoder Authors                                   |
  +----------------------------------------------------------------------+
  | This source file is subject to the MIT license that is bundled with |
  | this package in the file LICENSE.                                    |
  +----------------------------------------------------------------------+
*/

#include "accessibility.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

/* Role name lookup table */
static const char *role_names[] = {
    "",              /* NONE */
    "button",
    "checkbox",
    "dialog",
    "grid",
    "gridcell",
    "group",
    "heading",
    "img",
    "link",
    "list",
    "listitem",
    "menu",
    "menuitem",
    "navigation",
    "progressbar",
    "radio",
    "region",
    "scrollbar",
    "search",
    "slider",
    "spinbutton",
    "status",
    "tab",
    "tablist",
    "tabpanel",
    "textbox",
    "tree",
    "treeitem",
    "alert",
    "alertdialog",
    "tooltip"
};

#define NUM_ROLES (sizeof(role_names) / sizeof(role_names[0]))

void tui_aria_props_init(tui_aria_props *props)
{
    if (!props) return;

    props->label = NULL;
    props->description = NULL;
    props->role = TUI_ARIA_ROLE_NONE;
    props->custom_role = NULL;
    props->hidden = TUI_ARIA_UNSET;
    props->expanded = TUI_ARIA_UNSET;
    props->checked = TUI_ARIA_UNSET;
    props->disabled = TUI_ARIA_UNSET;
    props->selected = TUI_ARIA_UNSET;
    props->level = 0;
    props->value_now = 0;
    props->value_min = 0;
    props->value_max = 0;
    props->live = NULL;
}

void tui_aria_props_free(tui_aria_props *props)
{
    if (!props) return;

    free(props->label);
    free(props->description);
    free(props->custom_role);
    free(props->live);

    props->label = NULL;
    props->description = NULL;
    props->custom_role = NULL;
    props->live = NULL;
}

int tui_aria_set_label(tui_aria_props *props, const char *label)
{
    if (!props) return -1;

    free(props->label);
    props->label = NULL;

    if (label) {
        props->label = strdup(label);
        if (!props->label) return -1;
    }

    return 0;
}

int tui_aria_set_description(tui_aria_props *props, const char *description)
{
    if (!props) return -1;

    free(props->description);
    props->description = NULL;

    if (description) {
        props->description = strdup(description);
        if (!props->description) return -1;
    }

    return 0;
}

int tui_aria_set_role(tui_aria_props *props, tui_aria_role role, const char *custom_role)
{
    if (!props) return -1;

    props->role = role;

    free(props->custom_role);
    props->custom_role = NULL;

    if (role == TUI_ARIA_ROLE_CUSTOM && custom_role) {
        props->custom_role = strdup(custom_role);
        if (!props->custom_role) return -1;
    }

    return 0;
}

const char *tui_aria_role_to_string(tui_aria_role role)
{
    if (role == TUI_ARIA_ROLE_CUSTOM) {
        return "custom";
    }

    if (role >= 0 && (size_t)role < NUM_ROLES) {
        return role_names[role];
    }

    return "";
}

tui_aria_role tui_aria_role_from_string(const char *role_str)
{
    if (!role_str || !*role_str) {
        return TUI_ARIA_ROLE_NONE;
    }

    for (size_t i = 0; i < NUM_ROLES; i++) {
        if (strcmp(role_str, role_names[i]) == 0) {
            return (tui_aria_role)i;
        }
    }

    /* Not a standard role - treat as custom */
    return TUI_ARIA_ROLE_CUSTOM;
}

/* Write output to stdout */
static void write_stdout(const char *data, size_t len)
{
    size_t written = 0;
    while (written < len) {
        ssize_t n = write(STDOUT_FILENO, data + written, len - written);
        if (n <= 0) break;
        written += (size_t)n;
    }
}

int tui_announce(const char *message, tui_announce_priority priority)
{
    if (!message) return -1;

    /* Screen reader announcement via OSC 777 (used by VTE terminals)
     * Format: ESC ] 777 ; notify ; title ; body ST
     * Also try OSC 9 (iTerm2) and console.log for accessibility tools
     */

    size_t msg_len = strlen(message);

    /* Buffer for escape sequence */
    size_t buf_size = msg_len + 64;
    char *buf = malloc(buf_size);
    if (!buf) return -1;

    const char *urgency = (priority == TUI_ANNOUNCE_ASSERTIVE) ? "critical" : "normal";

    /* VTE notification (OSC 777) */
    int len = snprintf(buf, buf_size,
        "\x1b]777;notify;Announcement;%s\x07", message);

    if (len > 0 && (size_t)len < buf_size) {
        write_stdout(buf, (size_t)len);
    }

    /* iTerm2 notification (OSC 9) */
    len = snprintf(buf, buf_size,
        "\x1b]9;%s\x07", message);

    if (len > 0 && (size_t)len < buf_size) {
        write_stdout(buf, (size_t)len);
    }

    /* Kitty notification (OSC 99) with urgency */
    len = snprintf(buf, buf_size,
        "\x1b]99;i=1:u=%s;%s\x07",
        (priority == TUI_ANNOUNCE_ASSERTIVE) ? "2" : "0",
        message);

    if (len > 0 && (size_t)len < buf_size) {
        write_stdout(buf, (size_t)len);
    }

    /* Suppress unused variable warning */
    (void)urgency;

    free(buf);
    return 0;
}

int tui_prefers_reduced_motion(void)
{
    /* Check various environment variables */
    const char *reduce_motion = getenv("REDUCE_MOTION");
    if (reduce_motion && (strcmp(reduce_motion, "1") == 0 ||
                          strcmp(reduce_motion, "true") == 0)) {
        return 1;
    }

    const char *no_motion = getenv("NO_MOTION");
    if (no_motion && (strcmp(no_motion, "1") == 0 ||
                      strcmp(no_motion, "true") == 0)) {
        return 1;
    }

    /* Check for accessibility indicators */
    const char *a11y = getenv("GTK_MODULES");
    if (a11y && strstr(a11y, "gail") != NULL) {
        /* GAIL indicates screen reader may be active */
        return 1;
    }

    return 0;
}

int tui_prefers_high_contrast(void)
{
    const char *high_contrast = getenv("HIGH_CONTRAST");
    if (high_contrast && (strcmp(high_contrast, "1") == 0 ||
                          strcmp(high_contrast, "true") == 0)) {
        return 1;
    }

    /* Check for monochrome/no color terminal */
    const char *colorterm = getenv("COLORTERM");
    if (colorterm && strcmp(colorterm, "none") == 0) {
        return 1;
    }

    /* Check for GTK high contrast theme */
    const char *gtk_theme = getenv("GTK_THEME");
    if (gtk_theme && strstr(gtk_theme, "HighContrast") != NULL) {
        return 1;
    }

    return 0;
}

int tui_get_accessibility_features(void)
{
    int features = 0;

    if (tui_prefers_reduced_motion()) {
        features |= TUI_A11Y_REDUCED_MOTION;
    }

    if (tui_prefers_high_contrast()) {
        features |= TUI_A11Y_HIGH_CONTRAST;
    }

    /* Screen reader detection (very limited) */
    const char *orca = getenv("ORCA_PID");
    const char *a11y = getenv("ACCESSIBILITY_ENABLED");
    if ((orca && *orca) ||
        (a11y && (strcmp(a11y, "1") == 0 || strcmp(a11y, "true") == 0))) {
        features |= TUI_A11Y_SCREEN_READER;
    }

    return features;
}
