dnl ACRO test options

AC_DEFUN([ACRO_TEST_OPTION],[
  dnl
  dnl --enable-tests=yes is the default
  dnl 
#
# --enable-tests=yes is the default
#
AC_ARG_ENABLE(tests,
AS_HELP_STRING([--disable-tests],[Omit the build of test directories]),
[ENABLE_TESTS=$enableval],
[ENABLE_TESTS=yes]
)

if test X${ENABLE_TESTS} = Xyes ; then
  AM_CONDITIONAL(BUILD_TESTS, true)
else
  AM_CONDITIONAL(BUILD_TESTS, false)
fi

])
