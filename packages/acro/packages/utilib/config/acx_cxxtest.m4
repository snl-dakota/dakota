dnl @synopsis ACX_CXXTEST([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl This macro enables configuration of the TinyXML library.
dnl
dnl This macro should check for a cxxtest installation in the system
dnl include directories.  But that is uncommon right now...
dnl
AC_DEFUN([ACX_CXXTEST], [
acx_cxxtest_ok=no

AC_ARG_WITH(cxxtest,
	AS_HELP_STRING([--with-cxxtest=<dir>], [use TinyXML library in directory <dir>]))
case $with_cxxtest in
	yes | "") ;;
	no) acx_cxxtest_ok=disable ;;
	*) acx_cxxtest_ok=yes ; 
       CXXTEST_INCLUDE="-I$with_cxxtest" ;;
esac

AC_SUBST(CXXTEST_INCLUDE)

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$acx_cxxtest_ok" = xyes; then
        AM_CONDITIONAL(HAVE_CXXTEST, true)
        ifelse([$1],,AC_DEFINE(HAVE_CXXTEST,1,[Define if you have a CXXTEST library.]),[$1])
        :
else
        AM_CONDITIONAL(HAVE_CXXTEST, false)
        acx_cxxtest_ok=no
        $2
fi
])dnl ACX_CXXTEST
