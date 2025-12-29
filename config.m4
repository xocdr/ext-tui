dnl config.m4 for extension tui

PHP_ARG_ENABLE([tui],
  [whether to enable tui support],
  [AS_HELP_STRING([--enable-tui],
    [Enable tui support])],
  [no])

if test "$PHP_TUI" != "no"; then
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
  TUI_SOURCES="tui.c \
     src/terminal/terminal.c \
     src/terminal/ansi.c \
     src/event/loop.c \
     src/event/input.c \
     src/node/node.c \
     src/node/reconciler.c \
     src/render/buffer.c \
     src/render/output.c \
     src/text/measure.c \
     src/text/wrap.c \
     src/drawing/primitives.c \
     src/drawing/canvas.c \
     src/drawing/animation.c \
     src/drawing/table.c \
     src/drawing/progress.c \
     src/drawing/sprite.c \
     src/app/app.c \
     src/testing/renderer.c \
     src/testing/query.c \
     src/pool/pool.c"

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
