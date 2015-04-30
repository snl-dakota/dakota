dnl Request that executables be built with all static libraries
dnl
dnl When you link with -static, the linker will build static libraries
dnl  and link with your local static libraries, but will use shared
dnl  system libraries if they are found
dnl
dnl This option requests that *all* libraries linked with the
dnl  executables be static.

AC_DEFUN([ACRO_STATIC_EXECUTABLES],[
  dnl
  dnl --enable-static-executables=no is the default
  dnl 
  dnl Link flags:
  dnl    -all-static   is for libtool to know to link in all static libs
  dnl
  dnl    -static-libgcc   GCC has a library that's only shared, and it
  dnl                     isn't needed in static builds, but GCC needs
  dnl                     this flag to know to omit it
  dnl
  dnl    -allow-multiple-definition  ld's synonym for "-z muldefs", it's
  dnl                                supported by gnu mingw linker
  dnl
AC_ARG_ENABLE(static-executables,
AS_HELP_STRING([--enable-static-executables],[Link executables with static libraries only]),
[ENABLE_STATIC_EXECUTABLES=$enableval],
[ENABLE_STATIC_EXECUTABLES=no]
)

AM_CONDITIONAL(BUILD_STATIC_EXECUTABLES, false)

if test X${ENABLE_STATIC_EXECUTABLES} = Xyes; then
    AM_CONDITIONAL(BUILD_STATIC_EXECUTABLES, true)
    if test X${GCC} = Xyes ; then
      STATIC_FLAGS="-all-static -static-libgcc -allow-multiple-definition"
    else
      STATIC_FLAGS="-all-static"
    fi
    AC_SUBST([STATIC_FLAGS])
    AC_DEFINE(YES_STATIC_EXECUTABLES,,[define whether executables are linked with static libraries only])
fi

])
