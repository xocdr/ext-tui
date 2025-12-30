# Fuzz Testing for ext-tui

This directory contains fuzz harnesses for testing ext-tui's C code against random/malformed input.

## Fuzz Targets

| Harness | Tests |
|---------|-------|
| `fuzz_utf8.c` | UTF-8 decoding, character width, string width |
| `fuzz_input.c` | Keyboard parsing, mouse events, input history |

## Quick Start (Standalone Mode)

Build and run without libFuzzer (uses AddressSanitizer):

```bash
cd fuzz

# Build UTF-8 fuzzer
cc -g -O0 -fsanitize=address -DSTANDALONE \
   -I.. -I../src fuzz_utf8.c ../src/text/measure.c -o fuzz_utf8_standalone

# Build input fuzzer (needs measure.c for UTF-8 functions)
cc -g -O0 -fsanitize=address -DSTANDALONE \
   -I.. -I../src fuzz_input.c ../src/event/input.c ../src/text/measure.c \
   -o fuzz_input_standalone

# Create corpus directory with seed inputs
mkdir -p corpus
echo -n "Hello" > corpus/ascii
echo -n "你好世界" > corpus/cjk
printf '\033[31m' > corpus/ansi
printf '\033[<0;10;20M' > corpus/mouse

# Run fuzzers
./fuzz_utf8_standalone corpus/
./fuzz_input_standalone corpus/
```

## Using libFuzzer (Recommended)

libFuzzer provides better coverage and crash detection:

```bash
# Requires clang
clang -g -O1 -fno-omit-frame-pointer \
      -fsanitize=fuzzer,address \
      -I.. -I../src \
      fuzz_utf8.c ../src/text/measure.c \
      -o fuzz_utf8

# Run for 60 seconds
./fuzz_utf8 -max_total_time=60 corpus/

# Run until crash found
./fuzz_utf8 corpus/
```

## Using AFL++

```bash
# Build with AFL instrumentation
afl-cc -g -O2 -DSTANDALONE \
       -I.. -I../src \
       fuzz_utf8.c ../src/text/measure.c \
       -o fuzz_utf8_afl

# Run AFL
afl-fuzz -i corpus/ -o findings/ -- ./fuzz_utf8_afl @@
```

## Corpus Seeds

Good seed inputs for the corpus:

1. **ASCII**: Plain ASCII text
2. **CJK**: Chinese/Japanese/Korean characters
3. **Emoji**: Various emoji with modifiers and ZWJ
4. **ANSI**: Escape sequences (CSI, OSC)
5. **Mouse**: SGR mouse events
6. **Invalid UTF-8**: Truncated/overlong sequences
7. **Binary**: Random bytes

## Reproducing Crashes

When a crash is found, reproduce it:

```bash
# With ASAN for detailed trace
./fuzz_utf8_standalone crash-file

# With gdb
gdb ./fuzz_utf8_standalone
(gdb) run crash-file
```

## Integration with CI

The `../scripts/fuzz-test.sh` script runs basic fuzzing using the PHP extension directly. For deeper testing, use these C harnesses.
