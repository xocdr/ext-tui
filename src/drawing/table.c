/*
  +----------------------------------------------------------------------+
  | ext-tui: Table layout and rendering                                 |
  +----------------------------------------------------------------------+
  | Provides table creation, layout calculation, and box-drawing        |
  | character rendering for terminal tables.                            |
  +----------------------------------------------------------------------+
*/

#include "table.h"
#include "primitives.h"
#include "../text/measure.h"
#include <stdlib.h>
#include <string.h>

/**
 * Create a new table with specified headers.
 *
 * @param headers Array of header strings
 * @param header_count Number of headers/columns
 * @return Table structure, or NULL on error
 */
tui_table* tui_table_create(const char **headers, int header_count)
{
    if (!headers || header_count <= 0 || header_count > 1000) {
        return NULL;
    }

    tui_table *table = calloc(1, sizeof(tui_table));
    if (!table) return NULL;

    table->header_count = header_count;
    table->headers = calloc((size_t)header_count, sizeof(char*));
    table->column_widths = calloc((size_t)header_count, sizeof(int));
    table->column_align_right = calloc((size_t)header_count, sizeof(int));

    if (!table->headers || !table->column_widths || !table->column_align_right) {
        tui_table_free(table);
        return NULL;
    }

    /* Copy headers */
    for (int i = 0; i < header_count; i++) {
        const char *src = headers[i] ? headers[i] : "";
        table->headers[i] = strdup(src);
        if (!table->headers[i]) {
            tui_table_free(table);
            return NULL;
        }
    }

    return table;
}

/**
 * Free table and all associated memory.
 */
void tui_table_free(tui_table *table)
{
    if (!table) return;

    /* Free headers */
    if (table->headers) {
        for (int i = 0; i < table->header_count; i++) {
            free(table->headers[i]);
        }
        free(table->headers);
    }

    /* Free rows */
    if (table->rows) {
        for (int r = 0; r < table->row_count; r++) {
            if (table->rows[r]) {
                for (int c = 0; c < table->header_count; c++) {
                    free(table->rows[r][c]);
                }
                free(table->rows[r]);
            }
        }
        free(table->rows);
    }

    free(table->column_widths);
    free(table->column_align_right);
    free(table);
}

/**
 * Add a row to the table.
 *
 * @param table Table to modify
 * @param cells Array of cell strings (must match header_count)
 * @return 0 on success, -1 on error
 */
int tui_table_add_row(tui_table *table, const char **cells)
{
    if (!table || !cells) return -1;

    /* Grow rows array */
    size_t new_size = ((size_t)table->row_count + 1) * sizeof(char**);
    char ***new_rows = realloc(table->rows, new_size);
    if (!new_rows) return -1;
    table->rows = new_rows;

    /* Allocate row */
    table->rows[table->row_count] = calloc((size_t)table->header_count, sizeof(char*));
    if (!table->rows[table->row_count]) return -1;

    /* Copy cells */
    for (int i = 0; i < table->header_count; i++) {
        const char *src = cells[i] ? cells[i] : "";
        table->rows[table->row_count][i] = strdup(src);
        if (!table->rows[table->row_count][i]) {
            /* Cleanup partially allocated row */
            for (int j = 0; j < i; j++) {
                free(table->rows[table->row_count][j]);
            }
            free(table->rows[table->row_count]);
            table->rows[table->row_count] = NULL;
            return -1;
        }
    }

    table->row_count++;
    return 0;
}

/**
 * Set column alignment.
 *
 * @param table Table to modify
 * @param column Column index (0-based)
 * @param right_align Non-zero for right alignment
 */
void tui_table_set_align(tui_table *table, int column, int right_align)
{
    if (!table || column < 0 || column >= table->header_count) return;
    if (!table->column_align_right) return;
    table->column_align_right[column] = right_align;
}

/**
 * Calculate column widths to fit content.
 *
 * @param table Table to layout
 * @param max_width Maximum total width
 * @param output Output array for column widths (must be header_count size)
 */
void tui_table_layout(tui_table *table, int max_width, int *output)
{
    if (!table || !output) return;
    if (!table->headers || !table->column_widths) return;

    /* Calculate minimum widths based on content */
    for (int c = 0; c < table->header_count; c++) {
        int max_w = 1;  /* Minimum width of 1 */
        if (table->headers[c]) {
            max_w = tui_string_width(table->headers[c]);
            if (max_w < 1) max_w = 1;
        }

        for (int r = 0; r < table->row_count; r++) {
            if (table->rows && table->rows[r] && table->rows[r][c]) {
                int w = tui_string_width(table->rows[r][c]);
                if (w > max_w) max_w = w;
            }
        }

        output[c] = max_w;
        table->column_widths[c] = max_w;
    }

    /* Calculate total width needed */
    int total = 0;
    for (int c = 0; c < table->header_count; c++) {
        total += output[c];
    }

    /* Add space for borders and padding: │ col │ col │ = (cols + 1) * 1 + cols * 2 */
    int border_overhead = (table->header_count + 1) + (table->header_count * 2);
    total += border_overhead;

    /* If we exceed max_width, shrink proportionally */
    if (total > max_width && max_width > border_overhead) {
        int content_width = max_width - border_overhead;
        int content_total = total - border_overhead;

        /* Avoid division by zero */
        if (content_total > 0) {
            for (int c = 0; c < table->header_count; c++) {
                output[c] = (output[c] * content_width) / content_total;
                if (output[c] < 1) output[c] = 1;
                table->column_widths[c] = output[c];
            }
        }
    }
}

