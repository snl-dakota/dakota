dnl @synopsis AC_PROG_DVIPS
dnl
dnl This macro test if dvips is installed. If dvips is installed, it
dnl set $dvips to the right value
dnl
dnl @category InstalledPackages
dnl @category LaTeX
dnl @author Mathieu Boretti <boretti@bss-network.com>
dnl @version 2005-01-21
dnl @license GPLWithACException

AC_DEFUN([AC_PROG_DVIPS],[
AC_CHECK_PROGS(dvips,dvips,no)
export dvips;
if test $dvips = "no" ;
then
	AC_MSG_ERROR([Unable to find a dvips application]);
fi;
AC_SUBST(dvips)
])
