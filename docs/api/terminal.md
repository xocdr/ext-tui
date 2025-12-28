# API Reference: Terminal Functions

[Back to Documentation](../README.md) | [Feature Guide](../features/terminal.md)

---

## tui_get_terminal_size

Get the current terminal dimensions.

```php
array tui_get_terminal_size()
```

### Parameters

None.

### Return Value

Returns an indexed array with two elements:
- `[0]`: Width in character columns
- `[1]`: Height in character rows

### Example

```php
[$width, $height] = tui_get_terminal_size();
echo "Terminal: {$width}x{$height}\n";
```

### Notes

- Uses `TIOCGWINSZ` ioctl on Unix systems
- Falls back to environment variables `COLUMNS` and `LINES` if ioctl fails
- Default fallback is 80×24 if no size can be determined

### Related

- [tui_on_resize()](events.md#tui_on_resize) - Handle resize events
- [tui_buffer_create()](buffers.md#tui_buffer_create) - Create sized buffer

---

## tui_is_interactive

Check if running in an interactive terminal.

```php
bool tui_is_interactive()
```

### Parameters

None.

### Return Value

Returns `true` if:
- stdin is connected to a TTY
- stdout is connected to a TTY
- Not running in background

Returns `false` otherwise.

### Example

```php
if (!tui_is_interactive()) {
    // Output plain text for piped/redirected output
    echo json_encode($data);
    exit(0);
}

// Use full TUI
$buf = tui_buffer_create(...);
```

### Notes

- Uses `isatty()` internally
- Pipe detection: `php script.php | less` returns `false`
- Redirect detection: `php script.php > file` returns `false`

### Related

- [tui_is_ci()](terminal.md#tui_is_ci) - CI environment detection

---

## tui_is_ci

Check if running in a CI/CD environment.

```php
bool tui_is_ci()
```

### Parameters

None.

### Return Value

Returns `true` if any CI environment variable is detected:
- `CI=true`
- `GITHUB_ACTIONS`
- `GITLAB_CI`
- `JENKINS_URL`
- `TRAVIS`
- `CIRCLECI`
- `BUILDKITE`
- `DRONE`
- `TEAMCITY_VERSION`

Returns `false` otherwise.

### Example

```php
$use_colors = !tui_is_ci();
$style = $use_colors ? ['fg' => [100, 200, 255]] : [];
```

### Notes

- Useful for disabling colors/animations in CI
- Many CI systems set `CI=true` conventionally

### Related

- [tui_is_interactive()](terminal.md#tui_is_interactive) - TTY detection

---

[Back to Documentation](../README.md) | [Feature Guide](../features/terminal.md)