/**
 * Calculate column widths from raw data (static function).
 */
void tui_table_calculate_widths(const char **headers, int header_count,
                                const char ***rows, int row_count,
                                int max_width, int *output)
{
    if (!headers || !output || header_count <= 0) return;

    /* Calculate minimum widths based on content */
    for (int c = 0; c < header_count; c++) {
        int max_w = 1;
        if (headers[c]) {
            max_w = tui_string_width(headers[c]);
            if (max_w < 1) max_w = 1;
        }

        if (rows) {
            for (int r = 0; r < row_count; r++) {
                if (rows[r] && rows[r][c]) {
                    int w = tui_string_width(rows[r][c]);
                    if (w > max_w) max_w = w;
                }
            }
        }

        output[c] = max_w;
    }

    /* Calculate total width needed */
    int total = 0;
    for (int c = 0; c < header_count; c++) {
        total += output[c];
    }

    /* Add space for borders and padding */
    int border_overhead = (header_count + 1) + (header_count * 2);
    total += border_overhead;

    /* Shrink if needed */
    if (total > max_width && max_width > border_overhead) {
        int content_width = max_width - border_overhead;
        int content_total = total - border_overhead;

        /* Avoid division by zero */
        if (content_total > 0) {
            for (int c = 0; c < header_count; c++) {
                output[c] = (output[c] * content_width) / content_total;
                if (output[c] < 1) output[c] = 1;
            }
        }
    }
}

/* Border character codepoints for table (const for safety) */
static const uint32_t tbl_single[] = {
    0x250C, 0x2500, 0x252C, 0x2510,  /* ┌ ─ ┬ ┐ */
    0x2502, 0x2502,                   /* │ │ */
    0x251C, 0x2500, 0x253C, 0x2524,  /* ├ ─ ┼ ┤ */
    0x2514, 0x2500, 0x2534, 0x2518   /* └ ─ ┴ ┘ */
};

static const uint32_t tbl_double[] = {
    0x2554, 0x2550, 0x2566, 0x2557,  /* ╔ ═ ╦ ╗ */
    0x2551, 0x2551,                   /* ║ ║ */
    0x2560, 0x2550, 0x256C, 0x2563,  /* ╠ ═ ╬ ╣ */
    0x255A, 0x2550, 0x2569, 0x255D   /* ╚ ═ ╩ ╝ */
};

static const uint32_t tbl_round[] = {
    0x256D, 0x2500, 0x252C, 0x256E,  /* ╭ ─ ┬ ╮ */
    0x2502, 0x2502,                   /* │ │ */
    0x251C, 0x2500, 0x253C, 0x2524,  /* ├ ─ ┼ ┤ */
    0x2570, 0x2500, 0x2534, 0x256F   /* ╰ ─ ┴ ╯ */
};

static const uint32_t tbl_bold[] = {
    0x250F, 0x2501, 0x2533, 0x2513,  /* ┏ ━ ┳ ┓ */
    0x2503, 0x2503,                   /* ┃ ┃ */
    0x2523, 0x2501, 0x254B, 0x252B,  /* ┣ ━ ╋ ┫ */
    0x2517, 0x2501, 0x253B, 0x251B   /* ┗ ━ ┻ ┛ */
};

static const uint32_t* get_table_chars(tui_border_style style)
{
    switch (style) {
        case TUI_BORDER_DOUBLE: return tbl_double;
        case TUI_BORDER_ROUND: return tbl_round;
        case TUI_BORDER_BOLD: return tbl_bold;
        default: return tbl_single;
    }
}

/**
 * Render table to buffer.
 *
 * @param buf Target buffer
 * @param table Table to render
 * @param x X position
 * @param y Y position
 * @param border_style Border style (single, double, round, bold)
 * @param header_style Style for header row
 * @param cell_style Style for data cells
 * @return Number of rows rendered
 */
