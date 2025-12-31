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

.PHONY: coverage coverage-clean coverage-summary
