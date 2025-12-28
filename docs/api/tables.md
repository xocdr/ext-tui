# API Reference: Table Functions

[Back to Documentation](../README.md) | [Feature Guide](../features/tables.md)

---

## tui_table_create

Create a new table.

```php
resource tui_table_create(array $headers, array $options = [])
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$headers` | array | Column header strings |
| `$options` | array | Configuration options |

### Options

| Option | Type | Description |
|--------|------|-------------|
| `border` | bool | Show borders (default: true) |
| `header_style` | array | Style for header row |

### Return Value

Returns a table resource on success.

### Example

```php
$headers = ['Name', 'Price', 'Qty'];

$table = tui_table_create($headers, [
    'border' => true,
    'header_style' => ['fg' => [255, 255, 0], 'bold' => true],
]);
```

### Related

- [tui_table_add_row()](#tui_table_add_row) - Add data rows
- [tui_table_render_to_buffer()](#tui_table_render_to_buffer) - Render table

---

## tui_table_add_row

Add a data row to a table.

```php
void tui_table_add_row(resource $table, array $row)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$table` | resource | Table resource |
| `$row` | array | Array of cell values |

### Example

```php
$table = tui_table_create(['Name', 'Price', 'Qty']);

tui_table_add_row($table, ['Widget', '$19.99', '100']);
tui_table_add_row($table, ['Gadget', '$49.99', '50']);
tui_table_add_row($table, ['Tool', '$29.99', '75']);
```

---

## tui_table_set_align

Set column alignment.

```php
void tui_table_set_align(resource $table, int $column, string $align)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$table` | resource | Table resource |
| `$column` | int | Column index (0-based) |
| `$align` | string | Alignment: "left", "right", or "center" |

### Example

```php
// Right-align price and quantity columns
tui_table_set_align($table, 1, 'right');  // Price
tui_table_set_align($table, 2, 'right');  // Qty
```

---

## tui_table_render_to_buffer

Render a table to a buffer.

```php
void tui_table_render_to_buffer(resource $table, resource $buffer, int $x, int $y, array $style = [])
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$table` | resource | Table resource |
| `$buffer` | resource | Target buffer |
| `$x` | int | X position |
| `$y` | int | Y position |
| `$style` | array | Optional style array |

### Example

```php
$table = tui_table_create(['Product', 'Price', 'Stock']);
tui_table_add_row($table, ['Widget', '$19.99', '100']);
tui_table_add_row($table, ['Gadget', '$49.99', '50']);

tui_table_set_align($table, 1, 'right');
tui_table_set_align($table, 2, 'right');

tui_table_render_to_buffer($table, $buf, 5, 3, []);
tui_buffer_render($buf);
```

### Output Example

```
┌───────────┬──────────┬─────┐
│ Product   │    Price │ Stock │
├───────────┼──────────┼─────┤
│ Widget    │   $19.99 │ 100 │
│ Gadget    │   $49.99 │  50 │
└───────────┴──────────┴─────┘
```

---

## Complete Example

```php
<?php
[$width, $height] = tui_get_terminal_size();
$buf = tui_buffer_create($width, $height);

// Create table
$table = tui_table_create(['Product', 'Category', 'Price', 'Stock'], [
    'border' => true,
    'header_style' => ['fg' => [100, 200, 255], 'bold' => true],
]);

// Add rows
$data = [
    ['Laptop Pro 15"', 'Electronics', '$1,299.00', '25'],
    ['Wireless Mouse', 'Electronics', '$29.99', '150'],
    ['USB-C Cable 2m', 'Accessories', '$12.99', '500'],
    ['Monitor Stand', 'Furniture', '$89.99', '45'],
];

foreach ($data as $row) {
    tui_table_add_row($table, $row);
}

// Set alignments
tui_table_set_align($table, 2, 'right');  // Price
tui_table_set_align($table, 3, 'right');  // Stock

// Render
tui_buffer_clear($buf);
tui_buffer_write($buf, 2, 1, "Inventory", ['bold' => true]);
tui_table_render_to_buffer($table, $buf, 2, 3, []);
tui_buffer_render($buf);

sleep(3);
```

---

[Back to Documentation](../README.md) | [Feature Guide](../features/tables.md)
