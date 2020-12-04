AC_INIT([shellfront], [2.0.0], [wylieyyyy@gmail.com])
: ${CFLAGS=""}
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PROG_AR
AC_PROG_CC
AC_PROG_CC_STDC
LT_INIT

PKG_CHECK_MODULES([GTK], [gtk+-3.0 >= 3.10])
PKG_CHECK_MODULES([VTE], [vte-2.91 >= 0.43])

AC_MSG_CHECKING([whether to build unit tests])
AC_ARG_ENABLE([unit-tests],
	[AS_HELP_STRING([--enable-unit-tests], [build unit tests])],
	[enable_unit_tests="yes"], [enable_unit_tests="no"])
AC_MSG_RESULT([$enable_unit_tests])
AM_CONDITIONAL([UNIT_TEST], [test "X$enable_unit_tests" = "Xyes"])

GETTEXT_PACKAGE=shellfront
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"],
	[The domain to use with gettext])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.21])
SHELLFRONT_LOCALEDIR=[${datadir}/locale]
AC_SUBST([SHELLFRONT_LOCALEDIR])

AC_CONFIG_FILES([po/Makefile.in src/makefile makefile shellfront.pc])
AC_CONFIG_COMMANDS_POST([mv po/Makevars.template po/Makevars])
AC_PROG_INSTALL
AC_OUTPUT
