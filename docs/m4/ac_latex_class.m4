dnl @synopsis AC_LATEX_CLASSE(<class1>,<var>)
dnl
dnl Test if class1 exists and set $var to the right value
dnl
dnl  AC_LATEX_CLASSES([book],book)
dnl  should set $book="yes"
dnl
dnl  AC_LATEX_CLASSES(allo,book)
dnl  should set $book="no"
dnl
dnl @category LaTeX
dnl @author Mathieu Boretti boretti@bss-network.com
dnl @version 2005-01-21
dnl @license GPLWithACException

AC_DEFUN([AC_LATEX_CLASS],[
AC_CACHE_CHECK([for class $1],[ac_cv_latex_class_]translit($1,[-],[_]),[
_AC_LATEX_TEST([
\begin{document}
\end{document}
],[ac_cv_latex_class_]translit($1,[-],[_]),$1)
])
$2=$[ac_cv_latex_class_]translit($1,[-],[_]) ; export $2;
AC_SUBST($2)
])
