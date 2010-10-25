dnl @synopsis AC_LATEX_CLASS_BOOK
dnl
dnl same as AC_LATEX_CLASS(book,book)
dnl
dnl @category LaTeX
dnl @author Mathieu Boretti boretti@bss-network.com
dnl @version 2005-01-21
dnl @license GPLWithACException

AC_DEFUN([AC_LATEX_CLASS_BOOK],[
AC_LATEX_CLASS(book,book)
if test $book = "no";
then
    AC_MSG_ERROR([Unable to find the book class])
fi
])
