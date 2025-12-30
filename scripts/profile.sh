#!/bin/bash
#
# Performance profiling script for ext-tui
#
# Usage:
#   ./scripts/profile.sh [benchmark]
#
# Benchmarks:
#   string_width  - UTF-8 string width calculation
#   wrap_text     - Text wrapping
#   render        - Full render cycle
#   all           - Run all benchmarks (default)
#
# Requirements:
#   - PHP with tui extension
#   - time command
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BENCHMARK="${1:-all}"

# Check extension is built
if [ ! -f "$PROJECT_DIR/modules/tui.so" ]; then
    echo "Error: Extension not built. Run 'make' first."
    exit 1
fi

PHP_OPTS="-n -d extension=$PROJECT_DIR/modules/tui.so"

echo "=== ext-tui Performance Profiling ==="
echo ""

# Benchmark: String width calculation
benchmark_string_width() {
    echo "[Benchmark: string_width]"
    echo "Testing tui_string_width() with various inputs..."
    echo ""

    php $PHP_OPTS << 'PHPCODE'
<?php
$iterations = 100000;

// Test cases
$tests = [
    'ASCII short' => 'Hello, World!',
    'ASCII long' => str_repeat('The quick brown fox jumps over the lazy dog. ', 10),
    'CJK short' => '你好世界',
    'CJK long' => str_repeat('这是一段较长的中文文本用于测试。', 10),
    'Emoji' => '👋🏻😀🔥🚀⭐🇺🇸🇯🇵',
    'Mixed' => 'Hello 你好 👋 World 世界 🌍',
    'ANSI codes' => "\033[31mRed\033[0m \033[32mGreen\033[0m \033[34mBlue\033[0m",
];

foreach ($tests as $name => $text) {
    $start = hrtime(true);
    for ($i = 0; $i < $iterations; $i++) {
        tui_string_width($text);
    }
    $elapsed = (hrtime(true) - $start) / 1e6; // ms
    $ops_per_sec = $iterations / ($elapsed / 1000);
    printf("  %-15s: %8.2f ms (%s ops/sec)\n", $name, $elapsed, number_format($ops_per_sec, 0));
}
PHPCODE
    echo ""
}

# Benchmark: Text wrapping
benchmark_wrap_text() {
    echo "[Benchmark: wrap_text]"
    echo "Testing tui_wrap_text() with various inputs..."
    echo ""

    php $PHP_OPTS << 'PHPCODE'
<?php
$iterations = 10000;

$paragraph = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.";

$tests = [
    'Short/40' => [str_repeat('word ', 10), 40],
    'Medium/60' => [$paragraph, 60],
    'Long/80' => [str_repeat($paragraph . ' ', 5), 80],
    'CJK/30' => [str_repeat('这是中文测试文本。', 20), 30],
];

foreach ($tests as $name => [$text, $width]) {
    $start = hrtime(true);
    for ($i = 0; $i < $iterations; $i++) {
        tui_wrap_text($text, $width);
    }
    $elapsed = (hrtime(true) - $start) / 1e6;
    $ops_per_sec = $iterations / ($elapsed / 1000);
    printf("  %-15s: %8.2f ms (%s ops/sec)\n", $name, $elapsed, number_format($ops_per_sec, 0));
}
PHPCODE
    echo ""
}

# Benchmark: Truncate
benchmark_truncate() {
    echo "[Benchmark: truncate]"
    echo "Testing tui_truncate() with various inputs..."
    echo ""

    php $PHP_OPTS << 'PHPCODE'
<?php
$iterations = 100000;

$tests = [
    'Short ASCII' => ['Hello, World!', 10],
    'Long ASCII' => [str_repeat('The quick brown fox. ', 10), 50],
    'CJK' => ['这是一段较长的中文文本用于测试性能', 20],
    'Mixed' => ['Hello 你好 World 世界 🌍 Earth', 15],
];

foreach ($tests as $name => [$text, $width]) {
    $start = hrtime(true);
    for ($i = 0; $i < $iterations; $i++) {
        tui_truncate($text, $width);
    }
    $elapsed = (hrtime(true) - $start) / 1e6;
    $ops_per_sec = $iterations / ($elapsed / 1000);
    printf("  %-15s: %8.2f ms (%s ops/sec)\n", $name, $elapsed, number_format($ops_per_sec, 0));
}
PHPCODE
    echo ""
}

