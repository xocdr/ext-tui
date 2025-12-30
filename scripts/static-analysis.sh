#!/bin/bash
#
# Run static analysis tools on ext-tui source code.
#
# Tools used:
# - cppcheck: General C/C++ static analyzer
# - clang-tidy: LLVM-based linter (optional)
#
# Usage:
#   ./scripts/static-analysis.sh [--verbose]
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXT_DIR="$(dirname "$SCRIPT_DIR")"
VERBOSE=""

if [ "$1" = "--verbose" ] || [ "$1" = "-v" ]; then
    VERBOSE="--verbose"
fi

cd "$EXT_DIR"

echo "=== Static Analysis for ext-tui ==="
echo ""

# Check if cppcheck is installed
if command -v cppcheck &> /dev/null; then
    echo "[1/2] Running cppcheck..."
    echo ""

    # Run cppcheck on our C sources (exclude Yoga C++ code)
    cppcheck \
        --enable=warning,style,performance,portability \
        --suppress=missingIncludeSystem \
        --suppress=unusedFunction \
        --suppress=unmatchedSuppression \
        --error-exitcode=0 \
        --inline-suppr \
        -I src \
        -I . \
        $VERBOSE \
        tui.c \
        src/terminal/*.c \
        src/event/*.c \
        src/node/*.c \
        src/render/*.c \
        src/text/*.c \
        src/drawing/*.c \
        src/app/*.c \
        src/testing/*.c \
        src/pool/*.c \
        2>&1 | grep -v "^$" || true

    echo ""
    echo "cppcheck completed."
else
    echo "[1/2] cppcheck not installed - skipping"
    echo "      Install with: brew install cppcheck"
fi

echo ""

# Check if clang-tidy is installed
if command -v clang-tidy &> /dev/null; then
    echo "[2/2] Running clang-tidy..."
    echo ""

    # Get PHP include paths
    PHP_INCLUDES=$(php-config --includes 2>/dev/null || echo "")

    # Run clang-tidy on key files (not all to avoid noise)
    clang-tidy \
        --quiet \
        -checks='-*,bugprone-*,cert-*,clang-analyzer-*,-cert-err33-c,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling' \
        src/text/measure.c \
        src/terminal/ansi.c \
        src/event/input.c \
        -- \
        -I. -Isrc $PHP_INCLUDES \
        2>&1 | grep -v "^$" || true

    echo ""
    echo "clang-tidy completed."
else
    echo "[2/2] clang-tidy not installed - skipping"
    echo "      Install with: brew install llvm"
fi

echo ""
echo "=== Static Analysis Complete ==="
echo ""
echo "Notes:"
echo "- Warnings from PHP headers are expected and can be ignored"
echo "- Focus on warnings in src/ and tui.c files"
echo "- Some warnings may be false positives - review carefully"
