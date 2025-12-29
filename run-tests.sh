#!/bin/bash
# Run ext-tui tests with results saved to tmp/

mkdir -p tmp

# Run tests with output going to tmp/
TEST_PHP_ARGS="-q" \
NO_INTERACTION=1 \
make test 2>&1 | tee tmp/test-output.txt

# Move any generated result files to tmp/
mv php_test_results_*.txt tmp/ 2>/dev/null || true

echo ""
echo "Test output saved to tmp/test-output.txt"
