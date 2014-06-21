AC_PREREQ([2.68])
AC_INIT([xmms-osd], [1.0.2], [Tommy.Reynolds@MegaCoder.com])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([src/config.h])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strtolu])
AC_FUNC_MALLOC
AC_TYPE_SIZE_T

# Checks for header files.
AC_CHECK_HEADERS([limits.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([
        Makefile
        src/Makefile
        xmms-osd.spec
])
AC_OUTPUT
