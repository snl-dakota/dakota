AC_DEFUN([ACRO_CPLEX_OPTIONS], [

AC_ARG_WITH(cplex-incdir,
        AS_HELP_STRING([--with-cplex-incdir=<include-dir>], [path to CPLEX header files]),
        [CPLEX_INCLUDE_DIR=$withval], [CPLEX_INCLUDE_DIR=""])

AC_ARG_WITH(cplex-lib,
        AS_HELP_STRING([--with-cplex-lib=<library>], [Full path to cplex library]),
        [CPLEX_LIB=$withval], [CPLEX_LIB=""])

if test "X${CPLEX_LIB}" = "X" ; then
  AM_CONDITIONAL(BUILD_CPLEX, false)
else
  AM_CONDITIONAL(BUILD_CPLEX, true)
  AC_DEFINE(HAVE_CPLEX,1,[Define if you have CPLEX library.])
fi

AC_SUBST(CPLEX_INCLUDE_DIR)
AC_SUBST(CPLEX_LIB)

])
