dnl ACRO executables options

AC_DEFUN([ACRO_EXECUTABLES_OPTION],[
  dnl
  dnl --enable-executables=yes is the default
  dnl 
AC_ARG_ENABLE(executables,
AS_HELP_STRING([--disable-executables],[Do not build any executables, just the library]),
[ENABLE_EXECUTABLES=$enableval],
[ENABLE_EXECUTABLES=yes]
)
AM_CONDITIONAL(BUILD_EXECUTABLES, test X${ENABLE_EXECUTABLES} = Xyes)

])