# Benchmark: Canvas operations
benchmark_canvas() {
    echo "[Benchmark: canvas]"
    echo "Testing canvas drawing operations..."
    echo ""

    php $PHP_OPTS << 'PHPCODE'
<?php
if (!defined('TUI_CANVAS_BRAILLE')) {
    echo "  Canvas functions not available in this build.\n";
    exit(0);
}

$iterations = 1000;

// Create canvas
$canvas = tui_canvas_create(160, 80, TUI_CANVAS_BRAILLE);

// Benchmark line drawing
$start = hrtime(true);
for ($i = 0; $i < $iterations; $i++) {
    tui_canvas_clear($canvas);
    for ($j = 0; $j < 100; $j++) {
        tui_canvas_line($canvas, rand(0, 159), rand(0, 79), rand(0, 159), rand(0, 79));
    }
}
$elapsed = (hrtime(true) - $start) / 1e6;
printf("  %-15s: %8.2f ms (%d iterations)\n", "100 lines/iter", $elapsed, $iterations);

// Benchmark circle drawing
$start = hrtime(true);
for ($i = 0; $i < $iterations; $i++) {
    tui_canvas_clear($canvas);
    for ($j = 0; $j < 50; $j++) {
        tui_canvas_circle($canvas, rand(20, 140), rand(20, 60), rand(5, 20));
    }
}
$elapsed = (hrtime(true) - $start) / 1e6;
printf("  %-15s: %8.2f ms (%d iterations)\n", "50 circles/iter", $elapsed, $iterations);

// Benchmark render
$start = hrtime(true);
for ($i = 0; $i < $iterations; $i++) {
    $lines = tui_canvas_render($canvas);
}
$elapsed = (hrtime(true) - $start) / 1e6;
printf("  %-15s: %8.2f ms (%d iterations)\n", "Render", $elapsed, $iterations);
PHPCODE
    echo ""
}

# Benchmark: Buffer operations
benchmark_buffer() {
    echo "[Benchmark: buffer]"
    echo "Testing buffer drawing operations..."
    echo ""

    php $PHP_OPTS << 'PHPCODE'
<?php
if (!function_exists('tui_buffer_create')) {
    echo "  Buffer functions not available in this build.\n";
    exit(0);
}

$iterations = 1000;

$buffer = tui_buffer_create(80, 24);
$style = ['color' => [255, 255, 255]];

// Benchmark rectangle drawing
$start = hrtime(true);
for ($i = 0; $i < $iterations; $i++) {
    tui_buffer_clear($buffer);
    for ($j = 0; $j < 20; $j++) {
        tui_draw_rect($buffer, rand(0, 60), rand(0, 18), rand(5, 15), rand(3, 6), 'single', $style);
    }
}
$elapsed = (hrtime(true) - $start) / 1e6;
printf("  %-15s: %8.2f ms (%d iterations)\n", "20 rects/iter", $elapsed, $iterations);

// Benchmark buffer render
$start = hrtime(true);
for ($i = 0; $i < $iterations; $i++) {
    $output = tui_buffer_render($buffer);
}
$elapsed = (hrtime(true) - $start) / 1e6;
printf("  %-15s: %8.2f ms (%d iterations)\n", "render", $elapsed, $iterations);
PHPCODE
    echo ""
}

# Benchmark: Easing functions
benchmark_easing() {
    echo "[Benchmark: easing]"
    echo "Testing animation/easing functions..."
    echo ""

    php $PHP_OPTS << 'PHPCODE'
<?php
if (!defined('TUI_EASE_LINEAR')) {
    echo "  Easing functions not available in this build.\n";
    exit(0);
}

$iterations = 100000;

$easings = [
    'LINEAR' => TUI_EASE_LINEAR,
    'IN_QUAD' => TUI_EASE_IN_QUAD,
    'OUT_BOUNCE' => TUI_EASE_OUT_BOUNCE,
    'OUT_ELASTIC' => TUI_EASE_OUT_ELASTIC,
];

foreach ($easings as $name => $easing) {
    $start = hrtime(true);
    for ($i = 0; $i < $iterations; $i++) {
        tui_ease(($i % 100) / 100.0, $easing);
    }
    $elapsed = (hrtime(true) - $start) / 1e6;
    $ops_per_sec = $iterations / ($elapsed / 1000);
    printf("  %-15s: %8.2f ms (%s ops/sec)\n", $name, $elapsed, number_format($ops_per_sec, 0));
}

// Color interpolation
$from = [255, 0, 0];
$to = [0, 0, 255];
$start = hrtime(true);
for ($i = 0; $i < $iterations; $i++) {
    tui_lerp_color($from, $to, ($i % 100) / 100.0);
}
$elapsed = (hrtime(true) - $start) / 1e6;
$ops_per_sec = $iterations / ($elapsed / 1000);
printf("  %-15s: %8.2f ms (%s ops/sec)\n", "lerp_color", $elapsed, number_format($ops_per_sec, 0));
PHPCODE
    echo ""
}

# Run benchmarks
case "$BENCHMARK" in
    string_width)
        benchmark_string_width
        ;;
    wrap_text)
        benchmark_wrap_text
        ;;
    truncate)
        benchmark_truncate
        ;;
    canvas)
        benchmark_canvas
        ;;
    buffer)
        benchmark_buffer
        ;;
    easing)
        benchmark_easing
        ;;
    all)
        benchmark_string_width
        benchmark_wrap_text
        benchmark_truncate
        benchmark_canvas
        benchmark_buffer
        benchmark_easing
        ;;
    *)
        echo "Unknown benchmark: $BENCHMARK"
        echo "Available: string_width, wrap_text, truncate, canvas, buffer, easing, all"
        exit 1
        ;;
esac

echo "=== Profiling Complete ==="
