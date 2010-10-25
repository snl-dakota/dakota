dnl @synopsis AC_LATEX_PACKAGE_OPT(<package>,<class>,<variable>,<option>)
dnl
dnl Internal macro for other LaTeX-related tests.
dnl
dnl @category LaTeX
dnl @author Mathieu Boretti <boretti@bss-network.com>
dnl @version 2005-01-21
dnl @license GPLWithACException

AC_DEFUN([_AC_LATEX_TEST],[
AC_REQUIRE([AC_PROG_LATEX])
rm -rf .tmps_latex
mkdir .tmps_latex
cd .tmps_latex
ifelse($#,2,[
$2="no"; export $2;
cat > testconf.tex << \EOF
$1
EOF
],$#,3,[
echo "\\documentclass{$3}" > testconf.tex
cat >> testconf.tex << \EOF
$1
EOF
],$#,4,[
echo "\\documentclass{$3}" > testconf.tex
echo "\\usepackage{$4}" > testconf.tex
cat >> testconf.tex << \EOF
$1
])
cat testconf.tex | $latex 2>&1 1>/dev/null && $2=yes; export $2;
cd ..
rm -rf .tmps_latex
])
