dnl @synopsis ACX_SNOPT([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl This macro looks for a library that implements the SNOPT
dnl optimization library (see 
dnl http://www.sbsi-sol-optimize.com/asp/sol_product_snopt.htm).  On success, 
dnl it sets the SNOPT_LIBS output variable to hold the requisite library
dnl linkages.
dnl
dnl To link with SNOPT, you should link with:
dnl
dnl     $SNOPT_LIBS $FLIBS
dnl
dnl in that order.  FLIBS is the output variable of the
dnl AC_F77_LIBRARY_LDFLAGS macro and is sometimes necessary in order to 
dnl link with F77 libraries.  Users will also need to use AC_F77_DUMMY_MAIN 
dnl (see the autoconf manual), for the same reason.
dnl
dnl The user may also use --with-snopt=<lib> in order to use some
dnl specific SNOPT library <lib>.  In order to link successfully,
dnl however, be aware that you will probably need to use the same
dnl Fortran compiler (which can be set via the F77 env. var.) as
dnl was used to compile the SNOPT library.
dnl
dnl ACTION-IF-FOUND is a list of shell commands to run if a SNOPT
dnl library is found, and ACTION-IF-NOT-FOUND is a list of commands
dnl to run it if it is not found.  If ACTION-IF-FOUND is not specified,
dnl the default action will define HAVE_SNOPT.
dnl
dnl @version $Id: acx_snopt.m4 4379 2007-01-31 03:36:20Z wehart $
dnl @author William E. Hart <wehart@sandia.gov>

AC_DEFUN([ACX_SNOPTLIB], [
acx_snopt_ok=no

AC_ARG_WITH(snopt,
        AS_HELP_STRING([--with-snopt=<lib-flags>], [use SNOPT library]))
case $with_snopt in
        yes | "") ;;
        no) acx_snopt_ok=disable ;;
        *) SNOPT_LIBS="$with_snopt" ;;
esac

# Get fortran linker name of SNOPT function to check for.
AC_F77_FUNC(snopt)


# First, check SNOPT_LIBS environment variable
if test "x$SNOPT_LIBS" = x; then :; else
        save_LIBS="$LIBS"; LIBS="$SNOPT_LIBS $LIBS $BLAS_LIBS $FLIBS"
        AC_MSG_CHECKING([for $snopt in $SNOPT_LIBS $FLIBS])
        AC_TRY_LINK_FUNC($snopt, [acx_snopt_ok=yes], [SNOPT_LIBS=""])
        AC_MSG_RESULT($acx_snopt_ok)
        LIBS="$save_LIBS"
        if test acx_snopt_ok = no; then
                SNOPT_LIBS=""
        fi
fi

# SNOPT linked to by default?
if test $acx_snopt_ok = no; then
        save_LIBS="$LIBS"; LIBS="$LIBS $FLIBS"
        AC_CHECK_FUNC($snopt, [acx_snopt_ok=yes])
        LIBS="$save_LIBS"
fi

# Generic SNOPT library?
for snopt in snopt snopt_rs6k; do
        if test $acx_snopt_ok = no; then
                save_LIBS="$LIBS"; LIBS="$LIBS"
                AC_CHECK_LIB($snopt, $snopt,
                    [acx_snopt_ok=yes; SNOPT_LIBS="-l$snopt"], [], [$FLIBS])
                LIBS="$save_LIBS"
        fi
done

AC_SUBST(SNOPT_LIBS)

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$acx_snopt_ok" = xyes; then
        ifelse([$1],,AC_DEFINE(HAVE_SNOPTLIB,1,[Define if you have SNOPT library.]),[$1])
        :
else
        acx_snopt_ok=no
        $2
fi
])dnl ACX_SNOPTLIB
