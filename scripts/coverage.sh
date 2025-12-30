#!/bin/bash
#
# Code coverage script for ext-tui
#
# Usage:
#   ./scripts/coverage.sh [report]
#
# This script:
#   1. Rebuilds the extension with gcov support
#   2. Runs the test suite
#   3. Generates coverage reports
#
# Requirements:
#   - gcov (included with gcc/clang)
#   - lcov (optional, for HTML reports)
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
COVERAGE_DIR="$PROJECT_DIR/coverage"
REPORT_TYPE="${1:-summary}"

cd "$PROJECT_DIR"

echo "=== ext-tui Code Coverage ==="
echo ""

# Step 1: Clean and rebuild with coverage flags
echo "[1/4] Rebuilding with coverage instrumentation..."

# Check if already built with coverage
if [ ! -f ".coverage_build" ]; then
    make clean 2>/dev/null || true
    phpize --clean 2>/dev/null || true
    phpize

    # Configure with coverage flags
    CFLAGS="-O0 -g --coverage -fprofile-arcs -ftest-coverage" \
    LDFLAGS="--coverage" \
    ./configure --enable-tui

    make
    touch .coverage_build
else
    echo "  Using existing coverage build (delete .coverage_build to rebuild)"
fi

# Step 2: Clean previous coverage data
echo "[2/4] Cleaning previous coverage data..."
find . -name "*.gcda" -delete 2>/dev/null || true
rm -rf "$COVERAGE_DIR" 2>/dev/null || true
mkdir -p "$COVERAGE_DIR"

# Step 3: Run tests to generate coverage data
echo "[3/4] Running tests..."
make test TESTS=tests/ 2>&1 | tail -10

# Step 4: Generate coverage report
echo "[4/4] Generating coverage report..."
echo ""

case "$REPORT_TYPE" in
    summary)
        # Use gcov for summary
        echo "=== Coverage Summary ==="
        echo ""

        # Process each source file
        for src in tui.c src/**/*.c; do
            if [ -f "$src" ]; then
                # Get the corresponding .gcno file
                gcno="${src%.c}.gcno"
                if [ -f "$gcno" ]; then
                    gcov -n "$src" 2>/dev/null | grep -E "^File|^Lines" | head -2
                fi
            fi
        done
        ;;

    html)
        # Check for lcov
        if ! command -v lcov &> /dev/null; then
            echo "Error: lcov not installed. Install with: brew install lcov"
            exit 1
        fi

        # Generate lcov data
        lcov --capture --directory . --output-file "$COVERAGE_DIR/coverage.info" \
             --exclude '/opt/*' --exclude '/usr/*' --exclude '*/yoga/*' 2>/dev/null

        # Generate HTML report
        genhtml "$COVERAGE_DIR/coverage.info" --output-directory "$COVERAGE_DIR/html"

        echo "HTML report generated: $COVERAGE_DIR/html/index.html"

        # Open in browser (macOS)
        if [ "$(uname)" = "Darwin" ]; then
            open "$COVERAGE_DIR/html/index.html"
        fi
        ;;

    gcov)
        # Generate detailed gcov files
        for src in tui.c src/**/*.c; do
            if [ -f "$src" ]; then
                gcov -o "$(dirname "$src")" "$src" 2>/dev/null || true
            fi
        done

        echo "Generated .gcov files in source directories"
        echo ""
        echo "Files with coverage data:"
        find . -name "*.gcov" -type f | head -20
        ;;

    *)
        echo "Unknown report type: $REPORT_TYPE"
        echo "Available: summary, html, gcov"
        exit 1
        ;;
esac

echo ""
echo "=== Coverage Complete ==="
echo ""
echo "To clean coverage build:"
echo "  rm .coverage_build && make clean && phpize --clean"
