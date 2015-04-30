dnl @synopsis _AC_C_IFDEF(MACRO-NAME, ACTION-IF-DEF, ACTION-IF-NOT-DEF)
dnl
dnl Check for the definition of macro MACRO-NAME using the current
dnl language's compiler.
dnl
dnl @category Misc
dnl @author Ludovic Courtès <ludo@chbouib.org>
dnl @version 2004-09-07
dnl @license AllPermissive

AC_DEFUN([_AC_C_IFDEF],
  [AC_COMPILE_IFELSE([#ifndef $1
                      # error "Macro $1 is undefined!"
		      /* For some compilers (eg. SGI's CC), #error is not
		         enough...  */
		      please, do fail
		      #endif],
		     [$2], [$3])])
