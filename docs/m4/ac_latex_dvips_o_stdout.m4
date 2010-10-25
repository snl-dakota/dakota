dnl @synopsis AC_LATEX_DVIPS_O_STDOUT
dnl
dnl test if dvips -o- works. If so, set $dvips_o_stdout to yes else to
dnl no
dnl
dnl @category LaTeX
dnl @author Mathieu Boretti boretti@bss-network.com
dnl @version 2005-01-21
dnl @license GPLWithACException

AC_DEFUN([AC_LATEX_DVIPS_O_STDOUT],[
AC_REQUIRE([AC_LATEX_CLASS_BOOK])
AC_CACHE_CHECK([for option -o- in dvips],ac_cv_dvips_o_stdout,[
rm -rf .dvips
mkdir .dvips
cd .dvips
cat > test.tex << EOF
\documentclass{book}
\begin{document}
Test
\end{document}
EOF
$latex test.tex 1>/dev/null 2>&1
ac_cv_dvips_o_stdout="no"; export ac_cv_dvips_o_stdout;
$dvips -o- test.dvi   1>/dev/null 2>&1 && ac_cv_dvips_o_stdout="yes"; export ac_cv_dvips_o_stdout
cd ..
])
DVIPS_O_STDOUT=$ac_cv_dvips_o_stdout; export DVIPS_O_STDOUT;
if test $DVIPS_O_STDOUT = "no" ;
then
    AC_MSG_ERROR(Unable to find the option -o- in dvips)
fi
AC_SUBST(DVIPS_O_STDOUT)
])
