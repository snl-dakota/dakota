dnl @synopsis ACRO_ARG_ENABLE_PACKAGE(LC_PACKAGE, UC_PACKAGE,
dnl               PATH_TO_PACKAGE_DIR
dnl               PATH_TO_FILE_IN_PACKAGE
dnl               DEFAULT)
dnl  
dnl LC_PACKAGE, UC_PACKAGE - package name in lower and upper case, alphanumeric
dnl                          and underscores only
dnl PATH_TO_PACKAGE_DIR - path to package RELATIVE to Makefile that will list
dnl                       it as a SRCDIR
dnl PATH_TO_FILE_IN_PACKAGE - absolute path to a file in the package in the
dnl                           source directory
dnl DEFAULT - yes or no, default --disable-package value
dnl
AC_DEFUN([ACRO_ARG_ENABLE_PACKAGE],
[
if test -f $4 ; then 
  AC_ARG_ENABLE([$3],
  AS_HELP_STRING([--enable-$3],[build acro with $2]),
  ac_cv_enable_$1=$enableval,
  ac_cv_enable_$1=$5
  )
else
  ac_cv_enable_$1=no
fi
  
AC_MSG_CHECKING(whether to include [$1])
  
AM_CONDITIONAL([BUILD_$2], test "X$ac_cv_enable_$1" = "Xyes")
if test "X$ac_cv_enable_$1" = "Xyes" ; then
  AC_DEFINE([USING_$2],,[Define if want to build with $3 enabled])
  AC_MSG_RESULT(yes)  
  $2_DIR=$3
else
  AC_MSG_RESULT(no)
fi

AC_SUBST([$2_DIR])
])
