# Text Utilities

[← Terminal](terminal.md) | [Back to Documentation](../README.md) | [Next: Buffers →](buffers.md)

---

Text utilities help you measure, wrap, truncate, and pad text correctly, with full Unicode support including CJK characters and emoji.

## Overview

Terminal UIs need precise text handling because:
- Characters have different display widths (ASCII=1, CJK=2, emoji=2)
- Text needs to wrap at word boundaries
- Long strings need truncation with ellipsis
- Text needs padding for alignment

## Functions

### tui_string_width()

Calculate the display width of a string in terminal columns.

```php
int tui_string_width(string $text)
```

**Parameters**:
- `$text`: The string to measure

**Returns**: Display width in columns (not byte length or character count)

**Example**:
```php
// ASCII - each character is 1 column
echo tui_string_width("Hello");     // 5

// CJK - each character is 2 columns
echo tui_string_width("世界");       // 4 (2 chars × 2 width)

// Mixed content
echo tui_string_width("Hello 世界"); // 11 (5 + 1 + 4 + 1)

// Emoji - typically 2 columns
echo tui_string_width("🎉");         // 2

// Combining characters don't add width
echo tui_string_width("é");          // 1 (e + combining acute)
```

**Why not `strlen()` or `mb_strlen()`?**
```php
$text = "Hello 世界";
echo strlen($text);          // 12 (bytes)
echo mb_strlen($text);       // 8 (characters)
echo tui_string_width($text); // 10 (display columns) ✓
```

---

### tui_wrap_text()

Wrap text to fit within a specified width.

```php
array tui_wrap_text(string $text, int $width, int $mode = TUI_WRAP_WORD)
```

**Parameters**:
- `$text`: The text to wrap
- `$width`: Maximum width per line in columns
- `$mode`: Wrap mode (see below)

**Wrap Modes**:
- `TUI_WRAP_NONE` (0): No wrapping, return single line
- `TUI_WRAP_WORD` (1): Wrap at word boundaries (default)
- `TUI_WRAP_CHAR` (2): Wrap at any character
- `TUI_WRAP_WORD_CHAR` (3): Try word boundaries, fall back to character

**Returns**: Array of wrapped lines

**Example**:
```php
$text = "The quick brown fox jumps over the lazy dog.";

// Word wrapping (default)
$lines = tui_wrap_text($text, 20, TUI_WRAP_WORD);
// [
//   "The quick brown fox",
//   "jumps over the lazy",
//   "dog."
// ]

// Character wrapping
$lines = tui_wrap_text($text, 20, TUI_WRAP_CHAR);
// [
//   "The quick brown fox ",
//   "jumps over the lazy ",
//   "dog."
// ]

// Word with character fallback (for long words)
$text = "Supercalifragilisticexpialidocious is a word";
$lines = tui_wrap_text($text, 15, TUI_WRAP_WORD_CHAR);
// [
//   "Supercalifragil",
//   "isticexpialidoc",
//   "ious is a word"
// ]
```

**Handling newlines**:
```php
$text = "Line one\nLine two\nLine three";
$lines = tui_wrap_text($text, 20);
// Newlines are preserved as line breaks
// ["Line one", "Line two", "Line three"]
```

---

### tui_truncate()

Truncate text to fit within a width, adding ellipsis.

```php
string tui_truncate(string $text, int $width, ?string $ellipsis = "...")
```

**Parameters**:
- `$text`: The text to truncate
- `$width`: Maximum width including ellipsis
- `$ellipsis`: Ellipsis string (default: "...")

**Returns**: Truncated string with ellipsis if needed

**Example**:
```php
$title = "This is a very long title that won't fit";

// Truncate to 20 columns
echo tui_truncate($title, 20);
// "This is a very lo..."

// Custom ellipsis
echo tui_truncate($title, 20, "…");
// "This is a very long…"

// No truncation needed
echo tui_truncate("Short", 20);
// "Short"

// Extreme case - width smaller than ellipsis
echo tui_truncate($title, 2, "...");
// "..."
```

**With Unicode**:
```php
$text = "Hello 世界 and more text";
echo tui_truncate($text, 15);
// "Hello 世界 a..."  (width-aware truncation)
```

