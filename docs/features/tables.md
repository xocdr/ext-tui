# Tables

[← Animation](animation.md) | [Back to Documentation](../README.md) | [Next: Progress →](progress.md)

---

Table functions help you display structured data in formatted, aligned tables with optional borders and styling.

## Overview

Tables provide:
- **Automatic column sizing**: Based on content width
- **Unicode borders**: Clean box-drawing characters
- **Header styling**: Distinct header row appearance
- **Alignment options**: Left, right, or center per column
- **Truncation**: Handle long content gracefully

## Functions

### tui_table_create()

Create a new table.

```php
resource tui_table_create(array $headers, array $rows, array $options = [])
```

**Parameters**:
- `$headers`: Array of column header strings
- `$rows`: Array of row arrays (each row is an array of cell strings)
- `$options`: Configuration options (see below)

**Options**:
```php
$options = [
    'border' => true,           // Show borders
    'header_style' => [...],    // Style for header row
    'row_style' => [...],       // Style for data rows
    'align' => ['left', 'right', 'center'],  // Per-column alignment
];
```

**Returns**: Table resource

**Example**:
```php
$headers = ['Name', 'Age', 'City'];
$rows = [
    ['Alice', '30', 'New York'],
    ['Bob', '25', 'London'],
    ['Charlie', '35', 'Tokyo'],
];

$table = tui_table_create($headers, $rows, [
    'border' => true,
    'header_style' => ['fg' => [255, 255, 0], 'bold' => true],
]);
```

---

### tui_table_render()

Render a table to an array of strings.

```php
array tui_table_render(resource $table, int $max_width = 0)
```

**Parameters**:
- `$table`: Table resource
- `$max_width`: Maximum width (0 = no limit)

**Returns**: Array of strings, one per row

**Example**:
```php
$lines = tui_table_render($table, 60);

// Display directly
foreach ($lines as $line) {
    echo $line . "\n";
}

// Or render to buffer
foreach ($lines as $i => $line) {
    tui_buffer_write($buf, 2, 5 + $i, $line, $style);
}
```

---

### tui_table_set_column_width()

Set a specific column's width.

```php
void tui_table_set_column_width(resource $table, int $column, int $width)
```

**Parameters**:
- `$table`: Table resource
- `$column`: Column index (0-based)
- `$width`: Column width in characters

**Example**:
```php
// Fixed-width columns
tui_table_set_column_width($table, 0, 20);  // Name column
tui_table_set_column_width($table, 1, 5);   // Age column
tui_table_set_column_width($table, 2, 15);  // City column
```

---

### tui_table_free()

Free a table resource.

```php
void tui_table_free(resource $table)
```

---

## Practical Patterns

### Basic Table Display

```php
<?php
$headers = ['Product', 'Price', 'Stock'];
$rows = [
    ['Widget A', '$19.99', '150'],
    ['Widget B', '$24.99', '75'],
    ['Gadget X', '$49.99', '30'],
    ['Gadget Y', '$99.99', '12'],
];

$table = tui_table_create($headers, $rows, [
    'border' => true,
    'header_style' => ['fg' => [100, 200, 255], 'bold' => true],
    'align' => ['left', 'right', 'right'],
]);

$lines = tui_table_render($table, 50);

foreach ($lines as $line) {
    echo $line . "\n";
}

tui_table_free($table);
```

Output:
```
┌─────────────────────┬──────────┬───────┐
│ Product             │    Price │ Stock │
├─────────────────────┼──────────┼───────┤
│ Widget A            │   $19.99 │   150 │
│ Widget B            │   $24.99 │    75 │
│ Gadget X            │   $49.99 │    30 │
│ Gadget Y            │   $99.99 │    12 │
└─────────────────────┴──────────┴───────┘
```

### Alternating Row Colors

```php
function render_striped_table(array $headers, array $rows, int $x, int $y): void
{
    global $buf;

    $even_style = ['fg' => [200, 200, 200], 'bg' => [40, 40, 50]];
    $odd_style = ['fg' => [200, 200, 200], 'bg' => [50, 50, 60]];
    $header_style = ['fg' => [255, 255, 100], 'bg' => [60, 60, 80], 'bold' => true];

    // Render header
    $table = tui_table_create($headers, [], ['border' => false, 'header_style' => $header_style]);
    $header_lines = tui_table_render($table, 60);
    tui_buffer_write($buf, $x, $y, $header_lines[0], $header_style);
    tui_table_free($table);

    // Render rows with alternating colors
    foreach ($rows as $i => $row) {
        $style = ($i % 2 === 0) ? $even_style : $odd_style;
        $table = tui_table_create($headers, [$row], ['border' => false]);
        $lines = tui_table_render($table, 60);
        // Skip header, get data row
        if (isset($lines[1])) {
            tui_buffer_write($buf, $x, $y + 1 + $i, $lines[1], $style);
        }
        tui_table_free($table);
    }
}
```

### Scrollable Table

