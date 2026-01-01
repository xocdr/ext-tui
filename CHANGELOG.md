# Changelog

All notable changes to ext-tui will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.3.1] - Unreleased

### Added
- String interning pool for node keys and IDs (`src/pool/intern.c`)
- `tui_node_set_key()` function for setting interned keys
- `key_interned` and `id_interned` flags on `tui_node` structure
- Pointer equality fast-path in keymap for interned string comparisons

### Changed
- `tui_node_set_id()` now uses string interning for memory efficiency
- Node key/ID comparisons use pointer equality before strcmp() when interned

### Performance
- Reduced memory allocations for repeated key/ID strings
- O(1) key comparisons for interned strings in reconciler

## [0.3.0] - 2024-12-31

### Changed
- **BREAKING**: Minimum PHP version raised to 8.4.0
- Removed PHP 8.1-8.3 compatibility code
- CI now tests PHP 8.4 and 8.5 only

### Added
- `showCursor` property on Box nodes for focus-based cursor visibility
- Typed exceptions: `InstanceDestroyedException`, `ValidationException`, etc.

### Fixed
- Exit artifact (`/Us%`) no longer appears after exiting alternate screen
- Heap corruption on rerender caused by dangling zval pointer

## [0.2.14] - 2024-12-30

### Fixed
- PHP 8.1 compatibility for enum value lookup

## [0.2.11] - 2024-12-29

### Added
- Screen recording functionality (`tui_record_*` functions)
- Accessibility features (`tui_announce`, `tui_prefers_reduced_motion`)
- Drag and drop support (`tui_drag_*` functions)
- Grapheme cluster counting (`tui_grapheme_count`)
- Kitty/iTerm2/Sixel graphics protocol support
- Virtual list windowing for large datasets
- Smooth scrolling animations

## [0.2.10] - 2024-12-28

### Added
- Terminal notification functions (`tui_bell`, `tui_flash`, `tui_notify`)
- Terminal capability detection (`tui_get_capabilities`, `tui_has_capability`)
- Mouse event handling (`tui_mouse_enable`, `tui_mouse_disable`)
- Clipboard operations (`tui_clipboard_copy`, `tui_clipboard_request`)
- Input history management (`tui_history_*` functions)

## [0.2.0] - 2024-12-27

### Added
- Object pooling for children arrays and key maps
- Telemetry/metrics system (`tui_metrics_*` functions)
- Test renderer for headless testing (`tui_test_*` functions)
- Buffer and canvas drawing primitives
- Animation easing functions
- Table rendering
- Progress bars and spinners
- Sprite system for animations

### Changed
- Reconciler uses hash-based key map for O(1) lookups
- Iterative node destruction to prevent stack overflow

## [0.1.0] - 2024-12-26

### Added
- Initial release
- Terminal handling (raw mode, ANSI escape codes)
- Yoga layout engine integration (flexbox for terminals)
- Virtual DOM with reconciler (diff-based updates)
- Event loop for keyboard input
- Text utilities (Unicode width, wrapping, truncation)
- Box and Text node types
- Focus management system
