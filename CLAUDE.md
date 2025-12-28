# ext-tui

PHP C extension for terminal UI with embedded Yoga layout engine.

## Project Overview

This is a PHP extension that provides:
- Terminal handling (raw mode, ANSI escape codes)
- Yoga layout engine integration (flexbox for terminals)
- Virtual DOM with reconciler (diff-based updates)
- Event loop for keyboard input
- Text utilities (Unicode width, wrapping)

## Build Commands

```bash
# Generate configure script
phpize

# Configure the build
./configure --enable-tui

# Compile
make

# Run tests
make test

# Install (requires root/sudo)
make install

# Clean build artifacts
make clean
phpize --clean
```

## Development Workflow

```bash
# Full rebuild cycle
phpize --clean && phpize && ./configure --enable-tui && make

# Quick rebuild (after code changes)
make

# Run specific test
make test TESTS=tests/001-basic.phpt

# Check for memory leaks (requires debug build)
USE_ZEND_ALLOC=0 valgrind php -d extension=modules/tui.so test.php
```

## Project Structure

```
ext-tui/
├── config.m4              # Autoconf configuration
├── php_tui.h              # Extension header
├── tui.c                  # Extension entry point
├── src/
│   ├── yoga/              # Embedded Yoga layout engine
│   ├── terminal/          # Terminal handling (termios, ANSI)
│   ├── event/             # Event loop, input parsing
│   ├── node/              # Virtual DOM nodes, reconciler
│   ├── render/            # Character buffer, output
│   └── text/              # Unicode width, text wrapping
└── tests/
    └── *.phpt             # PHP extension tests
```

## Key Files

- `config.m4` - Build configuration, defines sources to compile
- `php_tui.h` - Module globals, function declarations
- `tui.c` - Module init, PHP function implementations

## PHP API (Target)

```php
// Lifecycle
tui_render(callable $component, array $options = []): TuiInstance
tui_rerender(TuiInstance $instance): void
tui_unmount(TuiInstance $instance): void
tui_wait_until_exit(TuiInstance $instance): void

// Terminal
tui_get_terminal_size(): array  // [width, height]
tui_is_interactive(): bool
tui_is_ci(): bool

// Text utilities
tui_string_width(string $text): int
tui_wrap_text(string $text, int $width): array
tui_truncate(string $text, int $width, string $ellipsis = '...'): string
```

## Classes Exposed

```php
class TuiBox { /* flexbox container node */ }
class TuiText { /* text node */ }
class TuiInstance { /* render instance */ }
class TuiKey { /* keyboard input */ }
```

## Dependencies

- PHP 8.1+
- Yoga (embedded, from npm package)
- Standard C library (termios, poll)

## Testing

Tests use PHP's `.phpt` format:

```
--TEST--
Basic tui_string_width test
--EXTENSIONS--
tui
--FILE--
<?php
var_dump(tui_string_width("Hello"));
var_dump(tui_string_width("你好"));
?>
--EXPECT--
int(5)
int(4)
```

## Related Packages

- `exocoder/tui` - Composer package with components and hooks
- `exocoder/tui-widgets` - Pre-built widget library

## Specifications

See `/docs/tui/SPEC.md` in the exocoder repository for full specification.
