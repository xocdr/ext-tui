# Class Reference

Complete reference for ext-tui classes.

## TuiBox

Flexbox container component.

### Constructor

```php
new TuiBox(array $props = [])
```

### Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `flexDirection` | string | `'column'` | `'row'`, `'column'`, `'row-reverse'`, `'column-reverse'` |
| `alignItems` | string\|null | null | `'stretch'`, `'flex-start'`, `'center'`, `'flex-end'` |
| `justifyContent` | string\|null | null | `'flex-start'`, `'center'`, `'flex-end'`, `'space-between'`, `'space-around'`, `'space-evenly'` |
| `flexGrow` | int | 0 | Grow factor |
| `flexShrink` | int | 1 | Shrink factor |
| `width` | int\|string\|null | null | Width in cells or `'nn%'` |
| `height` | int\|string\|null | null | Height in cells or `'nn%'` |
| `padding` | int | 0 | All-side padding |
| `paddingTop` | int | 0 | Top padding |
| `paddingBottom` | int | 0 | Bottom padding |
| `paddingLeft` | int | 0 | Left padding |
| `paddingRight` | int | 0 | Right padding |
| `paddingX` | int | 0 | Horizontal padding |
| `paddingY` | int | 0 | Vertical padding |
| `margin` | int | 0 | All-side margin |
| `marginTop` | int | 0 | Top margin |
| `marginBottom` | int | 0 | Bottom margin |
| `marginLeft` | int | 0 | Left margin |
| `marginRight` | int | 0 | Right margin |
| `marginX` | int | 0 | Horizontal margin |
| `marginY` | int | 0 | Vertical margin |
| `gap` | int | 0 | Gap between children |
| `borderStyle` | string\|null | null | `'single'`, `'double'`, `'round'`, `'bold'` |
| `borderColor` | array\|string\|null | null | RGB array or hex string |
| `focusable` | bool | false | Can receive focus |
| `focused` | bool | false | Currently focused |
| `children` | array | [] | Child components |

### Methods

```php
addChild(TuiBox|TuiText $child): self
```

Adds child component. Returns `$this` for chaining.

---

## TuiText

Text display component.

### Constructor

```php
new TuiText(array $props = [])
```

### Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `content` | string | `''` | Text content |
| `color` | array\|string\|null | null | Foreground (RGB or hex) |
| `backgroundColor` | array\|string\|null | null | Background (RGB or hex) |
| `bold` | bool | false | Bold text |
| `dim` | bool | false | Dim text |
| `italic` | bool | false | Italic text |
| `underline` | bool | false | Underlined text |
| `inverse` | bool | false | Inverted colors |
| `strikethrough` | bool | false | Strikethrough |

---

## TuiInstance

Running TUI application. Returned by `tui_render()`.

### Methods

```php
rerender(): void
```
Forces re-render.

```php
unmount(): void
```
Stops TUI, restores terminal.

```php
waitUntilExit(): void
```
Blocks until exit.

```php
exit(int $code = 0): void
```
Requests exit with code.

---

## TuiKey

Keyboard event. Passed to input handlers.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `key` | string | Character pressed |
| `name` | string | Key name (`'up'`, `'down'`, `'return'`, etc.) |
| `upArrow` | bool | Up arrow |
| `downArrow` | bool | Down arrow |
| `leftArrow` | bool | Left arrow |
| `rightArrow` | bool | Right arrow |
| `return` | bool | Enter/Return |
| `escape` | bool | Escape |
| `backspace` | bool | Backspace |
| `delete` | bool | Delete |
| `tab` | bool | Tab |
| `ctrl` | bool | Ctrl held |
| `alt` | bool | Alt held |
| `meta` | bool | Meta held |
| `shift` | bool | Shift held |

---

## TuiFocusEvent

Focus change event. Passed to focus handlers.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `previous` | ?array | Previously focused node info |
| `current` | ?array | Newly focused node info |
| `direction` | string | `'next'`, `'prev'`, or `'programmatic'` |

Node info arrays contain:
- `focusable` (bool)
- `focused` (bool)
- `x` (int)
- `y` (int)
- `width` (int)
- `height` (int)
- `type` (string): `'box'` or `'text'`
