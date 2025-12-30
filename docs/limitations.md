# Known Limitations

This document describes known limitations and areas for future improvement in ext-tui.

## Current Limitations

### Terminal Compatibility

- **Tested on macOS only**: Primary development and testing has been on macOS. Linux support should work but is less tested.
- **Windows not supported**: The extension uses POSIX APIs (termios, poll, signals) that are not available on Windows.
- **Requires a TTY**: Input handling and raw mode require a real terminal. Piped input may not work correctly.

### Event Handling

- **Limited modifier detection**: Some terminal emulators may not send full modifier information (Shift, Alt, Ctrl) for all key combinations.
  - **SHIFT+letter**: ext-tui infers SHIFT from uppercase letters (A-Z) and normalizes the key to lowercase. So typing `A` results in `$key->shift = true` and `$key->key = 'a'`.
  - **SHIFT+number/symbol**: Cannot be detected. Terminals send the resulting character directly (e.g., `!` for Shift+1), with no way to know the original key or that Shift was pressed.
  - **SHIFT+arrow/special keys**: Detected via escape sequences when the terminal supports it (most modern terminals do).
- **Escape key delay**: Single Escape key press requires a timeout to distinguish from escape sequences, which may cause a brief delay.
- **Mouse support varies**: Mouse support uses SGR mode (1006) which is widely supported, but some terminals may have limited functionality.

### Text Rendering

- **Wide character handling**: CJK characters and emojis are assumed to be 2 cells wide. Some terminal emulators may display them differently.
- **No bidirectional text**: Right-to-left text (Arabic, Hebrew) is not properly supported.
- **No ligature support**: Font ligatures will not render correctly.

### Memory and Performance

- **No lazy rendering**: The entire visible area is re-rendered on each update. This may be slow for very complex UIs.
- **Node tree depth limit**: Tree traversal is limited to 256 levels to prevent stack overflow.

### Thread Safety

- **Not thread-safe**: ext-tui is designed for single-threaded use. All TUI operations must be performed from the same thread.
- **No concurrent access**: Multiple PHP threads or processes should not access the same TUI instance simultaneously.
- **Signal handlers**: Signal handling (e.g., for window resize) uses global state and is not reentrant.
- **Terminal state**: Raw mode and alternate screen buffer are global terminal states that cannot be safely shared.

**Recommendation**: Use a single PHP process for TUI applications. For concurrent operations, use async I/O or message passing rather than threads.

## Implemented Features

The following features have been fully implemented:

- ✅ **Yoga layout engine**: Fully integrated and vendored (no external dependency required)
- ✅ **Animation and transitions**: Using `tui_ease()`, `tui_lerp()`, and `tui_lerp_color()`
- ✅ **Easing functions**: 15+ built-in easing functions (linear, quad, cubic, bounce, elastic, etc.)
- ✅ **Canvas graphics**: Braille, block, and ASCII character modes
- ✅ **Sprites**: Animated sprite support with frame-based animation
- ✅ **Tables**: Table rendering with alignment and borders
- ✅ **Progress indicators**: Progress bars, busy bars, and spinners
- ✅ **Drawing primitives**: Lines, rectangles, circles, ellipses, triangles
- ✅ **Mouse events**: Click, scroll, drag with SGR mode (1006)
- ✅ **Clipboard**: OSC 52 clipboard read/write
- ✅ **Hyperlinks**: OSC 8 terminal hyperlinks
- ✅ **Bracketed paste**: Detect and handle pasted text
- ✅ **Input history**: Navigate through input history
- ✅ **Focus management**: Tab index, focus groups, focus traps

## Planned Improvements

### Near Term
- [ ] Built-in input components (TextInput, Select, Checkbox)
- [ ] Performance optimizations for large node trees

### Future
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
