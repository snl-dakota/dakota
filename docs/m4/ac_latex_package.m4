dnl @synopsis AC_LATEX_PACKAGE(<package>,<class>,<variable>)
dnl
dnl This macro test if package in <class> exists and set <variable> to
dnl the right value
dnl
dnl  AC_LATEX_PACKAGE(varioref,book,vbook)
dnl  should set $vbook="yes"
dnl
dnl  AC_LATEX_PACKAGE(xyz,book,vbook)
dnl  should set $vbook="no"
dnl
dnl @category LaTeX
dnl @author Mathieu Boretti boretti@bss-network.com
dnl @version 2005-01-21
dnl @license GPLWithACException

AC_DEFUN([AC_LATEX_PACKAGE],[
if test "$[ac_cv_latex_class_]translit($2,[-],[_])" = "" ;
then
	AC_LATEX_CLASS($2,boretti_classesansparametre)
	export boretti_classesansparametre;
else
	boretti_classesansparametre=$[ac_cv_latex_class_]translit($2,[-],[_]) ;
	export boretti_classesansparemetre;
fi;
if test $boretti_classesansparametre = "no" ;
then
    AC_MSG_ERROR([Unable to find $2 class])
fi
AC_CACHE_CHECK([for $1 in class $2],[ac_cv_latex_]translit($1,[-],[_])[_]translit($2,[-],[_]),[
_AC_LATEX_TEST([
\documentclass{$2}
\usepackage{$1}
\begin{document}
\end{document}
],[ac_cv_latex_]translit($1,[-],[_])[_]translit($2,[-],[_]))
])
$3=$[ac_cv_latex_]translit($1,[-],[_])[_]translit($2,[-],[_]); export $3;
AC_SUBST($3)
])
