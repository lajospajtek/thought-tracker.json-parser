AC_INIT([json-parser], [0.901], [bug-automake@gnu.org])
AM_INIT_AUTOMAKE([-Wall -Werror gnu])

AC_PROG_CXX

AC_ENABLE_STATIC(no)
AC_PROG_LIBTOOL

AC_PREFIX_DEFAULT([/usr])
AC_CONFIG_COMMANDS_PRE([if test "$sysconfdir" = '${prefix}/etc'; then sysconfdir=/etc; fi])

AC_ARG_ENABLE(debug, [AS_HELP_STRING([--enable-debug], [enable debug @<:@default=no@:>@])], [CXXFLAGS="-g -O0 -fno-inline"], [CXXFLAGS="-O2"])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/unit/Makefile
])

AC_OUTPUT
