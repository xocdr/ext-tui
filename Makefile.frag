# ext-tui Makefile fragment

# Code coverage targets (requires --enable-tui-gcov during configure)
# Usage:
#   phpize && ./configure --enable-tui --enable-tui-gcov
#   make clean && make
#   make test
#   make coverage

# Coverage report output directory
COVERAGE_DIR = coverage

# Generate HTML coverage report using lcov
coverage: test
	@if [ ! -f ".libs/tui.gcno" ]; then \
		echo "Error: No coverage data found."; \
		echo "Rebuild with: ./configure --enable-tui --enable-tui-gcov"; \
		exit 1; \
	fi
	@command -v lcov >/dev/null 2>&1 || { echo "Error: lcov not installed"; exit 1; }
	@command -v genhtml >/dev/null 2>&1 || { echo "Error: genhtml not installed"; exit 1; }
	@mkdir -p $(COVERAGE_DIR)
	lcov --capture --directory . --output-file $(COVERAGE_DIR)/coverage.info \
		--exclude '/usr/*' \
		--exclude '*/src/yoga/*' \
		--exclude '*/php-src/*' \
		--ignore-errors inconsistent
	genhtml $(COVERAGE_DIR)/coverage.info --output-directory $(COVERAGE_DIR)/html \
		--ignore-errors inconsistent
	@echo ""
	@echo "Coverage report generated: $(COVERAGE_DIR)/html/index.html"

# Clean coverage data files
coverage-clean:
	find . -name "*.gcda" -delete
	find . -name "*.gcno" -delete
	rm -rf $(COVERAGE_DIR)

# Show coverage summary without generating full report
coverage-summary:
	@if [ ! -f ".libs/tui.gcda" ]; then \
		echo "No coverage data. Run 'make test' first."; \
		exit 1; \
	fi
	@echo "Coverage summary for main source files:"
	@gcov -o .libs tui.c 2>/dev/null | grep -E "^(File|Lines)" || echo "  tui.c: data available"
	@gcov -o .libs/src/app src/app/app.c 2>/dev/null | grep -E "^(File|Lines)" || echo "  app.c: data available"
	@gcov -o .libs/src/node src/node/node.c 2>/dev/null | grep -E "^(File|Lines)" || echo "  node.c: data available"
	@gcov -o .libs/src/render src/render/buffer.c 2>/dev/null | grep -E "^(File|Lines)" || echo "  buffer.c: data available"
	@gcov -o .libs/src/render src/render/output.c 2>/dev/null | grep -E "^(File|Lines)" || echo "  output.c: data available"
	@echo ""
	@echo "Run 'make coverage' for full HTML report (requires lcov)"

# Static analysis with clang-tidy
# Requires: clang-tidy, compile_commands.json (generate with bear or cmake)
lint:
	@command -v clang-tidy >/dev/null 2>&1 || { echo "Error: clang-tidy not installed"; exit 1; }
	@echo "Running clang-tidy on main source files..."
	@PHP_INCLUDE=$$(php-config --include-dir) && \
	clang-tidy tui.c \
		-p . \
		--quiet \
		-- -I. -I./src -I$$PHP_INCLUDE -I$$PHP_INCLUDE/main \
		-I$$PHP_INCLUDE/TSRM -I$$PHP_INCLUDE/Zend -I$$PHP_INCLUDE/ext \
		-DHAVE_CONFIG_H 2>/dev/null || true

# Quick lint (only show errors, not warnings)
lint-errors:
	@command -v clang-tidy >/dev/null 2>&1 || { echo "Error: clang-tidy not installed"; exit 1; }
	@echo "Running clang-tidy (errors only)..."
	@PHP_INCLUDE=$$(php-config --include-dir) && \
	clang-tidy tui.c \
		-p . \
		--warnings-as-errors='*' \
		--quiet \
		-- -I. -I./src -I$$PHP_INCLUDE -I$$PHP_INCLUDE/main \
		-I$$PHP_INCLUDE/TSRM -I$$PHP_INCLUDE/Zend -I$$PHP_INCLUDE/ext \
		-DHAVE_CONFIG_H 2>/dev/null || true

