/*
  +----------------------------------------------------------------------+
  | ext-tui: Table layout and rendering                                 |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_TABLE_H
#define TUI_TABLE_H

#include "../render/buffer.h"
#include "../node/node.h"

/**
 * Table structure.
 */
typedef struct {
    char **headers;             /* Array of header strings */
    int header_count;           /* Number of columns */
    char ***rows;               /* Array of row arrays */
    int row_count;              /* Number of rows */
    int *column_widths;         /* Calculated column widths */
    int *column_align_right;    /* Right-align flags per column */
} tui_table;

/**
 * Create a table.
 *
 * @param headers Array of header strings
 * @param header_count Number of headers/columns
 * @return New table structure
 */
tui_table* tui_table_create(const char **headers, int header_count);

/**
 * Free a table.
 */
void tui_table_free(tui_table *table);

/**
 * Add a row to the table.
 *
 * @param table Table to modify
 * @param cells Array of cell strings (must match header_count)
 * @return 0 on success, -1 on error
 */
int tui_table_add_row(tui_table *table, const char **cells);

/**
 * Set column alignment.
 *
 * @param table Table to modify
 * @param column Column index
 * @param right_align 1 for right-align, 0 for left-align
 */
void tui_table_set_align(tui_table *table, int column, int right_align);

/**
 * Calculate optimal column widths.
 *
 * @param table Table to calculate
 * @param max_width Maximum total width (including borders)
 * @param output Array to store calculated widths (must be header_count size)
 */
void tui_table_layout(tui_table *table, int max_width, int *output);

/**
 * Render table to buffer.
 *
 * @param buf Buffer to render to
 * @param table Table to render
 * @param x X position
 * @param y Y position
 * @param border_style Border style
 * @param header_style Style for header row
 * @param cell_style Style for data cells
 * @return Height of rendered table
 */
int tui_table_render(tui_buffer *buf, tui_table *table, int x, int y,
                     tui_border_style border_style,
                     const tui_style *header_style,
                     const tui_style *cell_style);

/**
 * Calculate layout and return column widths as array.
 *
 * @param headers Array of header strings
 * @param header_count Number of columns
 * @param rows Array of row arrays
 * @param row_count Number of rows
 * @param max_width Maximum total width
 * @param output Pre-allocated array for output widths
 */
void tui_table_calculate_widths(const char **headers, int header_count,
                                const char ***rows, int row_count,
                                int max_width, int *output);

#endif /* TUI_TABLE_H */
