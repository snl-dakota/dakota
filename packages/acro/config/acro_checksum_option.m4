dnl ACRO CheckSum option

AC_DEFUN([ACRO_CHECKSUM_OPTION],[
  dnl
  dnl --enable-checksum=yes is the default
  dnl 
AC_ARG_ENABLE(checksum,
AS_HELP_STRING([--disable-checksum],[Omit the utilib checksum function]),
[ENABLE_CHECKSUM=$enableval],
[ENABLE_CHECKSUM=yes]
)

if test X${ENABLE_CHECKSUM} = Xyes; then
    AC_DEFINE(YES_CHECKSUM,,[define whether checksum function is included in utilib])
fi

])
