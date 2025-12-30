/*
 * Fuzz harness for UTF-8 decoding and width calculation.
 *
 * This harness tests:
 * - tui_utf8_decode_n() - UTF-8 decoding with bounds checking
 * - tui_char_width() - Character width calculation
 * - tui_string_width_n() - String width calculation
 *
 * Build with libFuzzer (requires clang):
 *   clang -g -O1 -fno-omit-frame-pointer -fsanitize=fuzzer,address \
 *         -I.. -I../src fuzz_utf8.c ../src/text/measure.c -o fuzz_utf8
 *
 * Build standalone (for manual testing):
 *   cc -g -O0 -fsanitize=address -DSTANDALONE \
 *      -I.. -I../src fuzz_utf8.c ../src/text/measure.c -o fuzz_utf8_standalone
 *
 * Run standalone:
 *   ./fuzz_utf8_standalone <corpus_dir>
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "text/measure.h"

/* Test UTF-8 decoding doesn't crash or read out of bounds */
static void test_utf8_decode(const uint8_t *data, size_t size)
{
    if (size == 0) return;

    uint32_t codepoint;
    size_t pos = 0;

    while (pos < size) {
        int bytes = tui_utf8_decode_n((const char *)data + pos,
                                       (int)(size - pos), &codepoint);
        if (bytes <= 0) break;
        pos += (size_t)bytes;
    }
}

/* Test string width calculation doesn't crash */
static void test_string_width(const uint8_t *data, size_t size)
{
    if (size == 0) return;

    /* Test with exact size */
    (void)tui_string_width_n((const char *)data, (int)size);

    /* Test with null-terminated copy */
    if (size < 4096) {
        char *copy = malloc(size + 1);
        if (copy) {
            memcpy(copy, data, size);
            copy[size] = '\0';
            (void)tui_string_width(copy);
            free(copy);
        }
    }
}

/* Test char width for all possible bytes */
static void test_char_width(const uint8_t *data, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        /* Treat each byte as a potential codepoint hint */
        uint32_t cp = data[i];
        if (i + 1 < size) {
            cp = (cp << 8) | data[i + 1];
        }
        if (i + 2 < size) {
            cp = (cp << 8) | data[i + 2];
        }
        /* Limit to valid Unicode range */
        cp &= 0x1FFFFF;
        if (cp <= 0x10FFFF) {
            (void)tui_char_width(cp);
        }
    }
}

/* libFuzzer entry point */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    test_utf8_decode(data, size);
    test_string_width(data, size);
    test_char_width(data, size);
    return 0;
}

#ifdef STANDALONE
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

static void fuzz_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size > 0 && size < 1024 * 1024) {
        uint8_t *data = malloc((size_t)size);
        if (data) {
            if (fread(data, 1, (size_t)size, f) == (size_t)size) {
                LLVMFuzzerTestOneInput(data, (size_t)size);
            }
            free(data);
        }
    }
    fclose(f);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <corpus_dir>\n", argv[0]);
        fprintf(stderr, "       %s <input_file>\n", argv[0]);
        return 1;
    }

    struct stat st;
    if (stat(argv[1], &st) != 0) {
        perror(argv[1]);
        return 1;
    }

    if (S_ISDIR(st.st_mode)) {
        DIR *d = opendir(argv[1]);
        if (!d) {
            perror(argv[1]);
            return 1;
        }

        struct dirent *ent;
        int count = 0;
        while ((ent = readdir(d)) != NULL) {
            if (ent->d_name[0] == '.') continue;

            char path[4096];
            snprintf(path, sizeof(path), "%s/%s", argv[1], ent->d_name);
            fuzz_file(path);
            count++;
        }
        closedir(d);
        printf("Processed %d corpus files without crash\n", count);
    } else {
        fuzz_file(argv[1]);
        printf("Processed file without crash\n");
    }

    return 0;
}
#endif
