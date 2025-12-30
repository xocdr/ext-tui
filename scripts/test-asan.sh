#!/bin/bash
#
# Run ext-tui tests with AddressSanitizer (ASAN) to detect memory errors.
#
# ASAN detects:
# - Buffer overflows (heap, stack, global)
# - Use-after-free
# - Double-free
# - Memory leaks
#
# Prerequisites:
# - PHP must be compiled with ASAN support, OR
# - Use USE_ZEND_ALLOC=0 to bypass Zend memory manager
#
# Usage:
#   ./scripts/test-asan.sh [test-file.phpt]
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

# ASAN options for better output
export ASAN_OPTIONS="detect_leaks=1:halt_on_error=0:print_stats=1:detect_stack_use_after_return=1"

# Bypass Zend memory manager to let ASAN track allocations
export USE_ZEND_ALLOC=0

# Increase memory limit for ASAN overhead
export TEST_PHP_ARGS="-d memory_limit=512M -d extension=$EXT_DIR/modules/tui.so"

echo "=== Running tests with AddressSanitizer ==="
echo "Extension: $EXT_DIR/modules/tui.so"
echo ""

if [ -n "$1" ]; then
    # Run specific test
    php $TEST_PHP_ARGS -r "echo 'ASAN test for: $1\n';"
    make test TESTS="$1"
else
    # Run all tests
    make test TESTS=tests/
fi

echo ""
echo "=== ASAN test complete ==="
