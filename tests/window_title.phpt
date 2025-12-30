--TEST--
Window title functions
--EXTENSIONS--
tui
--FILE--
<?php
// Test set_title function exists
tui_set_title('Test Title');
echo "set_title: OK\n";

// Test with empty string
tui_set_title('');
echo "set_title empty: OK\n";

// Test with special characters
tui_set_title('Title with émojis');
echo "set_title unicode: OK\n";

// Test reset_title
tui_reset_title();
echo "reset_title: OK\n";
?>
--EXPECTREGEX--
.*set_title: OK
.*set_title empty: OK
.*set_title unicode: OK
.*reset_title: OK
