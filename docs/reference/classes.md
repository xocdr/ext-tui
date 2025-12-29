# Class Reference

Complete reference for ext-tui classes.

All classes are in the `Xocdr\Tui\Ext` namespace.

## Xocdr\Tui\Ext\Box

Flexbox container component.

### Constructor

```php
use Xocdr\Tui\Ext\Box;

new Box(array $props = [])
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
| `borderTopColor` | array\|string\|null | null | Top border color |
| `borderRightColor` | array\|string\|null | null | Right border color |
| `borderBottomColor` | array\|string\|null | null | Bottom border color |
| `borderLeftColor` | array\|string\|null | null | Left border color |
| `focusable` | bool | false | Can receive focus |
| `focused` | bool | false | Currently focused |
| `key` | string\|null | null | Reconciliation key |
| `id` | string\|null | null | Focus ID |
| `children` | array | [] | Child components |

### Methods

```php
addChild(Box|Text $child): self
```

Adds child component. Returns `$this` for chaining.

---

## Xocdr\Tui\Ext\Text

Text display component.

### Constructor

```php
use Xocdr\Tui\Ext\Text;

new Text(array $props = [])
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
| `wrap` | string\|null | null | Wrap mode (`'word'`, `'char'`) |

---

## Xocdr\Tui\Ext\Instance

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

### Hook Methods

```php
useState(mixed $initialValue): array
```
Returns `[$value, $setter]` for state management.

```php
useInput(callable $handler): void
```
Registers input handler.

```php
useFocus(): Focus
```
Returns Focus object with `isFocused` property.

```php
useFocusManager(): FocusManager
```
Returns FocusManager for programmatic focus control.

```php
useStdin(): StdinContext
```
Returns stdin stream context.

```php
useStdout(): StdoutContext
```
Returns stdout stream context with terminal dimensions.

```php
useStderr(): StderrContext
```
Returns stderr stream context.

```php
getTerminalSize(): array
```
Returns `[width, height]`.

```php
measureElement(Box|Text $element): ?array
```
Returns layout info for element.

---

## Xocdr\Tui\Ext\Key

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
| `home` | bool | Home |
| `end` | bool | End |
| `pageUp` | bool | Page Up |
| `pageDown` | bool | Page Down |
| `functionKey` | int | Function key number (1-12), 0 if not F-key |
| `ctrl` | bool | Ctrl held |
| `alt` | bool | Alt held |
| `meta` | bool | Meta held |
| `shift` | bool | Shift held |

---

## Xocdr\Tui\Ext\FocusEvent

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

---

## Xocdr\Tui\Ext\Focus

Focus state object returned by `$instance->useFocus()`.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `isFocused` | bool | Whether this element has focus |

### Methods

```php
focus(string $id): void
```
Focus element by ID.

---

## Xocdr\Tui\Ext\FocusManager

Focus management returned by `$instance->useFocusManager()`.

### Methods

```php
focusNext(): void
```
Move focus to next focusable element.

```php
focusPrevious(): void
```
Move focus to previous focusable element.

```php
focus(string $id): void
```
Focus element by ID.

```php
enableFocus(): void
```
Enable focus system.

```php
disableFocus(): void
```
Disable focus system.

---

## Xocdr\Tui\Ext\StdinContext

Stdin stream context.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `isRawModeSupported` | bool | Whether raw mode is available |

### Methods

```php
setRawMode(bool $mode): void
```
Enable/disable raw mode.

---

## Xocdr\Tui\Ext\StdoutContext

Stdout stream context.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `columns` | int | Terminal width |
| `rows` | int | Terminal height |
| `isTTY` | bool | Whether stdout is a TTY |

### Methods

```php
write(string $data): void
```
Write directly to stdout.

---

## Xocdr\Tui\Ext\StderrContext

Stderr stream context.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `columns` | int | Terminal width |
| `rows` | int | Terminal height |
| `isTTY` | bool | Whether stderr is a TTY |

### Methods

```php
write(string $data): void
```
Write directly to stderr.

---

## Xocdr\Tui\Ext\Newline

Newline component (extends Box).

### Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `count` | int | 1 | Number of blank lines |

---

## Xocdr\Tui\Ext\Spacer

Spacer component (extends Box). Automatically sets `flexGrow: 1`.

---

## Xocdr\Tui\Ext\Transform

Transform component (extends Box).

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `transform` | callable\|null | Text transformation function |

---

## Xocdr\Tui\Ext\StaticOutput

Static output component (extends Box). Content rendered above dynamic UI.

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `items` | array\|null | Items to render statically |
| `render` | callable\|null | Render function for items |

---

## Xocdr\Tui\Ext\Color

Backed string enum containing CSS named colors. Each case value is the hex color string (e.g., `#ff0000`).

### Usage

```php
use Xocdr\Tui\Ext\Color;

// Access by case name
$red = Color::Red;
echo $red->value; // "#ff0000"

// Get RGB components
$rgb = Color::Coral->toRgb();
// ['r' => 255, 'g' => 127, 'b' => 80]

// Get ANSI escape sequences for terminal styling
echo Color::Blue->toAnsi() . "Blue text" . "\033[0m";
echo Color::Green->toAnsiBg() . "Green background" . "\033[0m";

// Lookup by CSS name (case-insensitive)
$color = Color::fromName("darkorange"); // Color::DarkOrange

// Use built-in BackedEnum methods
$color = Color::from("#ff0000");      // Color::Red
$color = Color::tryFrom("#invalid");  // null
```

### Available Cases (141 colors)

All 147 CSS named colors are available (excluding 6 grey/gray duplicates - American spelling is used).

| Category | Count | Examples |
|----------|-------|----------|
| Basic | 17 | Black, White, Red, Green, Blue, Yellow, Cyan, Magenta, Gray... |
| Reds | 8 | IndianRed, LightCoral, Salmon, Crimson, FireBrick, DarkRed... |
| Pinks | 6 | Pink, LightPink, HotPink, DeepPink, MediumVioletRed... |
| Oranges | 5 | Orange, DarkOrange, OrangeRed, Tomato, Coral |
| Yellows | 10 | Gold, LightYellow, LemonChiffon, PapayaWhip, Khaki... |
| Purples | 16 | Lavender, Violet, Orchid, RebeccaPurple, Indigo, SlateBlue... |
| Greens | 19 | Chartreuse, LimeGreen, SpringGreen, ForestGreen, DarkGreen... |
| Blues/Cyans | 20 | Turquoise, SteelBlue, SkyBlue, DodgerBlue, RoyalBlue, MidnightBlue... |
| Browns | 16 | Wheat, Tan, Peru, Chocolate, SaddleBrown, Sienna, Brown... |
| Whites | 16 | Snow, Azure, AliceBlue, GhostWhite, Ivory, Linen, MistyRose... |
| Grays | 7 | Gainsboro, LightGray, DarkGray, DimGray, SlateGray... |

### Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `toRgb()` | `(): array{r: int, g: int, b: int}` | Get RGB color components |
| `toAnsi()` | `(): string` | ANSI escape sequence for foreground color |
| `toAnsiBg()` | `(): string` | ANSI escape sequence for background color |
| `fromName()` | `static (string $name): ?Color` | Lookup by CSS color name (case-insensitive) |

### Inherited BackedEnum Methods

| Method | Description |
|--------|-------------|
| `Color::cases()` | Returns all enum cases |
| `Color::from(string $value)` | Get case by hex value (throws on invalid) |
| `Color::tryFrom(string $value)` | Get case by hex value (returns null on invalid) |
| `$color->value` | Get backing hex string |
| `$color->name` | Get case name (e.g., "Red") |
