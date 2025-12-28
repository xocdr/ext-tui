# Installation

[← Back to Documentation](../README.md) | [Next: Quick Start →](quickstart.md)

---

## System Requirements

- **PHP**: 8.0 or higher
- **OS**: Linux, macOS, or BSD (Windows not currently supported)
- **Terminal**: UTF-8 capable terminal emulator
- **Dependencies**: Yoga layout library

## Installing from Source

### 1. Install Dependencies

#### macOS (Homebrew)

```bash
# Install Yoga layout library
brew install yoga

# Ensure PHP development headers are available
brew install php
```

#### Ubuntu/Debian

```bash
# Install build tools
sudo apt-get install build-essential php-dev

# Install Yoga (may need to build from source)
git clone https://github.com/facebook/yoga.git
cd yoga
mkdir build && cd build
cmake ..
make
sudo make install
```

#### Fedora/RHEL

```bash
sudo dnf install php-devel gcc make
# Install Yoga from source (see Ubuntu instructions)
```

### 2. Clone and Build

```bash
# Clone the repository
git clone https://github.com/yourname/ext-tui.git
cd ext-tui

# Generate configure script
phpize

# Configure (adjust paths as needed)
./configure --with-yoga=/opt/homebrew  # macOS with Homebrew
# or
./configure --with-yoga=/usr/local     # Linux

# Build
make

# Run tests (optional but recommended)
make test

# Install system-wide
sudo make install
```

### 3. Enable the Extension

Add to your `php.ini`:

```ini
extension=tui.so
```

Or create a dedicated configuration file:

```bash
# Find PHP extension directory
php -i | grep extension_dir

# Create config file (adjust path for your system)
echo "extension=tui.so" | sudo tee /etc/php/8.3/cli/conf.d/20-tui.ini
```

### 4. Verify Installation

```bash
php -m | grep tui
# Should output: tui

php -r "var_dump(tui_get_terminal_size());"
# Should output terminal dimensions
```

## Configuration

The extension has no required configuration. All functions work immediately after loading.

### Optional php.ini Settings

Currently, ext-tui does not require any php.ini directives. All behavior is controlled via function parameters.

## Troubleshooting

### Extension not loading

```bash
# Check for errors
php -d "extension=tui.so" -m 2>&1 | head -20

# Verify the .so file exists
ls -la $(php -i | grep extension_dir | awk '{print $NF}')/tui.so
```

### Yoga library not found

```bash
# Check if Yoga is installed
ldconfig -p | grep yoga  # Linux
otool -L /path/to/tui.so | grep yoga  # macOS

# If missing, ensure LD_LIBRARY_PATH includes Yoga
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

### Build errors

```bash
# Clean and rebuild
make clean
phpize --clean
phpize
./configure --with-yoga=/path/to/yoga
make
```

### Terminal not detected

```php
// Check if running in a terminal
if (!tui_is_interactive()) {
    echo "Not running in an interactive terminal\n";
    exit(1);
}
```

## Building for Development

For development and debugging:

```bash
# Build with debug symbols
./configure --enable-debug --with-yoga=/opt/homebrew
make

# Run specific test
php -d "extension=modules/tui.so" test.php

# Run with Valgrind (memory checking)
valgrind --leak-check=full php -d "extension=modules/tui.so" test.php
```

## Uninstalling

```bash
# Remove the extension
sudo rm $(php -i | grep extension_dir | awk '{print $NF}')/tui.so

# Remove configuration
sudo rm /etc/php/*/cli/conf.d/*-tui.ini
```

---

[← Back to Documentation](../README.md) | [Next: Quick Start →](quickstart.md)