```php
class ScrollableTable
{
    private array $headers;
    private array $rows;
    private int $scroll_offset = 0;
    private int $visible_rows;
    private int $selected = 0;

    public function __construct(array $headers, array $rows, int $visible_rows)
    {
        $this->headers = $headers;
        $this->rows = $rows;
        $this->visible_rows = $visible_rows;
    }

    public function moveUp(): void
    {
        if ($this->selected > 0) {
            $this->selected--;
            if ($this->selected < $this->scroll_offset) {
                $this->scroll_offset = $this->selected;
            }
        }
    }

    public function moveDown(): void
    {
        if ($this->selected < count($this->rows) - 1) {
            $this->selected++;
            if ($this->selected >= $this->scroll_offset + $this->visible_rows) {
                $this->scroll_offset = $this->selected - $this->visible_rows + 1;
            }
        }
    }

    public function render($buf, int $x, int $y, int $width): void
    {
        $header_style = ['fg' => [255, 200, 0], 'bold' => true];
        $normal_style = ['fg' => [180, 180, 180]];
        $selected_style = ['fg' => [0, 0, 0], 'bg' => [100, 150, 255]];

        // Get visible rows
        $visible = array_slice($this->rows, $this->scroll_offset, $this->visible_rows);

        // Create and render table
        $table = tui_table_create($this->headers, $visible, [
            'border' => true,
            'header_style' => $header_style,
        ]);

        $lines = tui_table_render($table, $width);

        foreach ($lines as $i => $line) {
            // Determine if this is the selected row
            $row_index = $i - 2;  // Account for header and border
            $is_selected = ($row_index >= 0 &&
                           $row_index + $this->scroll_offset === $this->selected);

            $style = $is_selected ? $selected_style : $normal_style;
            tui_buffer_write($buf, $x, $y + $i, $line, $style);
        }

        tui_table_free($table);

        // Scroll indicator
        if (count($this->rows) > $this->visible_rows) {
            $indicator = sprintf(" [%d-%d of %d] ",
                $this->scroll_offset + 1,
                min($this->scroll_offset + $this->visible_rows, count($this->rows)),
                count($this->rows)
            );
            tui_buffer_write($buf, $x + $width - strlen($indicator) - 2, $y, $indicator, $header_style);
        }
    }
}
```

### Data Formatting Helpers

```php
function format_table_data(array $raw_data): array
{
    return array_map(function($row) {
        return [
            $row['name'],
            format_currency($row['price']),
            format_number($row['quantity']),
            format_percentage($row['change']),
            format_date($row['date']),
        ];
    }, $raw_data);
}

function format_currency(float $value): string
{
    return '$' . number_format($value, 2);
}

function format_number(int $value): string
{
    return number_format($value);
}

function format_percentage(float $value): string
{
    $sign = $value >= 0 ? '+' : '';
    return $sign . number_format($value * 100, 1) . '%';
}

function format_date(int $timestamp): string
{
    return date('Y-m-d', $timestamp);
}
```

### Responsive Table

```php
function responsive_table($buf, array $headers, array $rows, int $available_width): void
{
    // Determine which columns fit
    $min_column_width = 8;
    $max_columns = (int)($available_width / $min_column_width);
    $columns_to_show = min(count($headers), $max_columns);

    // Truncate headers and rows
    $display_headers = array_slice($headers, 0, $columns_to_show);
    $display_rows = array_map(function($row) use ($columns_to_show) {
        return array_slice($row, 0, $columns_to_show);
    }, $rows);

    $table = tui_table_create($display_headers, $display_rows, ['border' => true]);
    $lines = tui_table_render($table, $available_width);

    foreach ($lines as $i => $line) {
        tui_buffer_write($buf, 0, $i, $line, []);
    }

    if ($columns_to_show < count($headers)) {
        $msg = sprintf("(%d more columns hidden)", count($headers) - $columns_to_show);
        tui_buffer_write($buf, 0, count($lines), $msg, ['dim' => true]);
    }

    tui_table_free($table);
}
```

## Table Styles Reference

### Border Characters

```
Default (Unicode):
┌───┬───┐
│   │   │
├───┼───┤
│   │   │
└───┴───┘

Simple (ASCII):
+---+---+
|   |   |
+---+---+
|   |   |
+---+---+
```

### Alignment Examples

```
Left (default):    Right:           Center:
│ Text        │    │        Text │  │    Text    │
│ Longer text │    │ Longer text │  │ Longer text│
```

## Related Functions

- [tui_string_width()](../api/text.md#tui_string_width) - Measure text width
- [tui_truncate()](../api/text.md#tui_truncate) - Truncate long content
- [tui_pad()](../api/text.md#tui_pad) - Pad text for alignment

## See Also

- [Text Utilities](text.md) - Text formatting
- [Buffers](buffers.md) - Rendering tables to buffers
- [API Reference: Tables](../api/tables.md) - Complete API details

---

[← Animation](animation.md) | [Back to Documentation](../README.md) | [Next: Progress →](progress.md)
