dnl ACRO EXPAT library options

AC_DEFUN([ACRO_EXPAT_OPTIONS],[
  dnl
  dnl UTILIB can be built with or without the expat library.
  dnl
  dnl If any of the expat options (other than --without-expat) are given
  dnl then assume the user wants to use expat.  If expat is not found
  dnl or it doesn't work for us, issue an error.
  dnl
  dnl If no expat options are supplied, look for a working expat.  If
  dnl it is found, build with expat, otherwise build without expat.
  dnl
  dnl Defaults:

EXPAT_INCDIR=none
AM_CONDITIONAL(USE_EXPAT_INCDIR, false) 

EXPAT_LIBDIR=none
AM_CONDITIONAL(USE_EXPAT_LIBDIR, false) 

WISH_EXPAT=noPreference

AC_ARG_WITH(expat,
  AS_HELP_STRING([--without-expat],[don't use libexpat in compilation, default is to use it if we have it]),
[
  if test "X${withval}" = "Xno"; then WISH_EXPAT=no ; else WISH_EXPAT=yes ; fi
],
)

AC_ARG_WITH(expat-libs,
  AS_HELP_STRING([--with-expat-libs="-lexpat1 -lexpat2"],[expat libraries, default is "-lexpat"]),
[
  EXPAT_LIBS=${withval}
  WISH_EXPAT=yes
],
[
  EXPAT_LIBS="-lexpat"
]
)

AC_SUBST([EXPAT_LIBS])

AC_ARG_WITH(expat-incdir,
AS_HELP_STRING([--with-expat-incdir="/opt/expat/include"],[location of expat.h, default is the usual include directories]),
[
  EXPAT_INCDIR=${withval}
  AC_SUBST([EXPAT_INCDIR])
  AM_CONDITIONAL(USE_EXPAT_INCDIR, true) 
  WISH_EXPAT=yes
],
)

AC_ARG_WITH(expat-libdir,
AS_HELP_STRING([--with-expat-libdir="/opt/expat/lib"],[location of expat libraries, default is the usual library directories]),
[
  EXPAT_LIBDIR=${withval}
  AC_SUBST([EXPAT_LIBDIR])
  AM_CONDITIONAL(USE_EXPAT_LIBDIR, true) 
  WISH_EXPAT=yes
],
)

AM_CONDITIONAL(USE_EXPAT_LIBRARY, false) 

if test "X${WISH_EXPAT}" != "Xno"; then 

  USE_EXPAT=yes

  CPPF=${CPPFLAGS}
  LDF=${LDFLAGS}
  L=${LIBS}

  if test "X${EXPAT_INCDIR}" != "Xnone" ; then 
    MY_INCLUDE="-I${EXPAT_INCDIR}"
    ACRO_ADD(CPPFLAGS, ${MY_INCLUDE})
  fi
  if test "X${EXPAT_LIBDIR}" != "Xnone" ; then 
    MY_LIB="-L${EXPAT_LIBDIR}"
    ACRO_ADD(LDFLAGS, ${MY_LIB})
  fi

  ACRO_ADD(LIBS, ${EXPAT_LIBS})

  AC_LANG([C++])

  AC_MSG_CHECKING(whether we can preprocess expat.h)
  AC_PREPROC_IFELSE(
  [AC_LANG_SOURCE([[#include "expat.h"]])],
  [
    AC_MSG_RESULT(yes)
  ],[
    AC_MSG_RESULT(no)
    USE_EXPAT=no
    if test "X${WISH_EXPAT}" = "XnoPreference"; then 
      AC_MSG_WARN([UTILIB will be built without the expat library])
    else
      AC_MSG_ERROR([unable to find expat.h, see expat configure options])
    fi
  ])

  if test "X${USE_EXPAT}" = "Xyes" ; then

    AC_MSG_CHECKING(whether we can compile expat.h)
    AC_COMPILE_IFELSE(
    [AC_LANG_SOURCE([[#include "expat.h"]],[[XML_Parser parser; XML_SetUserData(parser, NULL);]])],
    [
      AC_MSG_RESULT(yes)
    ],[
      AC_MSG_RESULT(no)
      USE_EXPAT=no
      if test "X${WISH_EXPAT}" = "XnoPreference"; then 
        AC_MSG_WARN([UTILIB will be built without the expat library])
      else
        AC_MSG_ERROR([unable to compile expat.h])
      fi
    ])

    if test "X${USE_EXPAT}" = "Xyes" ; then
      AC_MSG_CHECKING(whether expat library will link using C++ compiler)
      AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([[#include "expat.h"]],[[XML_Parser parser; XML_SetUserData(parser, NULL);]])],
      [AC_MSG_RESULT(yes)],
      [AC_MSG_RESULT(no)
       USE_EXPAT=no
       if test "X${WISH_EXPAT}" = "XnoPreference"; then 
         AC_MSG_WARN([UTILIB will be built without the expat library])
       else
         AC_MSG_ERROR([unable to link code with expat library])
       fi
      ])
  
      if test "X${USE_EXPAT}" = "Xyes" ; then
        AM_CONDITIONAL(USE_EXPAT_LIBRARY, true) 
        AC_DEFINE(HAVE_EXPAT_H,,[define that expat library is available])
      fi
    fi
  fi

  CPPFLAGS=${CPPF}
  LDFLAGS=${LDF}
  LIBS=${L}
fi

])
