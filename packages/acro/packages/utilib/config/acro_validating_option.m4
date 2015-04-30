dnl ACRO validating option

AC_DEFUN([ACRO_VALIDATING_OPTION],[
  dnl
  dnl --enable-validating=no is the default
  dnl 
AC_ARG_ENABLE(validating,
AS_HELP_STRING([--enable-validating],[Turn on code validation tests.]),
[ENABLE_VALIDATING=$enableval],
[ENABLE_VALIDATING=no]
)

if test X${ENABLE_VALIDATING} = Xyes; then
    AC_DEFINE(VALIDATING,,[turn on code validation tests])
fi

])
