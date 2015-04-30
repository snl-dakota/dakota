dnl ACRO compiler flags

AC_DEFUN([ACRO_COMPILER_FLAGS],[
  dnl
  dnl Define the compiler flags.
  dnl 

dnl
dnl Special ACRO build flags - only processed if we are not using
dnl a cache file.
dnl
dnl If the user specified --with_*flags= on the command line, use those.
dnl Otherwise use any flags that were specified in the environment.
dnl Add the command line --with-debugging and/or --with-optimization flags.
dnl Add special flags that we may define for various platforms (but only
dnl   if there was no --with-*flags= on configure command line).
dnl TODO: There are more flags we could set: Fortran, AR, LDFLAGS, etc.
dnl
dnl TODO: Find copies of AX_CXXFLAGS_*_OPTION macros so we can
dnl test that the chosen flags work.
dnl (AX_CFLAGS_*_OPTION macros are in the autoconf macro archive)
dnl
dnl TAKE CARE: Do not allow leading or trailing spaces in variables.
dnl  It can break subsequent configures that use cache variables.
dnl  ACRO_ADD will add flags if they are defined and not equal to "notset".
dnl

ACRO_ADD(CPPFLAGS, ${USERDEF_CPPFLAGS})
ACRO_ADD(FFLAGS, ${USERDEF_FFLAGS})
ACRO_ADD(CFLAGS, ${USERDEF_CFLAGS})
ACRO_ADD(CXXFLAGS, ${USERDEF_CXXFLAGS})

dnl
dnl Extra configure command line options --with-debugging, --with-optimization
dnl

ACRO_ADD(FFLAGS, ${WITH_DEBUGGING})
ACRO_ADD(CFLAGS, ${WITH_DEBUGGING})
ACRO_ADD(CXXFLAGS, ${WITH_DEBUGGING})

ACRO_ADD(FFLAGS, ${WITH_OPT})
ACRO_ADD(CFLAGS, ${WITH_OPT})
ACRO_ADD(CXXFLAGS, ${WITH_OPT})

dnl
dnl Special flags: apply only if they did not specify --with-*flags=
dnl
if test -z "${USERDEF_CPPFLAGS}"; then
  ACRO_ADD(CPPFLAGS, ${SPECIAL_CPPFLAGS})
fi
if test -z "${USERDEF_CFLAGS}" ; then
  ACRO_ADD(CFLAGS, ${SPECIAL_CFLAGS})
fi
if test -z "${USERDEF_CXXFLAGS}" ; then
  ACRO_ADD(CXXFLAGS, ${SPECIAL_CXXFLAGS})
fi

dnl
dnl Default optimization level - apply if they didn't specify any flags
dnl

if test -z "${FFLAGS}" ; then
  FFLAGS=${optimization_default}
fi
if test -z "${CFLAGS}" ; then
  CFLAGS=${optimization_default}
fi
if test -z "${CXXFLAGS}" ; then
  CXXFLAGS=${optimization_default}
fi

dnl
dnl Special mingw flag
dnl

if test X${acro_cv_buildoption_mingw} = Xmingw ; then

  testflag=`echo ${CFLAGS} | grep cygwin`
  if test -z "${testflag}" ; then
    ACRO_ADD(CFLAGS, -mno-cygwin)
  fi
  testflag=`echo ${CXXFLAGS} | grep cygwin`
  if test -z "${testflag}" ; then
    ACRO_ADD(CXXFLAGS, -mno-cygwin)
  fi
  testflag=`echo ${FFLAGS} | grep cygwin`
  if test -z "${testflag}" ; then
    ACRO_ADD(FFLAGS, -mno-cygwin)
  fi
  testflag=`echo ${CPPFLAGS} | grep cygwin`
  if test -z "${testflag}" ; then
    ACRO_ADD(CPPFLAGS, -mno-cygwin)
  fi
  testflag=`echo ${LDFLAGS} | grep cygwin`
  if test -z "${testflag}" ; then
    ACRO_ADD(LDFLAGS, -mno-cygwin)
  fi
fi
  
ACRO_ADD(CFLAGS, ${WITH_GCOV})
ACRO_ADD(CXXFLAGS, ${WITH_GCOV})
ACRO_ADD(FFLAGS, ${WITH_GCOV})

])
