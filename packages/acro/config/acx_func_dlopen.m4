dnl @synopsis ACX_FUNC_DLOPEN([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl This macro looks for the dlopen function and identifies the library
dnl that is required to use it. On success, it sets the DLOPEN_LIBS output
dnl variable to hold the requisite library linkages.
dnl
dnl ACTION-IF-FOUND is a list of shell commands to run if DLOPEN
dnl is found, and ACTION-IF-NOT-FOUND is a list of commands
dnl to run it if it is not found.  If ACTION-IF-FOUND is not specified,
dnl the default action will define HAVE_DLOPEN.
dnl

AC_DEFUN([ACX_FUNC_DLOPEN], [
acx_func_dlopen_ok=no

# DLOPEN linked to by default?
if test $acx_func_dlopen_ok = no; then
        save_LIBS="$LIBS"; LIBS="$LIBS"
        AC_CHECK_FUNC([dlopen], [acx_func_dlopen_ok=yes])
        LIBS="$save_LIBS"
fi

# DLOPEN in Standard DL library? 
if test $acx_func_dlopen_ok = no; then
        AC_CHECK_LIB(dl, [dlopen], [acx_func_dlopen_ok=yes; DLOPEN_LIBS="-ldl"])
fi

AC_SUBST(DLOPEN_LIBS)

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$acx_func_dlopen_ok" = xyes; then
        ifelse([$1],,AC_DEFINE(HAVE_DLOPEN,1,[Define if you have the dlopen function.]),[$1])
        :
else
        acx_func_dlopen_ok=no
        $2
fi

])dnl ACX_FUNC_DLOPEN