int tui_table_render(tui_buffer *buf, tui_table *table, int x, int y,
                     tui_border_style border_style,
                     const tui_style *header_style,
                     const tui_style *cell_style)
{
    if (!buf || !table) return 0;
    if (!table->headers || !table->column_widths) return 0;
    if (table->header_count <= 0) return 0;

    const uint32_t *chars = get_table_chars(border_style);
    int row = y;

    /* Top border: ┌───┬───┐ */
    tui_buffer_set_cell(buf, x, row, chars[0], cell_style);
    int cx = x + 1;
    for (int c = 0; c < table->header_count; c++) {
        int w = table->column_widths[c] + 2;  /* Add padding */
        for (int i = 0; i < w; i++) {
            tui_buffer_set_cell(buf, cx + i, row, chars[1], cell_style);
        }
        cx += w;
        if (c < table->header_count - 1) {
            tui_buffer_set_cell(buf, cx, row, chars[2], cell_style);
        } else {
            tui_buffer_set_cell(buf, cx, row, chars[3], cell_style);
        }
        cx++;
    }
    row++;

    /* Header row: │ Name │ Age │ */
    tui_buffer_set_cell(buf, x, row, chars[4], header_style);
    cx = x + 1;
    for (int c = 0; c < table->header_count; c++) {
        /* Left padding */
        tui_buffer_set_cell(buf, cx, row, ' ', header_style);
        cx++;

        /* Content */
        const char *text = table->headers[c] ? table->headers[c] : "";
        int text_width = tui_string_width(text);
        int col_width = table->column_widths[c];

        if (table->column_align_right && table->column_align_right[c]) {
            /* Right align */
            int pad = col_width - text_width;
            if (pad < 0) pad = 0;
            for (int i = 0; i < pad; i++) {
                tui_buffer_set_cell(buf, cx + i, row, ' ', header_style);
            }
            tui_buffer_write_text(buf, cx + pad, row, text, header_style);
        } else {
            /* Left align */
            tui_buffer_write_text(buf, cx, row, text, header_style);
            int pad = col_width - text_width;
            if (pad < 0) pad = 0;
            for (int i = 0; i < pad; i++) {
                tui_buffer_set_cell(buf, cx + text_width + i, row, ' ', header_style);
            }
        }
        cx += col_width;

        /* Right padding */
        tui_buffer_set_cell(buf, cx, row, ' ', header_style);
        cx++;

        /* Separator */
        tui_buffer_set_cell(buf, cx, row, chars[5], header_style);
        cx++;
    }
    row++;

    /* Separator row: ├───┼───┤ */
    tui_buffer_set_cell(buf, x, row, chars[6], cell_style);
    cx = x + 1;
    for (int c = 0; c < table->header_count; c++) {
        int w = table->column_widths[c] + 2;
        for (int i = 0; i < w; i++) {
            tui_buffer_set_cell(buf, cx + i, row, chars[7], cell_style);
        }
        cx += w;
        if (c < table->header_count - 1) {
            tui_buffer_set_cell(buf, cx, row, chars[8], cell_style);
        } else {
            tui_buffer_set_cell(buf, cx, row, chars[9], cell_style);
        }
        cx++;
    }
    row++;

    /* Data rows */
    for (int r = 0; r < table->row_count; r++) {
        if (!table->rows || !table->rows[r]) continue;

        tui_buffer_set_cell(buf, x, row, chars[4], cell_style);
        cx = x + 1;

        for (int c = 0; c < table->header_count; c++) {
            /* Left padding */
            tui_buffer_set_cell(buf, cx, row, ' ', cell_style);
            cx++;

            /* Content */
            const char *text = table->rows[r][c] ? table->rows[r][c] : "";
            int text_width = tui_string_width(text);
            int col_width = table->column_widths[c];

            if (table->column_align_right && table->column_align_right[c]) {
                /* Right align */
                int pad = col_width - text_width;
                if (pad < 0) pad = 0;
                for (int i = 0; i < pad; i++) {
                    tui_buffer_set_cell(buf, cx + i, row, ' ', cell_style);
                }
                tui_buffer_write_text(buf, cx + pad, row, text, cell_style);
            } else {
                /* Left align */
                tui_buffer_write_text(buf, cx, row, text, cell_style);
                int pad = col_width - text_width;
                if (pad < 0) pad = 0;
                for (int i = 0; i < pad; i++) {
                    tui_buffer_set_cell(buf, cx + text_width + i, row, ' ', cell_style);
                }
            }
            cx += col_width;

            /* Right padding */
            tui_buffer_set_cell(buf, cx, row, ' ', cell_style);
            cx++;

            /* Separator */
            tui_buffer_set_cell(buf, cx, row, chars[5], cell_style);
            cx++;
        }
        row++;
    }

    /* Bottom border: └───┴───┘ */
    tui_buffer_set_cell(buf, x, row, chars[10], cell_style);
    cx = x + 1;
    for (int c = 0; c < table->header_count; c++) {
        int w = table->column_widths[c] + 2;
        for (int i = 0; i < w; i++) {
            tui_buffer_set_cell(buf, cx + i, row, chars[11], cell_style);
        }
        cx += w;
        if (c < table->header_count - 1) {
            tui_buffer_set_cell(buf, cx, row, chars[12], cell_style);
        } else {
            tui_buffer_set_cell(buf, cx, row, chars[13], cell_style);
        }
        cx++;
    }
    row++;

    return row - y;
}
