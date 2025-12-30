#!/bin/bash
#
# Run ext-tui under Valgrind for memory error detection.
#
# Valgrind detects:
# - Memory leaks
# - Use of uninitialized memory
# - Invalid memory access
# - Double-free / use-after-free
#
# Usage:
#   ./scripts/test-valgrind.sh                    # Run all tests
#   ./scripts/test-valgrind.sh tests/001.phpt    # Run specific test
#   ./scripts/test-valgrind.sh script.php        # Run PHP script
#
# Prerequisites:
#   - Valgrind must be installed
#   - Extension must be built (make)
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$EXT_DIR"

# Check if extension is built
if [ ! -f "modules/tui.so" ]; then
    echo "Error: Extension not built. Run 'make' first."
    exit 1
fi

# Check if Valgrind is installed
if ! command -v valgrind &> /dev/null; then
    echo "Error: Valgrind is not installed."
    echo "Install with: brew install valgrind (macOS) or apt install valgrind (Linux)"
    exit 1
fi

# Bypass Zend memory manager to let Valgrind track allocations
export USE_ZEND_ALLOC=0

# PHP options
PHP_OPTS="-n -d extension=$EXT_DIR/modules/tui.so -d memory_limit=256M"

echo "=== Running with Valgrind ==="
echo "Extension: $EXT_DIR/modules/tui.so"
echo ""

if [ -n "$1" ]; then
    if [[ "$1" == *.phpt ]]; then
        # Run specific .phpt test - extract PHP code and run it
        echo "Running test: $1"
        # Extract the --FILE-- section from the .phpt file
        TEMP_FILE=$(mktemp)
        awk '/^--FILE--$/,/^--EXPECT/' "$1" | grep -v '^--FILE--$' | grep -v '^--EXPECT' > "$TEMP_FILE"

        valgrind --leak-check=full \
            --show-leak-kinds=definite,possible \
            --track-origins=yes \
            --error-exitcode=1 \
            --suppressions=/dev/null \
            php $PHP_OPTS "$TEMP_FILE"

        RESULT=$?
        rm -f "$TEMP_FILE"
        exit $RESULT
    else
        # Run PHP script directly
        echo "Running script: $1"
        valgrind --leak-check=full \
            --show-leak-kinds=definite,possible \
            --track-origins=yes \
            --error-exitcode=1 \
            php $PHP_OPTS "$@"
    fi
else
    # Run a basic smoke test
    echo "Running smoke test..."
    valgrind --leak-check=full \
        --show-leak-kinds=definite,possible \
        --track-origins=yes \
        --error-exitcode=1 \
        php $PHP_OPTS -r "
            echo 'Testing tui_string_width: ';
            var_dump(tui_string_width('Hello'));

            echo 'Testing tui_truncate: ';
            var_dump(tui_truncate('Hello World', 8));

            echo 'Testing tui_wrap_text: ';
            var_dump(tui_wrap_text('Hello World', 5));

            echo 'Testing Box creation: ';
            \$box = new Xocdr\Tui\Ext\Box();
            var_dump(get_class(\$box));

            echo 'Testing Text creation: ';
            \$text = new Xocdr\Tui\Ext\Text('Hello');
            var_dump(get_class(\$text));

            echo 'All basic tests passed!';
        "
fi

echo ""
echo "=== Valgrind test complete ==="
