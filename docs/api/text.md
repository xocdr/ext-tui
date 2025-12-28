# API Reference: Text Functions

[Back to Documentation](../README.md) | [Feature Guide](../features/text.md)

---

## tui_string_width

Calculate the display width of a string in terminal columns.

```php
int tui_string_width(string $text)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$text` | string | The string to measure |

### Return Value

Returns the display width in columns, accounting for:
- ASCII characters: 1 column each
- CJK characters: 2 columns each
- Emoji: 2 columns each
- Combining characters: 0 columns (modify previous character)
- Control characters: 0 columns

### Example

```php
echo tui_string_width("Hello");       // 5
echo tui_string_width("世界");         // 4
echo tui_string_width("Hello 世界");   // 11
echo tui_string_width("🎉");           // 2
```

### Notes

- Uses Unicode East Asian Width property
- Different from `strlen()` (bytes) and `mb_strlen()` (characters)
- Essential for correct column alignment

### Related

- [tui_truncate()](text.md#tui_truncate) - Truncate to width
- [tui_pad()](text.md#tui_pad) - Pad to width

---

## tui_wrap_text

Wrap text to fit within a specified display width.

```php
array tui_wrap_text(string $text, int $width, int $mode = TUI_WRAP_WORD)
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$text` | string | Text to wrap |
| `$width` | int | Maximum width per line |
| `$mode` | int | Wrap mode constant |

### Wrap Mode Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `TUI_WRAP_NONE` | 0 | No wrapping, return single line |
| `TUI_WRAP_WORD` | 1 | Wrap at word boundaries (default) |
| `TUI_WRAP_CHAR` | 2 | Wrap at any character |
| `TUI_WRAP_WORD_CHAR` | 3 | Try word, fall back to character |

### Return Value

Returns an array of strings, one per line.

### Example

```php
$text = "The quick brown fox jumps over the lazy dog.";

$lines = tui_wrap_text($text, 20, TUI_WRAP_WORD);
// ["The quick brown fox", "jumps over the lazy", "dog."]

$lines = tui_wrap_text($text, 10, TUI_WRAP_CHAR);
// ["The quick ", "brown fox ", "jumps over", " the lazy ", "dog."]
```

### Notes

- Preserves existing newlines in input
- Unicode-aware word boundaries
- Handles CJK text correctly

### Related

- [tui_string_width()](text.md#tui_string_width) - Measure width
- [tui_truncate()](text.md#tui_truncate) - Single-line truncation

---

## tui_truncate

Truncate text to a maximum display width with ellipsis.

```php
string tui_truncate(string $text, int $width, ?string $ellipsis = "...")
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$text` | string | Text to truncate |
| `$width` | int | Maximum width including ellipsis |
| `$ellipsis` | string\|null | Ellipsis string (default: "...") |

### Return Value

Returns the truncated string with ellipsis appended if truncation occurred.

### Example

```php
echo tui_truncate("Hello World", 8);
// "Hello..."

echo tui_truncate("Hello World", 8, "…");
// "Hello W…"

echo tui_truncate("Short", 10);
// "Short" (no truncation needed)

echo tui_truncate("Hello", 2);
// "..." (width too small for any content)
```

### Notes

- Width-aware: considers display width, not byte length
- If width is smaller than ellipsis, returns just the ellipsis
- Pass `null` or `""` for no ellipsis

### Related

- [tui_string_width()](text.md#tui_string_width) - Measure width
- [tui_wrap_text()](text.md#tui_wrap_text) - Multi-line wrapping

---

## tui_pad

Pad a string to a specified display width.

```php
string tui_pad(string $text, int $width, string $align = "left", string $char = " ")
```

### Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `$text` | string | Text to pad |
| `$width` | int | Target width |
| `$align` | string | Alignment: "left", "right", or "center" |
| `$char` | string | Padding character (default: space) |

### Return Value

Returns the padded string.

### Example

```php
echo tui_pad("Hello", 10, "left");
// "Hello     "

echo tui_pad("Hello", 10, "right");
// "     Hello"

echo tui_pad("Hello", 10, "center");
// "  Hello   "

echo tui_pad("Title", 20, "center", "─");
// "───────Title────────"
```

### Notes

- Unicode-aware: handles CJK and emoji correctly
- If text is already wider than target, returns original text
- Padding character should be single-width

### Related

- [tui_string_width()](text.md#tui_string_width) - Measure width
- [tui_truncate()](text.md#tui_truncate) - Truncate long text

---

[Back to Documentation](../README.md) | [Feature Guide](../features/text.md)
