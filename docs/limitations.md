# Known Limitations

This document describes known limitations and areas for future improvement in ext-tui.

## Current Limitations

### Terminal Compatibility

- **Tested on macOS only**: Primary development and testing has been on macOS. Linux support should work but is less tested.
- **Windows not supported**: The extension uses POSIX APIs (termios, poll, signals) that are not available on Windows.
- **Requires a TTY**: Input handling and raw mode require a real terminal. Piped input may not work correctly.

### Event Handling

- **No mouse support**: Currently only keyboard input is supported. Mouse events (clicks, scroll) are not implemented.
- **Limited modifier detection**: Some terminal emulators may not send full modifier information (Shift, Alt, Ctrl) for all key combinations.
- **Escape key delay**: Single Escape key press requires a timeout to distinguish from escape sequences, which may cause a brief delay.

### Layout Engine

- **Yoga not fully integrated**: The Yoga layout engine sources need to be downloaded separately. See README for setup instructions.
- **Layout calculation is synchronous**: Large node trees may cause noticeable delays during layout computation.

### Text Rendering

- **Wide character handling**: CJK characters and emojis are assumed to be 2 cells wide. Some terminal emulators may display them differently.
- **No bidirectional text**: Right-to-left text (Arabic, Hebrew) is not properly supported.
- **No ligature support**: Font ligatures will not render correctly.

### Memory and Performance

- **No lazy rendering**: The entire visible area is re-rendered on each update. This may be slow for very complex UIs.
- **Node tree depth limit**: Extremely deep node trees (100+ levels) may cause stack issues during rendering.

## Planned Improvements

### Near Term
- [ ] Mouse event support (click, scroll, drag)
- [ ] Built-in input components (TextInput, Select, Checkbox)
- [ ] Yoga layout integration completion
- [ ] Performance optimizations for large node trees

### Future
- [ ] Animation and transition support
- [ ] Better wide character / emoji handling
- [ ] Bidirectional text support
- [ ] Windows support via Windows Console API or WSL

## Reporting Issues

If you encounter issues or have feature requests, please:

1. Check if the issue is a known limitation above
2. Search existing issues to avoid duplicates
3. Provide a minimal reproducible example
4. Include your terminal emulator and OS version

## Workarounds

### For wide characters
If emojis or CJK characters don't display correctly, try:
- Using a terminal with good Unicode support (iTerm2, Kitty, Alacritty)
- Setting your terminal's font to one with good Unicode coverage

### For keyboard shortcuts
If certain key combinations don't work:
- Check your terminal's key bindings configuration
- Some keys (like Ctrl+S, Ctrl+Q) may be reserved for flow control
- Disable conflicting OS-level shortcuts

### For performance
If the UI feels slow:
- Reduce the number of nodes in your tree
- Avoid deep nesting when possible
- Use simpler border styles (or none)
- Increase the render throttle interval
