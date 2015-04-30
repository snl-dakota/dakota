dnl @synopsis ACX_CPLEX([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl This macro looks for a library that implements the CPLEX
dnl optimization library (see http://www.ilog.com/products/cplex/).  On 
dnl success, it sets the CPLEX_LIBS output variable to hold the requisite 
dnl library linkages.
dnl
dnl The user may use the following configuration options to setup
dnl cplex:
dnl 	--with-cplex-includes=<include-dir> 
dnl 	--with-cplex-libs=<link-libraries> 
dnl 	--with-cplex-ldflags=<link-flags> 
dnl
dnl ACTION-IF-FOUND is a list of shell commands to run if a CPLEX
dnl library is found, and ACTION-IF-NOT-FOUND is a list of commands
dnl to run it if it is not found.  If ACTION-IF-FOUND is not specified,
dnl the default action will define HAVE_CPLEX.
dnl
dnl @version $Id: acx_cplex.m4 4379 2007-01-31 03:36:20Z wehart $
dnl @author William E. Hart <wehart@sandia.gov>

AC_DEFUN([ACX_CPLEX], [
acx_cplex_ok=no

AC_ARG_WITH(cplex-includes,
        AS_HELP_STRING([--with-cplex-includes=<include-dir>], [use CPLEX library headers in <include-dir>]))
AC_ARG_WITH(cplex-libs,
        AS_HELP_STRING([--with-cplex-libs=<link-libraris>], [link with these CPLEX libraries]))
AC_ARG_WITH(cplex-ldflags,
        AS_HELP_STRING([--with-cplex-ldflags=<link-flags>], [link with these CPLEX flags]))

case $with_cplex_includes in
        yes | "") ;;
        no) acx_cplex_ok=disable ;;
        *) CPLEX_INCLUDES="$with_cplex_includes" ;;
esac
case $with_cplex_libs in
        yes | "") ;;
        no) acx_cplex_ok=disable ;;
        *) CPLEX_LIBS="$with_cplex_libs" ;;
esac
case $with_cplex_ldflags in
        yes | "") ;;
        no) acx_cplex_ok=disable ;;
        *) CPLEX_LDFLAGS="$with_cplex_ldflags" ;;
esac

# First, check CPLEX_LIBS environment variable
if test "x$CPLEX_LIBS" = x; then :; else
        save_LIBS="$LIBS"; LIBS="$CPLEX_LIBS $LIBS"
        AC_MSG_CHECKING([for $cplex in $CPLEX_LIBS])
        AC_TRY_LINK_FUNC($cplex, [acx_cplex_ok=yes], [CPLEX_LIBS=""])
        AC_MSG_RESULT($acx_cplex_ok)
        LIBS="$save_LIBS"
        if test acx_cplex_ok = no; then
                CPLEX_INCLUDES=""
                CPLEX_LIBS=""
                CPLEX_LDFLAGS=""
        fi
fi

AC_SUBST(CPLEX_INCLUDES)
AC_SUBST(CPLEX_LIBS)
AC_SUBST(CPLEX_LDFLAGS)

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$acx_cplex_ok" = xyes; then
        ifelse([$1],,AC_DEFINE(HAVE_CPLEX,1,[Define if you have CPLEX library.]),[$1])
        :
        AM_CONDITIONAL(BUILD_CPLEX, true)
else
        acx_cplex_ok=no
        AM_CONDITIONAL(BUILD_CPLEX, false)
        $2
fi

])dnl ACX_CPLEX
