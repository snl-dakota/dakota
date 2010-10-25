#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Prerequisites.
AC_PREREQ([2.60])
AC_INIT([motif], [1.0], [dakota@sandia.gov])
AC_CONFIG_SRCDIR([src/Graph2D.C])
AC_CONFIG_HEADER([motif_config.h])
AM_INIT_AUTOMAKE([1.9.6 foreign subdir-objects])

# Since we're using libtool, don't build shared libraries!
AC_DISABLE_SHARED

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_INSTALL

# Tells autoconf that libtool will be used.
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_FIND_XPM
AC_HEADER_STDC
AC_CHECK_HEADERS([malloc.h sys/param.h sys/types.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_C_PROTOTYPES
dnl AC_CXX_HAVE_STD

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([floor pow sqrt strchr])

# Write Makefiles and exit.
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
