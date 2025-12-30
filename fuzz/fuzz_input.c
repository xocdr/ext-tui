/*
 * Fuzz harness for terminal input parsing.
 *
 * This harness tests:
 * - Escape sequence parsing
 * - Keyboard input decoding
 * - Mouse event parsing
 *
 * Build with libFuzzer (requires clang):
 *   clang -g -O1 -fno-omit-frame-pointer -fsanitize=fuzzer,address \
 *         -I.. -I../src fuzz_input.c ../src/event/input.c -o fuzz_input
 *
 * Build standalone:
 *   cc -g -O0 -fsanitize=address -DSTANDALONE \
 *      -I.. -I../src fuzz_input.c ../src/event/input.c -o fuzz_input_standalone
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "event/input.h"

/* Test keyboard input parsing */
static void test_parse_key(const uint8_t *data, size_t size)
{
    if (size == 0) return;

    tui_key_event key;
    memset(&key, 0, sizeof(key));

    /* Parse the input as potential key sequence */
    (void)tui_input_parse((const char *)data, (int)size, &key);
}

/* Test mouse sequence parsing (SGR format: ESC[<...M or ESC[<...m) */
static void test_parse_mouse(const uint8_t *data, size_t size)
{
    if (size < 6) return; /* Minimum: ESC[<0;0;0M */

    /* Create a buffer that looks like a mouse sequence */
    char buf[256];
    size_t len = size > 250 ? 250 : size;
    memcpy(buf, data, len);
    buf[len] = '\0';

    /* Prepend escape if not present */
    if (buf[0] != '\033') {
        memmove(buf + 3, buf, len);
        buf[0] = '\033';
        buf[1] = '[';
        buf[2] = '<';
        len += 3;
    }

    tui_mouse_event mouse;
    memset(&mouse, 0, sizeof(mouse));

    /* Try to parse - should not crash */
    (void)tui_input_parse_mouse(buf, (int)len, &mouse);
}

/* Test input history operations */
static void test_history(const uint8_t *data, size_t size)
{
    if (size < 2) return;

    /* Use first byte as operation count */
    int ops = data[0] % 10 + 1;
    size_t pos = 1;

    tui_input_history *history = tui_history_create(20);
    if (!history) return;

    for (int i = 0; i < ops && pos < size; i++) {
        int op = data[pos++] % 6;
        switch (op) {
            case 0: /* Add entry */
                if (pos + 1 < size) {
                    int entry_len = data[pos++] % 50 + 1;
                    if (pos + entry_len <= size) {
                        char *entry = malloc((size_t)entry_len + 1);
                        if (entry) {
                            memcpy(entry, data + pos, (size_t)entry_len);
                            entry[entry_len] = '\0';
                            tui_history_add(history, entry);
                            free(entry);
                        }
                        pos += (size_t)entry_len;
                    }
                }
                break;
            case 1: /* Previous */
                (void)tui_history_prev(history);
                break;
            case 2: /* Next */
                (void)tui_history_next(history);
                break;
            case 3: /* Reset */
                tui_history_reset_position(history);
                break;
            case 4: /* Save temp */
                if (pos + 1 < size) {
                    int temp_len = data[pos++] % 30 + 1;
                    if (pos + temp_len <= size) {
                        char *temp = malloc((size_t)temp_len + 1);
                        if (temp) {
                            memcpy(temp, data + pos, (size_t)temp_len);
                            temp[temp_len] = '\0';
                            tui_history_save_temp(history, temp);
                            free(temp);
                        }
                        pos += (size_t)temp_len;
                    }
                }
                break;
            case 5: /* Get temp */
                (void)tui_history_get_temp(history);
                break;
        }
    }

    tui_history_destroy(history);
}

/* libFuzzer entry point */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    test_parse_key(data, size);
    test_parse_mouse(data, size);
    test_history(data, size);
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
