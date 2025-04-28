dnl Process this file with autoconf to produce a configure script.
AC_INIT([C64play], [1.0.0-beta4], [], [], [https://github.com/ruby-R53/C64play])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([src/config.h])
AM_INIT_AUTOMAKE


AC_PROG_RANLIB

dnl Checks for programs.
AC_PROG_CXX

dnl Use C++ for tests.
AC_LANG([C++])

AX_CXX_COMPILE_STDCXX([20], [noext], [optional])
 AS_IF([test $ax_cv_cxx_compile_cxx20__std_cpp20 != "yes"], [
	 AX_CXX_COMPILE_STDCXX([17], [noext], [mandatory])
])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN

AM_ICONV
AM_CONDITIONAL([USE_ICONV], [test "x$am_cv_func_iconv" = "xyes"])

dnl Audio subsystem

AUDIO_LDFLAGS=""

PKG_CHECK_MODULES(PULSE,
	[libpulse-simple >= 1.0],
	[AC_DEFINE([HAVE_PULSE], 1, [Define to 1 if you have libpulse-simple (-lpulse-simple).])],
	[AC_MSG_WARN([$PULSE_PKG_ERRORS])]
)


dnl Checks what version of Unix we have and soundcard support
AC_CHECK_HEADERS([sys/ioctl.h linux/soundcard.h machine/soundcard.h sys/soundcard.h soundcard.h])

# NetBSD/OpenBSD OSS audio emulation
AS_IF([test "x$ac_cv_header_soundcard_h" = "xyes"],
	[AUDIO_LDFLAGS="$AUDIO_LDFLAGS -lossaudio"]
)

AC_SUBST(AUDIO_LDFLAGS)

PKG_CHECK_MODULES(SIDPLAYFP, [libsidplayfp >= 2.2.0])

# hack?
saveCPPFLAGS=$CPPFLAGS
CPPFLAGS="$CPPFLAGS $C64PLAY_CFLAGS"

AC_CHECK_HEADERS([sidplayfp/builders/residfp.h sidplayfp/builders/resid.h])

CPPFLAGS=$saveCPPFLAGS

AC_MSG_CHECKING([for debugging])
AC_ARG_ENABLE(debug, [AS_HELP_STRING([--enable-debug], [compile for debugging @<:@no/yes, default=no@:>@])],
 [], [enable_debug=no])

AS_IF([test "x$enable_debug" = "xno"],
  [AC_MSG_RESULT([Build without debugging messages]); debug_flags=-DNDEBUG]
)

AC_SUBST([debug_flags])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
