dnl
dnl
dnl AC_FIND_XPM : find libXpm, and provide variables
dnl	to easily use them in a Makefile.
dnl
dnl Adapted from a macro by Andreas Zeller.
dnl Modified for motif package in Dakota by Shannon Brown.
dnl
dnl The variables provided are :
dnl	link_xpm		(e.g. -L/usr/lesstif/lib -lXm)
dnl	include_xpm		(e.g. -I/usr/lesstif/lib)
dnl	xpm_libraries		(e.g. /usr/lesstif/lib)
dnl	xpm_includes		(e.g. /usr/lesstif/include)
dnl
dnl The link_xpm and include_xpm variables should be fit to put on
dnl your application's link line in your Makefile.
dnl
AC_DEFUN([AC_FIND_XPM],
[
AC_REQUIRE([AC_PATH_XTRA])
xpm_includes=
xpm_libraries=
AC_ARG_WITH(xpm,
[  --without-xpm         do not use Xpm])
dnl Treat --without-xpm like
dnl --without-xpm-includes --without-xpm-libraries.
if test "$with_xpm" = "no"
then
    xpm_includes=no
    xpm_libraries=no
fi
AC_ARG_WITH(xpm-includes,
    [  --with-xpm-includes=DIR    Xpm include files are in DIR], xpm_includes="$withval")
AC_ARG_WITH(xpm-libraries,
    [  --with-xpm-libraries=DIR   Xpm libraries are in DIR], xpm_libraries="$withval")
if test "$xpm_includes" = "no" && test "$xpm_libraries" = "no"
then
    with_xpm="no"
fi

AC_MSG_CHECKING([for Xpm])
if test "$with_xpm" != "no"
then
    #
    #
    # Search the include files.
    #
    if test "$xpm_includes" = ""
    then
	AC_CACHE_VAL(ac_cv_xpm_includes,
	[
	ac_xpm_save_CFLAGS="$CFLAGS"
	ac_xpm_save_CPPFLAGS="$CPPFLAGS"
	#
	CFLAGS="$X_CFLAGS $CFLAGS"
	CPPFLAGS="$X_CFLAGS $CPPFLAGS"
	#
	AC_TRY_COMPILE([#include <xpm.h>],[int a;],
	[
	# xpm.h is in the standard search path.
	ac_cv_xpm_includes=
	],
	[
	# xpm.h is not in the standard search path.
	# Locate it and put its directory in `xpm_includes'
	#
	# Other directories are just guesses.
	for dir in "$x_includes" "${prefix}/include" /usr/include /usr/local/include \
		/usr/include/Motif2.1 /usr/include/Motif2.0 /usr/include/Motif1.2 \
		/usr/include/X11R6 /usr/include/X11R5 \
		/usr/dt/include /usr/openwin/include \
		/usr/dt/*/include /opt/*/include /usr/include/Xpm* \
		/usr/netpub/xpm-3.4k \
		"${prefix}"/*/include /usr/*/include /usr/local/*/include \
		"${prefix}"/include/* /usr/include/* /usr/local/include/* \
		"${HOME}"/include
	do
	    if test -f "$dir/X11/xpm.h"
	    then
		ac_cv_xpm_includes="$dir/X11"
		break
	    elif test -f "$dir/xpm.h"
	    then
		ac_cv_xpm_includes="$dir"
		break
	    fi
	done
	])
	#
	CFLAGS="$ac_xpm_save_CFLAGS"
	CPPFLAGS="$ac_xpm_save_CPPFLAGS"
	])
	xpm_includes="$ac_cv_xpm_includes"
    fi

    if test -z "$xpm_includes"
    then
	xpm_includes_result="default path"
	XPM_CFLAGS=""
    else
	if test "$xpm_includes" = "no"
	then
	    xpm_includes_result="told not to use them"
	    XPM_CFLAGS=""
	else
	    xpm_includes_result="$xpm_includes"
	    XPM_CFLAGS="-I$xpm_includes"
	fi
    fi
    #
    #
    # Now for the libraries.
    #
    if test "$xpm_libraries" = ""
    then
	AC_CACHE_VAL(ac_cv_xpm_libraries,
	[
	ac_xpm_save_LIBS="$LIBS"
	ac_xpm_save_CFLAGS="$CFLAGS"
	ac_xpm_save_CPPFLAGS="$CPPFLAGS"
	#
	LIBS="-lXpm $X_LIBS -lX11 $X_EXTRA_LIBS $LIBS"
	CFLAGS="$XPM_CFLAGS $X_CFLAGS $CFLAGS"
	CPPFLAGS="$XPM_CFLAGS $X_CFLAGS $CPPFLAGS"
	#
	AC_TRY_LINK([#include <xpm.h>],[XpmAttributesSize();],
	[
	# libXpm.a is in the standard search path.
	ac_cv_xpm_libraries=
	],
	[
	# libXpm.a is not in the standard search path.
	# Locate it and put its directory in `xpm_libraries'
	#
	# Other directories are just guesses.
	for dir in "$x_libraries" "${prefix}/lib" /usr/lib /usr/local/lib \
		   /usr/lib/Xlt2.0 /usr/lib/Xlt1.2 /usr/lib/Xlt1.1 \
		   /usr/lib/X11R6 /usr/lib/X11R5 /usr/lib/X11 \
		   /usr/dt/lib /usr/openwin/lib \
		   /usr/dt/*/lib /opt/*/lib /usr/lib/Xpm* \
		   /usr/lesstif*/lib /usr/lib/Lesstif* \
		   /usr/netpub/xpm-3.4k/lib \
		   "${prefix}"/*/lib /usr/*/lib /usr/local/*/lib \
		   "${prefix}"/lib/* /usr/lib/* /usr/local/lib/* \
		   "${HOME}"/lib
	do
	    for ext in "sl" "so" "a" "lib" ; do
		if test -d "$dir" && test -f "$dir/libXpm.$ext"; then
		    ac_cv_xpm_libraries="$dir"
		    break 2
		fi
	    done
	done
	])
	#
	LIBS="$ac_xpm_save_LIBS"
	CFLAGS="$ac_xpm_save_CFLAGS"
	CPPFLAGS="$ac_xpm_save_CPPFLAGS"
	])
	#
	xpm_libraries="$ac_cv_xpm_libraries"
    fi
    if test -z "$xpm_libraries"
    then
	xpm_libraries_result="default path"
	XPM_LIBS="-lXpm"
    else
	if test "$xpm_libraries" = "no"
	then
	    xpm_libraries_result="told not to use it"
	    XPM_LIBS=""
	else
	    xpm_libraries_result="$xpm_libraries"
	    XPM_LIBS="-L$xpm_libraries -lXpm"
	fi
    fi
#
# Make sure, whatever we found out, we can link.
#
    ac_xpm_save_LIBS="$LIBS"
    ac_xpm_save_CFLAGS="$CFLAGS"
    ac_xpm_save_CPPFLAGS="$CPPFLAGS"
    #
    LIBS="$XPM_LIBS -lXpm $X_LIBS -lX11 $X_EXTRA_LIBS $LIBS"
    CFLAGS="$XPM_CFLAGS $X_CFLAGS $CFLAGS"
    CPPFLAGS="$XPM_CFLAGS $X_CFLAGS $CPPFLAGS"

    AC_TRY_LINK([#include <xpm.h>],[XpmAttributesSize();],
	[
	#
	# link passed
	#
	AC_DEFINE(HAVE_XPM, 1, Define if the Xpm library is available)
	],
	[
	#
	# link failed
	#
	xpm_libraries_result="test link failed"
	xpm_includes_result="test link failed"
	with_xpm="no"
	XPM_CFLAGS=""
	XPM_LIBS=""
	]) dnl AC_TRY_LINK

    LIBS="$ac_xpm_save_LIBS"
    CFLAGS="$ac_xpm_save_CFLAGS"
    CPPFLAGS="$ac_xpm_save_CPPFLAGS"
else
    xpm_libraries_result="told not to use it"
    xpm_includes_result="told not to use them"
    XPM_CFLAGS=""
    XPM_LIBS=""
fi
AC_MSG_RESULT([libraries $xpm_libraries_result, headers $xpm_includes_result])
AC_SUBST(XPM_CFLAGS)
AC_SUBST(XPM_LIBS)
])
dnl AC_DEFN
