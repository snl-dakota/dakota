dnl @synopsis ACRO_ARG_ENABLE_PACKAGE_RECONFIG(LC_PACKAGE, 
dnl                                            CONFIGSTATUS, DEFAULT)
dnl  
dnl Acro has subpackages which are independent autoconf'd packages.
dnl If we detect that configure has already been run, and the
dnl configure option --enable-{package}-reconfig has value "no",
dnl then we will not re-run configure for that package.
dnl
dnl LC_PACKAGE - package name in lower case
dnl CONFIGSTATUS - path to the package's config.status file, existence
dnl                of this file means configure has been run
dnl DEFAULT - "yes" to re-run configure even if it's been run, "no" to skip
dnl           running configure if it's already been run
dnl
AC_DEFUN([ACRO_ARG_ENABLE_PACKAGE_RECONFIG],
[
if test X${ac_cv_enable_$1} = Xyes ; then
  AC_ARG_ENABLE([$1-reconfig],
  AS_HELP_STRING([--enable-$1-reconfig],[re-run "configure" for package $1 (default $3)]),
  ac_cv_reconfigure_$1=$enableval, ac_cv_reconfigure_$1=$3)

  if test "X$ac_cv_reconfigure_$1" = "Xno" ; then
    if test -s $2 ; then
      :
    else
      ac_cv_reconfigure_$1=yes
    fi
  fi

  AC_MSG_CHECKING(whether to reconfigure [$1])
  if test "X$ac_cv_reconfigure_$1" = "Xno" ; then
    AC_MSG_RESULT(no)
    AC_MSG_WARN([Skipping configuration of $1 because it's already configured.])
    AC_MSG_WARN([Use --enable-$1-reconfig to force rerun of configure script.])
  else
    AC_MSG_RESULT(yes)  
  fi
else
  ac_cv_reconfigure_$1=no
fi
])

