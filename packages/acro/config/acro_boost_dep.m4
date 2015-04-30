dnl @synopsis ACRO_BOOST_DEP(DEPENDENCE)
dnl
dnl DEPENDENCE - type of Boost dependence 
dnl   in ([], [available], [optional], [required])
dnl
dnl Sets acro_boost_dependence to be the strictest mode encountered
dnl (where [available] < [optional] < [], [required])
AC_DEFUN([ACRO_BOOST_DEPENDENCE],
[
case "x$1" in
	xavailable) option=0;;
	xoptional)  option=1;;
	xnormal)    option=2;;
	xrequired)  option=3;;
	*) AC_MSG_ERROR([acro_boost_dependence: invalid option: $2 not in ([[available]], [[optional]], [[normal]], [[required]])]) ;;
esac
case "x$acro_boost_dependence" in
	x)          ;;
	xavailable) ;;
	xoptional)  if test $option -lt 1; then option=1; fi;;
	xnormal)    if test $option -lt 2; then option=2; fi;;
	xrequired)  if test $option -lt 3; then option=3; fi;;
	*) AC_MSG_ERROR([acro_boost_dependence: invalid value for acro_boost_dependence variable: not in ([[available]], [[optional]], [[normal]], [[required]])]) ;;
esac
case $option in
	0) acro_boost_dependence=available;;
	1) acro_boost_dependence=optional;;
	2) acro_boost_dependence=normal;;
	3) acro_boost_dependence=required;;
esac
])