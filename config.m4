dnl config.m4 for extension tui

PHP_ARG_ENABLE([tui],
  [whether to enable tui support],
  [AS_HELP_STRING([--enable-tui],
    [Enable tui support])],
  [no])

PHP_ARG_ENABLE([tui-gcov],
  [whether to enable code coverage for tui],
  [AS_HELP_STRING([--enable-tui-gcov],
    [Enable code coverage (gcov) for tui extension])],
  [no])

if test "$PHP_TUI" != "no"; then
  dnl Require PHP 8.4.0 or later
  AC_MSG_CHECKING([PHP version])
  PHP_VERSION_ID=`$PHP_CONFIG --vernum`
  if test "$PHP_VERSION_ID" -lt "80400"; then
    AC_MSG_ERROR([ext-tui requires PHP 8.4.0 or later (found PHP_VERSION_ID=$PHP_VERSION_ID)])
  fi
  AC_MSG_RESULT([ok (PHP_VERSION_ID=$PHP_VERSION_ID)])

  dnl Require C++ for Yoga layout engine
  PHP_REQUIRE_CXX()

  dnl Check for termios (Unix terminal control)
  AC_CHECK_HEADERS([termios.h], [], [
    AC_MSG_ERROR([termios.h not found, required for terminal handling])
  ])

  dnl Check for poll (event loop)
  AC_CHECK_HEADERS([poll.h], [], [
    AC_MSG_ERROR([poll.h not found, required for event loop])
  ])

  dnl Check for sys/ioctl.h (terminal size)
  AC_CHECK_HEADERS([sys/ioctl.h])

  dnl Define C sources (our extension code)
  dnl Split into logical modules for maintainability
  TUI_SOURCES="tui.c \
     tui_terminal.c \
     tui_text.c \
     tui_buffer.c \
     tui_canvas.c \
     tui_animation.c \
     tui_table.c \
     tui_progress.c \
     tui_sprite.c \
     tui_render.c \
     tui_testing.c \
     tui_metrics.c \
     tui_virtual.c \
     tui_scroll.c \
     tui_graphics.c \
     tui_recording.c \
     tui_a11y.c \
     tui_dragdrop.c \
     src/terminal/terminal.c \
     src/terminal/ansi.c \
     src/terminal/capabilities.c \
     src/terminal/notify.c \
     src/event/loop.c \
     src/event/input.c \
     src/node/node.c \
     src/node/reconciler.c \
     src/node/keymap.c \
     src/render/buffer.c \
     src/render/output.c \
     src/text/measure.c \
     src/text/wrap.c \
     src/text/grapheme.c \
     src/drawing/primitives.c \
     src/drawing/canvas.c \
     src/drawing/animation.c \
     src/drawing/table.c \
     src/drawing/progress.c \
     src/drawing/sprite.c \
     src/scroll/virtual.c \
     src/scroll/smooth.c \
     src/graphics/kitty.c \
     src/graphics/iterm2.c \
     src/graphics/sixel.c \
     src/recording/recorder.c \
     src/a11y/accessibility.c \
     src/interaction/dragdrop.c \
     src/app/app.c \
     src/testing/renderer.c \
     src/testing/query.c \
     src/pool/pool.c \
     src/pool/intern.c"

  dnl Define C++ sources (Yoga layout engine)
  YOGA_SOURCES="src/yoga/YGConfig.cpp \
     src/yoga/YGEnums.cpp \
     src/yoga/YGNode.cpp \
     src/yoga/YGNodeLayout.cpp \
     src/yoga/YGNodeStyle.cpp \
     src/yoga/YGPixelGrid.cpp \
     src/yoga/YGValue.cpp \
     src/yoga/algorithm/AbsoluteLayout.cpp \
     src/yoga/algorithm/Baseline.cpp \
     src/yoga/algorithm/Cache.cpp \
     src/yoga/algorithm/CalculateLayout.cpp \
     src/yoga/algorithm/FlexLine.cpp \
     src/yoga/algorithm/PixelGrid.cpp \
     src/yoga/config/Config.cpp \
     src/yoga/debug/AssertFatal.cpp \
     src/yoga/debug/Log.cpp \
     src/yoga/event/event.cpp \
     src/yoga/node/LayoutResults.cpp \
     src/yoga/node/Node.cpp"

  dnl Set C++20 for Yoga sources
  CXXFLAGS="$CXXFLAGS -std=c++20"

  dnl Enable strict compiler warnings for better code quality
  dnl -Wall: Enable most warnings
  dnl -Wextra: Enable additional warnings
  dnl -Wno-unused-parameter: Disable unused parameter warnings (common in PHP extensions)
  dnl -Wformat=2: Strict format string checking
  dnl -Wstrict-prototypes: Require function prototypes (C only)
  dnl -Wshadow: Warn when local variable shadows another
  dnl -Wconversion: Warn on implicit type conversions that may lose data
  dnl -Wcast-align: Warn on pointer casts that increase alignment requirements
  TUI_CFLAGS="-Wall -Wextra -Wno-unused-parameter -Wformat=2 -Wshadow -Wconversion -Wno-sign-conversion"

  dnl Add C-specific warnings
  CFLAGS="$CFLAGS $TUI_CFLAGS -Wstrict-prototypes"

  dnl Add C++ warnings (no strict-prototypes for C++)
  CXXFLAGS="$CXXFLAGS $TUI_CFLAGS"

  dnl Code coverage support (gcov)
  if test "$PHP_TUI_GCOV" = "yes"; then
    AC_MSG_NOTICE([Enabling code coverage for tui extension])
    TUI_GCOV_CFLAGS="-fprofile-arcs -ftest-coverage -O0 -g"
    CFLAGS="$CFLAGS $TUI_GCOV_CFLAGS"
    CXXFLAGS="$CXXFLAGS $TUI_GCOV_CFLAGS"
    dnl On Linux with GCC, link gcov library. On macOS with clang, not needed.
    case $host_os in
      linux*)
        LDFLAGS="$LDFLAGS -lgcov"
        ;;
    esac
    PHP_SUBST(TUI_GCOV_CFLAGS)
  fi

  dnl Create extension with all sources (C and C++)
  PHP_NEW_EXTENSION([tui],
    [$TUI_SOURCES $YOGA_SOURCES],
    [$ext_shared],,
    [-DZEND_ENABLE_STATIC_TSRMLS_CACHE=1])

  dnl Include Makefile.frag for extra definitions
  PHP_ADD_MAKEFILE_FRAGMENT

  dnl Add C++ standard library
  PHP_ADD_LIBRARY(stdc++, 1, TUI_SHARED_LIBADD)
  PHP_SUBST(TUI_SHARED_LIBADD)

  dnl Add build directories for C sources
  PHP_ADD_BUILD_DIR([$ext_builddir/src/terminal])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/event])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/node])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/render])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/text])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/drawing])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/app])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/testing])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/pool])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/scroll])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/graphics])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/recording])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/a11y])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/interaction])

  dnl Add build directories for Yoga
  PHP_ADD_BUILD_DIR([$ext_builddir/src/yoga])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/yoga/algorithm])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/yoga/config])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/yoga/debug])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/yoga/enums])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/yoga/event])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/yoga/node])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/yoga/numeric])
  PHP_ADD_BUILD_DIR([$ext_builddir/src/yoga/style])

  dnl Add include paths
  PHP_ADD_INCLUDE([$ext_srcdir/src])
  PHP_ADD_INCLUDE([$ext_srcdir/src/yoga])
fi
