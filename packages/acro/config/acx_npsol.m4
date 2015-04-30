dnl @synopsis ACX_NPSOL([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl This macro looks for a library that implements the NPSOL
dnl optimization library (see
dnl http://www.sbsi-sol-optimize.com/asp/sol_product_npsol.htm).  On success,
dnl it sets the NPSOL_LIBS output variable to hold the requisite library
dnl linkages.
dnl
dnl To link with NPSOL, you should link with:
dnl
dnl     $NPSOL_LIBS $FLIBS
dnl
dnl in that order.  FLIBS is the output variable of the
dnl AC_F77_LIBRARY_LDFLAGS macro and is sometimes necessary in order to
dnl link with F77 libraries.  Users will also need to use AC_F77_DUMMY_MAIN
dnl (see the autoconf manual), for the same reason.
dnl
dnl The user may also use --with-npsol=<lib> in order to use some
dnl specific NPSOL library <lib>.  In order to link successfully,
dnl however, be aware that you will probably need to use the same
dnl Fortran compiler (which can be set via the F77 env. var.) as
dnl was used to compile the NPSOL library.
dnl
dnl ACTION-IF-FOUND is a list of shell commands to run if a NPSOL
dnl library is found, and ACTION-IF-NOT-FOUND is a list of commands
dnl to run it if it is not found.  If ACTION-IF-FOUND is not specified,
dnl the default action will define HAVE_NPSOL.
dnl
dnl @version $Id: acx_npsol.m4 5111 2007-09-19 15:38:54Z dmgay $
dnl @author William E. Hart <wehart@sandia.gov>

AC_DEFUN([ACX_NPSOLLIB], [
acx_npsol_ok=no

AC_ARG_WITH(npsol,
        AS_HELP_STRING([--with-npsol=<lib-flags>], [use NPSOL library]))
case $with_npsol in
        yes | "") ;;
        no) acx_npsol_ok=disable ;;
        *) NPSOL_LIBS="$with_npsol" ;;
esac

# Get fortran linker name of NPSOL function to check for.
AC_F77_FUNC(npsol)

# First, check NPSOL_LIBS environment variable
if test "x$NPSOL_LIBS" = x; then :; else
        save_LIBS="$LIBS"; LIBS="$NPSOL_LIBS $LIBS $FLIBS"
        AC_MSG_CHECKING([for $npsol in $NPSOL_LIBS])
        AC_TRY_LINK_FUNC($npsol, [acx_npsol_ok=yes], [NPSOL_LIBS=""])
        AC_MSG_RESULT($acx_npsol_ok)
        LIBS="$save_LIBS"
        if test acx_npsol_ok = no; then
                NPSOL_LIBS=""
        fi
fi

# NPSOL linked to by default?
if test $acx_npsol_ok = no; then
        save_LIBS="$LIBS"; LIBS="$LIBS $FLIBS"
        AC_CHECK_FUNC($npsol, [acx_npsol_ok=yes])
        LIBS="$save_LIBS"
fi

# Generic NPSOL library?
for npsol in npsol npsol_rs6k; do
        if test $acx_npsol_ok = no; then
                save_LIBS="$LIBS"; LIBS="$LIBS"
                AC_CHECK_LIB($npsol, $npsol,
                    [acx_npsol_ok=yes; NPSOL_LIBS="-l$npsol"], [], [$FLIBS])
                LIBS="$save_LIBS"
        fi
done

AC_SUBST(NPSOL_LIBS)

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$acx_npsol_ok" = xyes; then
        ifelse([$1],,AC_DEFINE(HAVE_NPSOLLIB,1,[Define if you have NPSOL library.]),[$1])
        :
else
        acx_npsol_ok=no
        $2
fi
])dnl ACX_NPSOLLIB
