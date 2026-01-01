# Constants Reference

Complete list of ext-tui constants.

## Mouse Tracking Modes

Used with `tui_mouse_enable()`.

| Constant | Value | Description |
|----------|-------|-------------|
| `TUI_MOUSE_MODE_OFF` | 0 | Disable mouse tracking |
| `TUI_MOUSE_MODE_CLICK` | 1 | Basic click events only (mode 1000) |
| `TUI_MOUSE_MODE_BUTTON` | 2 | Button press/release events (mode 1002) |
| `TUI_MOUSE_MODE_ALL` | 3 | All events including hover (mode 1003) |

## Notification Priority

Used with `tui_notify()`.

| Constant | Value | Description |
|----------|-------|-------------|
| `TUI_NOTIFY_NORMAL` | 0 | Normal priority |
| `TUI_NOTIFY_URGENT` | 1 | Urgent/critical priority |

## ARIA Roles

Used with `tui_aria_role_to_string()` and `tui_aria_role_from_string()`.

| Constant | Value | Description |
|----------|-------|-------------|
| `TUI_ARIA_ROLE_NONE` | 0 | No role |
| `TUI_ARIA_ROLE_BUTTON` | 1 | Button |
| `TUI_ARIA_ROLE_CHECKBOX` | 2 | Checkbox |
| `TUI_ARIA_ROLE_DIALOG` | 3 | Dialog |
| `TUI_ARIA_ROLE_MENU` | 12 | Menu |
| `TUI_ARIA_ROLE_MENUITEM` | 13 | Menu item |
| `TUI_ARIA_ROLE_NAVIGATION` | 14 | Navigation |
| `TUI_ARIA_ROLE_STATUS` | 22 | Status |
| `TUI_ARIA_ROLE_TEXTBOX` | 26 | Text input |
| `TUI_ARIA_ROLE_ALERT` | 29 | Alert |

## Version

| Constant | Type | Description |
|----------|------|-------------|
| `TUI_VERSION` | string | Version string (e.g., "0.4.4") |
| `TUI_VERSION_ID` | int | Version as integer (e.g., 404) |

---

## String-Based Options

The following options use string values instead of constants:

### Easing Functions

Used with `tui_ease()`.

| Value | Description |
|-------|-------------|
| `"linear"` | Constant speed (default) |
| `"in_quad"` | Quadratic acceleration |
| `"out_quad"` | Quadratic deceleration |
| `"in_out_quad"` | Quadratic both ends |
| `"in_cubic"` | Cubic acceleration |
| `"out_cubic"` | Cubic deceleration |
| `"in_out_cubic"` | Cubic both ends |
| `"in_quart"` | Quartic acceleration |
| `"out_quart"` | Quartic deceleration |
| `"in_out_quart"` | Quartic both ends |
| `"in_sine"` | Sinusoidal acceleration |
| `"out_sine"` | Sinusoidal deceleration |
| `"in_out_sine"` | Sinusoidal both ends |
| `"out_bounce"` | Bouncy end |
| `"out_elastic"` | Springy overshoot |
| `"out_back"` | Slight overshoot |

### Canvas Modes

Used with `tui_canvas_create()`.

| Value | Resolution | Description |
|-------|------------|-------------|
| `"braille"` | 2×4 per cell | Braille dot patterns (default) |
| `"block"` | 2×2 per cell | Block elements |
| `"ascii"` | 1×1 per cell | ASCII characters |

### Spinner Types

Used with `tui_spinner_frame()` and `tui_spinner_frame_count()`.

| Value | Characters |
|-------|------------|
| `"dots"` | Braille dots (⠋⠙⠹...) |
| `"line"` | Rotating line (\|/-\\) |
| `"bounce"` | Bouncing bar |
| `"circle"` | Rotating circle (◐◓◑◒) |

### Clipboard Targets

Used with `tui_clipboard_copy()`, `tui_clipboard_request()`, `tui_clipboard_clear()`.

| Value | Description |
|-------|-------------|
| `"clipboard"` | System clipboard (default) |
| `"primary"` | Primary selection (X11) |
| `"secondary"` | Secondary selection (X11) |
