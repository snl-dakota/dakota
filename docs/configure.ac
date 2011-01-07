#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Preliminaries.
AC_PREREQ([2.59])
AC_INIT([dakota-docs],[5.0],[dakota-developers@development.sandia.gov])
AM_INIT_AUTOMAKE([1.9 foreign tar-ustar])

# Checks for programs.
AC_PROG_LATEX
AC_PROG_PDFLATEX
AC_PROG_BIBTEX
AC_PROG_DVIPS
AC_PROG_LN_S

# Checks for typedefs, structures, and compiler characteristics.
AC_LATEX_CLASS_BOOK
AC_LATEX_DVIPS_O_STDOUT

# Write Makefiles and exit.
AC_CONFIG_FILES([Makefile
                 latex-user/Makefile
                 latex-theory/Makefile])
AC_OUTPUT
