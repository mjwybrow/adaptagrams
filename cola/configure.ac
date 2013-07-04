AC_INIT([libcola],[0.1])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(libcola/cola.h)
AC_CONFIG_HEADERS([libcola/config.h])
AM_INIT_AUTOMAKE
# able to choose or disable these two at configure time:
AC_ENABLE_STATIC
AC_ENABLE_SHARED
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL
LT_INIT
AC_PROG_INSTALL
#AC_DEFINE(TRACE_LOGGING)
dnl ******************************
dnl   Compilation warnings
dnl ******************************
if test "$GXX" = "yes"; then
  # Enable some warnings from g++.

  # Rationale: a number of bugs in inkscape have been fixed by enabling g++
  # warnings and addressing the produced warnings.  Usually the committing
  # developer is the best person to address the warnings.

  ink_svd_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="-Wno-unused-parameter $CXXFLAGS"
  # -Wno-unused-parameter isn't accepted by gcc 2.95.
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([int dummy;
  ])], , CXXFLAGS="-Wno-unused $ink_svd_CXXFLAGS",)
  # Note: At least one bug has been caught from unused parameter warnings,
  # so it might be worth trying not to disable it.
  # One way of selectively disabling the warnings (i.e. only where the
  # programmer deliberately isn't using the parameter, e.g. for a callback)
  # is to remove the parameter name (leaving just its type), as is done
  # in src/seltrans.cpp:sp_seltrans_handle_event; this indicates that the
  # programmer deliberately has an unused parameter (e.g. because it's used
  # as a callback or similar function pointer use).

  CXXFLAGS="-Wall -Wpointer-arith -Wcast-align -Wsign-compare -Woverloaded-virtual -Wswitch  $CXXFLAGS"

  dnl Test for arch-specific situations.
  case "$host_cpu" in
    mips|mipsel)
      dnl Symbol tables can get too large: this uses alternate tables
      dnl See http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=283476
      CXXFLAGS="$CXXFLAGS -Wa,-xgot"
      CFLAGS="$CFLAGS -Wa,-xgot"
      ;;
  esac
fi

# If we are on a Windows OS using MinGW, extend the linker flags by '-no-undefined'
# to avoid linker warnings
if test "x$host_os" = "xmingw32"; then
  AM_LDFLAGS="$AM_LDFLAGS -no-undefined"
fi
# Distribute the changed linker flags among the Makefiles
AC_SUBST(AM_LDFLAGS)

#AC_CHECK_LIB(cairomm-1.0,cairo_create)
PKG_CHECK_MODULES(CAIROMM,cairomm-1.0,cairomm=yes,cairomm=no)
if test "x$cairomm" = "xyes"; then
	AC_DEFINE(HAVE_CAIROMM, 1, [Enable CairoMM code])
fi
AC_SUBST(CAIROMM_CFLAGS)
AC_SUBST(CAIROMM_LIBS)

AH_BOTTOM([ 
  
])

AC_CONFIG_FILES([Makefile libcola/Makefile libcola/tests/Makefile libvpsc/Makefile libvpsc/tests/Makefile libproject/Makefile libproject/tests/Makefile libtopology/Makefile libtopology/tests/Makefile libavoid/Makefile libavoid/tests/Makefile])
AC_OUTPUT