---

### tui_pad()

Pad a string to a specified width.

```php
string tui_pad(string $text, int $width, string $align = "left", string $char = " ")
```

**Parameters**:
- `$text`: The text to pad
- `$width`: Target width
- `$align`: Alignment - "left", "right", or "center"
- `$char`: Padding character (default: space)

**Returns**: Padded string

**Example**:
```php
// Left align (pad right)
echo "[" . tui_pad("Hello", 10, "left") . "]";
// "[Hello     ]"

// Right align (pad left)
echo "[" . tui_pad("Hello", 10, "right") . "]";
// "[     Hello]"

// Center align
echo "[" . tui_pad("Hello", 10, "center") . "]";
// "[  Hello   ]"

// Custom padding character
echo tui_pad("Title", 20, "center", "─");
// "───────Title────────"
```

**Unicode-aware**:
```php
// CJK characters are 2 columns wide
echo "[" . tui_pad("世界", 10, "left") . "]";
// "[世界      ]"  (6 spaces, because 世界 = 4 columns)
```

---

## Practical Patterns

### Building a Table Column

```php
function format_column(string $text, int $width, string $align = "left"): string
{
    $display_width = tui_string_width($text);

    if ($display_width > $width) {
        // Truncate if too long
        return tui_truncate($text, $width);
    } else {
        // Pad if too short
        return tui_pad($text, $width, $align);
    }
}

// Usage
$columns = [
    format_column("Product", 20, "left"),
    format_column("$1,234.56", 12, "right"),
    format_column("In Stock", 10, "center"),
];
echo implode(" │ ", $columns);
```

### Word-Wrapped Text Box

```php
function text_box(
    resource $buf,
    int $x,
    int $y,
    int $width,
    int $height,
    string $text,
    array $style
): void {
    // Draw border
    tui_draw_box($buf, $x, $y, $width, $height, $style);

    // Wrap text to fit inside (accounting for border)
    $inner_width = $width - 2;
    $lines = tui_wrap_text($text, $inner_width, TUI_WRAP_WORD);

    // Display lines (limit to box height)
    $max_lines = $height - 2;
    for ($i = 0; $i < min(count($lines), $max_lines); $i++) {
        tui_buffer_write($buf, $x + 1, $y + 1 + $i, $lines[$i], $style);
    }

    // Show indicator if text was truncated
    if (count($lines) > $max_lines) {
        tui_buffer_write($buf, $x + $width - 4, $y + $height - 1, "...", $style);
    }
}
```

### Progress with Percentage

```php
function progress_with_label(float $progress, int $width): string
{
    $label = sprintf("%3d%%", (int)($progress * 100));
    $bar_width = $width - tui_string_width($label) - 1;

    $filled = (int)($bar_width * $progress);
    $empty = $bar_width - $filled;

    return str_repeat("█", $filled) .
           str_repeat("░", $empty) .
           " " . $label;
}

echo progress_with_label(0.75, 40);
// "████████████████████████████░░░░░░░░░░ 75%"
```

### Centered Title

```php
function centered_title(string $title, int $width, string $line_char = "─"): string
{
    $title_width = tui_string_width($title);
    $padding = $width - $title_width - 4;  // 4 = "─ " + " ─"

    if ($padding <= 0) {
        return tui_truncate($title, $width);
    }

    $left = (int)($padding / 2);
    $right = $padding - $left;

    return str_repeat($line_char, $left) . " " . $title . " " . str_repeat($line_char, $right);
}

echo centered_title("Settings", 40);
// "────────────── Settings ───────────────"
```

## Related Functions

- [tui_buffer_write()](../api/buffers.md#tui_buffer_write) - Write text to buffer
- [tui_table_create()](../api/tables.md#tui_table_create) - Create formatted tables

## See Also

- [Tables](tables.md) - Formatted data tables
- [Drawing Primitives](drawing.md) - Drawing text and shapes
- [API Reference: Text](../api/text.md) - Complete API details

---

[← Terminal](terminal.md) | [Back to Documentation](../README.md) | [Next: Buffers →](buffers.md)
