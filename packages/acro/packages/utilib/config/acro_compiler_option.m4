dnl ACRO compiler option

AC_DEFUN([ACRO_COMPILER_OPTION],[
  dnl
  dnl Can be used to specify specific characteristics about the compiler.
  dnl
AC_CACHE_CHECK(
  [whether to build with MinGW environment],
  [acro_cv_buildoption_mingw],
  [ AC_ARG_WITH(compiler,
      AS_HELP_STRING([--with-compiler],
        [Use --with-compiler=mingw to build mingw executables and libraries]),
      [acro_cv_buildoption_mingw=$withval],
      [acro_cv_buildoption_mingw=notset])
  ]
)
if test X${acro_cv_buildoption_mingw} = Xmingw ; then
  if test $build_os = cygwin ; then :; else
     AC_MSG_ERROR([mingw code can only be built on a cygwin system])
  fi
  AM_CONDITIONAL(HOST_MINGW, true)
  AM_CONDITIONAL(TARGET_MINGW, true)
  AC_DEFINE(HOST_MINGW,1,[software host will be mingw])
  AC_DEFINE(TARGET_MINGW,1,[software target will be mingw])
fi

if test X${acro_cv_buildoption_mingw} = Xmingw ; then :; else
  if test X${acro_cv_buildoption_mingw} = Xnotset ; then :; else
     AC_MSG_ERROR([Sorry, --with-compiler feature is only implemented for mingw right now])
  fi
fi

])