# Memory leak detection with valgrind
# Usage: make test-valgrind
# Requires: valgrind (install with: brew install valgrind on macOS with rosetta,
#           or apt-get install valgrind on Linux)
#
# Note: On macOS ARM64 (Apple Silicon), valgrind is not natively supported.
# Consider using AddressSanitizer instead (see test-asan target).

# Run tests under valgrind (Linux/x86)
test-valgrind:
	@command -v valgrind >/dev/null 2>&1 || { \
		echo "Error: valgrind not installed."; \
		echo "On Linux: apt-get install valgrind"; \
		echo "On macOS ARM64: valgrind not supported, use 'make test-asan' instead"; \
		exit 1; \
	}
	@echo "Running tests under valgrind (this may take a while)..."
	USE_ZEND_ALLOC=0 valgrind --leak-check=full --show-leak-kinds=all \
		--track-origins=yes --suppressions=/dev/null \
		--error-exitcode=1 \
		php -d extension=modules/tui.so \
		-r 'echo "Extension loaded successfully\n";'
	@echo ""
	@echo "Running test suite under valgrind..."
	@for test in tests/*.phpt; do \
		echo "Testing: $$test"; \
		USE_ZEND_ALLOC=0 valgrind --leak-check=full --error-exitcode=1 -q \
			php -d extension=modules/tui.so \
			$$(grep -A1000 '^--FILE--$$' $$test | grep -B1000 '^--EXPECT' | tail -n +2 | head -n -1) \
			2>/dev/null || echo "  LEAK DETECTED in $$test"; \
	done

# Run a single test under valgrind with verbose output
test-valgrind-single:
	@if [ -z "$(TEST)" ]; then \
		echo "Usage: make test-valgrind-single TEST=tests/001-terminal-size.phpt"; \
		exit 1; \
	fi
	@command -v valgrind >/dev/null 2>&1 || { echo "Error: valgrind not installed"; exit 1; }
	USE_ZEND_ALLOC=0 valgrind --leak-check=full --show-leak-kinds=all \
		--track-origins=yes --error-exitcode=1 \
		php -d extension=modules/tui.so $(TEST)

# AddressSanitizer build and test (works on macOS ARM64)
# Requires: rebuild with ASAN enabled
test-asan:
	@echo "Building with AddressSanitizer..."
	@echo "Note: Requires clean rebuild with ASAN flags"
	@echo ""
	@echo "To use ASAN, rebuild with:"
	@echo "  export CFLAGS='-fsanitize=address -g'"
	@echo "  export LDFLAGS='-fsanitize=address'"
	@echo "  make clean && make"
	@echo "  make test"
	@echo ""
	@echo "Any memory errors will be reported during test execution."

# Quick memory check with a simple script
test-memory-quick:
	@echo "Quick memory check..."
	USE_ZEND_ALLOC=0 php -d extension=modules/tui.so -r ' \
		use Xocdr\Tui\Ext\Box; \
		use Xocdr\Tui\Ext\Text; \
		for ($$i = 0; $$i < 100; $$i++) { \
			$$r = tui_test_create(80, 24); \
			$$tree = new Box(["children" => [ \
				new Text("Test $$i"), \
				new Box(["children" => [new Text("Nested")]]) \
			]]); \
			tui_test_render($$r, $$tree); \
			tui_test_destroy($$r); \
		} \
		echo "100 render cycles completed\n"; \
		echo "Peak memory: " . round(memory_get_peak_usage(true) / 1024 / 1024, 2) . " MB\n"; \
	'

.PHONY: coverage coverage-clean coverage-summary lint lint-errors \
        test-valgrind test-valgrind-single test-asan test-memory-quick
