# Constants Reference

Complete list of ext-tui constants.

## Easing Functions

Used with `tui_ease()`.

| Constant | Description |
|----------|-------------|
| `TUI_EASE_LINEAR` | Constant speed |
| `TUI_EASE_IN_QUAD` | Quadratic acceleration |
| `TUI_EASE_OUT_QUAD` | Quadratic deceleration |
| `TUI_EASE_IN_OUT_QUAD` | Quadratic both ends |
| `TUI_EASE_IN_CUBIC` | Cubic acceleration |
| `TUI_EASE_OUT_CUBIC` | Cubic deceleration |
| `TUI_EASE_IN_OUT_CUBIC` | Cubic both ends |
| `TUI_EASE_IN_QUART` | Quartic acceleration |
| `TUI_EASE_OUT_QUART` | Quartic deceleration |
| `TUI_EASE_IN_OUT_QUART` | Quartic both ends |
| `TUI_EASE_IN_SINE` | Sinusoidal acceleration |
| `TUI_EASE_OUT_SINE` | Sinusoidal deceleration |
| `TUI_EASE_IN_OUT_SINE` | Sinusoidal both ends |
| `TUI_EASE_OUT_BOUNCE` | Bouncy end |
| `TUI_EASE_OUT_ELASTIC` | Springy overshoot |
| `TUI_EASE_OUT_BACK` | Slight overshoot |

## Canvas Modes

Used with `tui_canvas_create()`.

| Constant | Resolution | Description |
|----------|------------|-------------|
| `TUI_CANVAS_BRAILLE` | 2×4 per cell | Braille dot patterns |
| `TUI_CANVAS_BLOCK` | 2×2 per cell | Block elements |
| `TUI_CANVAS_ASCII` | 1×1 per cell | ASCII characters |

## Spinner Types

Used with `tui_spinner_frame()`.

| Constant | Characters |
|----------|------------|
| `TUI_SPINNER_DOTS` | Braille dots (⠋⠙⠹...) |
| `TUI_SPINNER_LINE` | Rotating line (\|/-\\) |
| `TUI_SPINNER_BOUNCE` | Bouncing bar |
| `TUI_SPINNER_CIRCLE` | Rotating circle (◐◓◑◒) |

## Table Alignment

Used with `tui_table_set_align()`.

| Constant | Description |
|----------|-------------|
| `TUI_ALIGN_LEFT` | Left-align column |
| `TUI_ALIGN_CENTER` | Center column |
| `TUI_ALIGN_RIGHT` | Right-align column |
