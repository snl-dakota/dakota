dnl ACRO CommonIO option

AC_DEFUN([ACRO_COMMONIO_OPTION],[
  dnl
  dnl --enable-commonio=yes is the default
  dnl 
AC_ARG_ENABLE(commonio,
AS_HELP_STRING([--disable-commonio],[Omit the utilib commonio functionality]),
[ENABLE_COMMONIO=$enableval],
[ENABLE_COMMONIO=yes]
)

if test X${ENABLE_COMMONIO} = Xno; then
    AC_DEFINE(DISABLE_COMMONIO,,[define Should CommonIO be disabled in utilib])
fi

])
