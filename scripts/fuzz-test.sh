#!/bin/bash
#
# Simple fuzz testing for ext-tui input parsing functions.
#
# Tests the extension with random input to find edge cases and crashes.
# This is a basic fuzzer - for production use, consider AFL or libFuzzer.
#
# Usage:
#   ./scripts/fuzz-test.sh [iterations]
#
# Default: 1000 iterations
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXT_DIR="$(dirname "$SCRIPT_DIR")"
ITERATIONS="${1:-1000}"

cd "$EXT_DIR"

# Check if extension is built
if [ ! -f "modules/tui.so" ]; then
    echo "Error: Extension not built. Run 'make' first."
    exit 1
fi

# Use -n to prevent loading from php.ini (avoids "already loaded" warnings)
PHP_OPTS="-n -d extension=modules/tui.so -d memory_limit=64M"

echo "=== Fuzz Testing ext-tui ==="
echo "Iterations: $ITERATIONS"
echo ""

# Track failures
FAILURES=0

# Test 1: tui_string_width with random UTF-8
echo "[1/4] Fuzzing tui_string_width()..."
for i in $(seq 1 $ITERATIONS); do
    # Generate random bytes (some valid UTF-8, some invalid)
    INPUT=$(head -c $((RANDOM % 100 + 1)) /dev/urandom | base64 | head -c $((RANDOM % 50 + 1)))

    if ! php $PHP_OPTS -r "tui_string_width('$INPUT');" 2>/dev/null; then
        echo "  Failed on iteration $i"
        ((FAILURES++)) || true
    fi

    # Progress indicator
    if [ $((i % 100)) -eq 0 ]; then
        echo "  Progress: $i/$ITERATIONS"
    fi
done

# Test 2: tui_truncate with random input
echo "[2/4] Fuzzing tui_truncate()..."
for i in $(seq 1 $ITERATIONS); do
    INPUT=$(head -c $((RANDOM % 100 + 1)) /dev/urandom | base64 | head -c $((RANDOM % 50 + 1)))
    WIDTH=$((RANDOM % 100))

    if ! php $PHP_OPTS -r "tui_truncate('$INPUT', $WIDTH);" 2>/dev/null; then
        echo "  Failed on iteration $i"
        ((FAILURES++)) || true
    fi

    if [ $((i % 100)) -eq 0 ]; then
        echo "  Progress: $i/$ITERATIONS"
    fi
done

# Test 3: tui_wrap_text with random input
echo "[3/4] Fuzzing tui_wrap_text()..."
for i in $(seq 1 $ITERATIONS); do
    INPUT=$(head -c $((RANDOM % 200 + 1)) /dev/urandom | base64 | head -c $((RANDOM % 100 + 1)))
    WIDTH=$((RANDOM % 80 + 1))

    if ! php $PHP_OPTS -r "tui_wrap_text('$INPUT', $WIDTH);" 2>/dev/null; then
        echo "  Failed on iteration $i"
        ((FAILURES++)) || true
    fi

    if [ $((i % 100)) -eq 0 ]; then
        echo "  Progress: $i/$ITERATIONS"
    fi
done

# Test 4: ANSI utilities with random input
echo "[4/4] Fuzzing ANSI utilities..."
for i in $(seq 1 $ITERATIONS); do
    # Generate input that might contain ANSI sequences
    INPUT=$(printf '\033[%dm%s\033[0m' $((RANDOM % 100)) "$(head -c 20 /dev/urandom | base64)")

    if ! php $PHP_OPTS -r "
        \$s = '$INPUT';
        tui_strip_ansi(\$s);
        tui_string_width(\$s);
    " 2>/dev/null; then
        echo "  Failed on iteration $i"
        ((FAILURES++)) || true
    fi

    if [ $((i % 100)) -eq 0 ]; then
        echo "  Progress: $i/$ITERATIONS"
    fi
done

echo ""
echo "=== Fuzz Testing Complete ==="
echo "Total iterations: $((ITERATIONS * 4))"
echo "Failures: $FAILURES"

if [ $FAILURES -gt 0 ]; then
    echo "WARNING: Some fuzz tests failed!"
    exit 1
else
    echo "All fuzz tests passed."
    exit 0
fi
