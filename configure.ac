AC_INIT([shellfront], [0.0], [wylieyyyy@gmail.com])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PROG_AR
AC_PROG_CC
LT_INIT

PKG_CHECK_MODULES([GTK], [gtk+-3.0 >= 3.10])
PKG_CHECK_MODULES([VTE], [vte-2.91 >= 0.43])

AC_MSG_CHECKING([whether to run unit tests])
AC_ARG_ENABLE([unit-tests],
	[AS_HELP_STRING([--enable-unit-tests], [run unit tests])],
	[enable_unit_tests="yes"], [enable_unit_tests="no"])
AC_MSG_RESULT([$enable_unit_tests])
AM_CONDITIONAL([UNIT_TEST], [test "X$enable_unit_tests" = "Xyes"])

AC_CONFIG_FILES([src/makefile makefile shellfront.pc])
AC_PROG_INSTALL
AC_OUTPUT
