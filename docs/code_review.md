# ext-tui Code Quality Review

**Date:** January 1, 2026
**Version:** 0.3.1
**Reviewer:** Claude Code

## Executive Summary

The ext-tui codebase demonstrates solid C extension engineering with comprehensive memory management, good API design, and strong testing (114 tests). The main areas for improvement are input validation (security) and filling testing gaps (fuzzing, security tests).

**Overall Grade: A-**

---

## Detailed Assessment

### 1. Code Organization

**Grade: A-**

**Strengths:**
- Well-organized modular structure with clear separation of concerns
- Clear directory hierarchy: terminal/, event/, node/, render/, text/, pool/, app/
- Good header/implementation file separation
- Comprehensive documentation in headers (memory management, thread safety, error handling)
- config.m4 well-structured with feature detection and strict compiler warnings

**Issues:**
- `tui.c` is large (4,780 lines) - could be split into separate files for classes/colors
- Named colors lookup table (lines 264-800+) could be generated from data file

**Recommendations:**
- Split tui.c into: tui_core.c, tui_classes.c, tui_colors.c
- Consider generating color lookup from data file

---

### 2. Memory Management

**Grade: A**

**Strengths:**
- Excellent hybrid allocation strategy with object pools (children arrays, string interning)
- Careful overflow checking before allocations
- NULL-safe functions throughout
- Proper cleanup patterns in error paths
- Pool allocation with size classes (4, 8, 16, 32) for children arrays
- String interning with FNV-1a hash, collision chaining, reference counting
- Buffer management with integer overflow checks

**Issues:**
- strdup() usage without bounds checking on input length in some paths
- Missing cleanup in some error paths (app.c buffer creation)

**Recommendations:**
- Add length validation before strdup operations
- Consider pooling text allocations (high churn item)
- Use goto error path cleanup pattern for multi-step initialization

---

### 3. Error Handling

**Grade: B+**

**Strengths:**
- Consistent error return conventions documented in php_tui.h
- Good error propagation in critical paths
- PHP error reporting (php_error_docref) used appropriately
- Exception class hierarchy (TuiException -> specific types)
- Pool failures logged with exponential backoff

**Issues:**
- Silent failures in app.c (sequential cleanup without rollback)
- Depth limit enforcement silently returns without logging
- Pool exhaustion only warns after 10,000 misses

**Recommendations:**
- Use goto error path cleanup pattern for multi-step initialization
- Add validation for callback functions before storing
- Log depth limit violations
- Reduce pool miss log threshold

---

### 4. Thread Safety (ZTS Compatibility)

**Grade: A**

**Strengths:**
- Good ZTS awareness documented in php_tui.h
- TUI_G() macro properly handles thread-local access
- Module globals correctly initialized via PHP_GINIT
- Signal handlers use sig_atomic_t for async-signal-safe access
- Shutdown ordering documented in pool.h

**Issues:**
- Static globals `pool_miss_log_threshold` and `pool_miss_count` not thread-safe
- Terminal state is process-global (documented, acceptable)

**Recommendations:**
- Make pool statistics thread-local (move to module globals)
- Consider adding ZTS build validation test

---

### 5. Security

**Grade: B**

**Strengths:**
- Buffer overflow protection with bounds checking
- Integer overflow checks before allocation
- Configurable limits via INI (max_buffer_width, max_buffer_height, max_tree_depth)
- String length constants defined (TUI_MAX_KEY_LENGTH, TUI_MAX_ID_LENGTH, TUI_MAX_TEXT_LENGTH)

**Issues (CRITICAL):**
- No length validation before strdup() in node creation
  - `node->text = strdup(text)` - no check against TUI_MAX_TEXT_LENGTH
  - `node->id = strdup(id)` - no check against TUI_MAX_ID_LENGTH
  - `node->hyperlink_url = strdup(url)` - unbounded
  - **Impact:** Allows DoS via huge text/key/id allocation

**Recommendations:**
1. **CRITICAL**: Add length checks before all strdup operations
2. Audit event/input.c for input buffer overflows
3. Add tests for oversized input (fuzzing with 100MB strings)
4. Consider using strndup with length parameter

