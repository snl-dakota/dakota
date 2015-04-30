dnl ACRO debugpr option

AC_DEFUN([ACRO_DEBUGPR_OPTION],[
  dnl
  dnl --enable-debug-output=yes is the default
  dnl 
AC_ARG_ENABLE(debug-output,
AS_HELP_STRING([--disable-debug-output],[Omit the utilib debugpr output printing]),
[ENABLE_DEBUGPR=$enableval],
[ENABLE_DEBUGPR=yes]
)

if test X${ENABLE_DEBUGPR} = Xyes; then
    AC_DEFINE(YES_DEBUGPR,,[define whether DEBUGPR is enabled in utilib])
fi

])
