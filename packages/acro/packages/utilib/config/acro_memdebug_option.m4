dnl ACRO MEMDEBUG option

AC_DEFUN([ACRO_MEMDEBUG_OPTION],[
  dnl
  dnl --enable-memdebug=no is the default
  dnl 
AC_ARG_ENABLE(memdebug,
AS_HELP_STRING([--enable-memdebug],[Turn on the utilib memdebug function]),
[ENABLE_MEMDEBUG=$enableval],
[ENABLE_MEMDEBUG=no]
)

if test X${ENABLE_MEMDEBUG} = Xyes; then
    AC_DEFINE(YES_MEMDEBUG,,[define whether memdebug is included in utilib])
fi

])