---

### 6. Performance

**Grade: A-**

**Strengths:**
- Object pools reduce allocation overhead
- Diff-based reconciliation (React-like, O(n))
- Metrics system for profiling (can be compiled out)
- Efficient string interning with FNV-1a hash
- React-like lastPlacedIndex optimization reduces reorder operations
- Double-buffering reduces flicker

**Issues:**
- Pool size class 32 is limit; larger arrays fall back to malloc
- Key map allocated per diff run (not pooled)

**Recommendations:**
- Profile rendering to identify remaining allocations
- Add size class 64, 128 for larger trees
- Benchmark reconciliation with 10k+ node trees

---

### 7. Code Style & Consistency

**Grade: A-**

**Strengths:**
- Consistent naming conventions (tui_ prefix, snake_case)
- Good documentation with detailed comments
- Coding style enforced via strict compiler flags
- Clear error return patterns
- Macros use do-while(0) pattern

**Issues:**
- Inconsistent comment styles (/* */ vs //)
- Some magic numbers without named constants

**Recommendations:**
- Standardize comment style
- Document magic numbers (pool threshold, capacity constants)
- Add style guide to CONTRIBUTING.md

---

### 8. Testing

**Grade: A-**

**Strengths:**
- 114 comprehensive tests using PHP .phpt format
- Tests cover: focus system, text wrapping, rendering, stress tests
- Good test naming convention
- Integration tests included

**Coverage Gaps:**
- No fuzzing tests for large inputs
- No security tests for oversized/malformed strings
- No explicit ZTS build tests
- No valgrind/asan integration visible
- Missing tests for 10k+ node trees, deep nesting

**Recommendations:**
- Add AddressSanitizer/UBSan tests in CI
- Add corpus-based fuzzing (libFuzzer or AFL)
- Create ZTS-specific test suite
- Add valgrind/memcheck integration tests
- Security test suite for large/malformed input

---

### 9. API Design

**Grade: A-**

**Strengths:**
- Clean PHP API with intuitive class names
- Good separation of concerns (Box, Text, Instance, Key)
- Focus system well-designed
- Event handling clear
- TuiNode interface with implementations
- Exception hierarchy (TuiException -> specific types)
- useState-like pattern for React familiarity

**Issues:**
- No fluent builder pattern (less elegant for complex UIs)
- Property vs method inconsistency
- No documented deprecation mechanism
- Instance ownership semantics not documented

**Recommendations:**
- Document property vs method usage guidelines
- Add deprecation mechanism
- Consider builder pattern for complex UI construction
- Document Instance ownership semantics

---

## Summary Scorecard

| Area | Grade | Risk Level |
|------|-------|-----------|
| Code Organization | A- | Low |
| Memory Management | A | Low |
| Error Handling | B+ | Medium |
| Thread Safety | A | Low |
| Security | B | **Medium** |
| Performance | A- | Low |
| Code Style | A- | Low |
| Testing | A- | Low |
| API Design | A- | Low |

---

## Top Priority Recommendations

### Critical (Security)
1. Add length validation before all strdup() calls to prevent DoS attacks

### High Priority
2. Add fuzzing and security tests for oversized/malformed input
3. Fix static pool statistics race condition for ZTS builds

### Medium Priority
4. Improve error handling in app.c initialization (cleanup paths)
5. Add logging for MAX_TREE_DEPTH/MAX_RECONCILE_DEPTH violations

### Low Priority
6. Split tui.c into smaller files (4,780 lines)
7. Add pool size classes 64, 128 for larger trees

---

## Code Metrics

- **Lines of Code:** ~8,314 in core modules (node, app, render, pool)
- **C Files:** 54
- **Header Files:** 92
- **Tests:** 114 .phpt files
- **Test/Code Ratio:** ~1 test per 5-7 LOC (good)

---

## Conclusion

The ext-tui codebase is well-engineered with professional-grade memory management, good modular organization, and comprehensive testing. The critical security issue (unbounded strdup) should be addressed immediately. After that, the codebase would warrant an A grade overall.
