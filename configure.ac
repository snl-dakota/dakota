#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Prerequisites.
AC_PREREQ([2.60])
AC_INIT([DAKOTA],[5.1],[dakota-developers@development.sandia.gov])
AC_CONFIG_SRCDIR([src/main.C])
AC_CONFIG_HEADER([dakota_config.h])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([1.9.6 foreign tar-ustar])

# Since DAKOTA includes headers / libraries for packages potentially already
# on the system, make /usr/local/dakota the default install prefix
AC_PREFIX_DEFAULT([/usr/local/dakota])

# Checks for libraries.
AC_CHECK_LIB([rpclib],[xdrmem_create])

# Since we're using libtool, don't build shared libraries!
AC_DISABLE_SHARED

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_F77
AC_ARG_ENABLE([f90],AS_HELP_STRING([--disable-f90],[turn F90 support off]),
	      [enable_f90=$enableval],[enable_f90=yes])
if test "x$enable_f90" = xyes; then
  AC_PROG_FC([],[90])
  if test -z "$FC"; then
    AC_MSG_ERROR([Cannot find a Fortran 90 compiler; please alter your PATH or
		  specify the absolute path to the compiler using the FC
		  environment variable when running configure.  Alternatively,
		  re-run configure with the --disable-f90 flag.])
  else
    AC_DEFINE([DAKOTA_F90],[1],[Macro to enable F90 in Dakota.])
  fi
fi
AM_CONDITIONAL([ENABLE_F90],[test "x$enable_f90" = xyes])
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_LN_S

# SUBVERSION check - needed for updating SVN revision info
AC_CHECK_PROG([HAVE_SVNVERSION],[svnversion],[found],[not_found])
if test "x$HAVE_SVNVERSION" = xfound; then
  SVNVERSION=svnversion 
  AC_SUBST([SVNVERSION])
fi

# Dakota is a C++ code.
AC_LANG([C++])

# Checks for header files.
AC_LANG_PUSH([C])
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([limits.h strings.h unistd.h pdb.h rpc/types.h rpc/xdr.h])
AC_LANG_POP([C])
AC_CHECK_HEADERS([algorithm limits])

# Checks for typedefs, structures, and compiler characteristics.
AC_LANG_PUSH([C])
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_C_RESTRICT
AC_LANG_POP([C])
AC_F77_WRAPPERS
if test "x$enable_f90" = xyes; then
  AC_FC_WRAPPERS
fi
AC_CHECK_TYPE([bool_t,int])
AC_CHECK_TYPE([enum_t,int])
AC_TYPE_PID_T
ACX_BLAS
ACX_LAPACK

# Platform-specific C++ compiler settings.
case "${host_os}" in
    *aix*)
	AX_CXXFLAGS_AIX_OPTION([-qlanglvl=redefmac])
	AM_CONDITIONAL(DAKOTA_HOST_OS_AIX, true)
    ;;
    *solaris*)
	dnl AX_CXXFLAGS_SUN_OPTION([-DSOLARIS])
	AM_CONDITIONAL(DAKOTA_HOST_OS_AIX, false)
    ;;
    *)
	AM_CONDITIONAL(DAKOTA_HOST_OS_AIX, false)
    ;;
esac
case "$ac_cv_cxx_compiler_vendor" in
    ibm)
        dnl the macro is misnamed (should be XLC_OPTION) (LANL machines NOT AIX!)
	AX_CXXFLAGS_AIX_OPTION([-qlanglvl=redefmac])
    ;;
dnl     sgi)
dnl 	AX_CXXFLAGS_IRIX_OPTION([-ptused])
dnl 	AX_CXXFLAGS_IRIX_OPTION([-FE:eliminate_duplicate_inline_copies])
dnl 	AX_CXXFLAGS_IRIX_OPTION([-FE:template_in_elf_section])
dnl 	AX_CXXFLAGS_IRIX_OPTION([-TENV:large_GOT=ON])
dnl 	AX_CXXFLAGS_IRIX_OPTION([-LD_LAYOUT:lgot_buffer=50])
dnl     ;;
    *)
    ;;
esac

# Additional Fortran 90 settings.
if test "x$enable_f90" = xyes; then
  AC_LANG_PUSH([Fortran])
  AC_FC_SRCEXT([f90])
  AC_FC_FIXEDFORM
  AC_LANG_POP([Fortran])
fi

# DAKOTA-specific package checks.
DAK_OPTIONS
DAK_PACKAGES
DAK_METHODS

# Checks for library functions.
AC_LANG_PUSH([C])
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRTOD
AC_CHECK_FUNCS([floor memset pow regcomp sleep sqrt strchr strcspn strerror \
		strpbrk strrchr strstr strtol system usleep wait waitpid])
AC_CHECK_FUNCS([xdrmem_create],[use_xdr="yes"])
if test x"$use_xdr" != xyes; then
  AC_DEFINE([NO_XDR],[1],[XDR isn't usable on this platform.])
fi
AM_CONDITIONAL([USE_XDR],[test x"$use_xdr" = xyes])
AC_LANG_POP([C])

# Set up conditional make check target using CppUnit.
AM_PATH_CPPUNIT([1.9.4], [
  AC_MSG_RESULT(

=== Found CppUnit
=== Setting up make check to run tests...)
found_cppunit=yes
],
  AC_MSG_RESULT([

=== Unable to find a recent enough CppUnit
=== Will not run unit testing!!
===
=== To get CppUnit see http://cppunit.sourceforge.net
]))
AM_CONDITIONAL(HAVE_CPPUNIT, test "$found_cppunit" = "yes")

# Write Makefiles and exit.
AC_CONFIG_FILES([Makefile 
                 Makefile.export
		 packages/Makefile
                 src/Makefile
                 test/Makefile
                 examples/Makefile
                 examples/methods/Makefile
                 examples/script_interfaces/Makefile
                 examples/script_interfaces/generic/Makefile
                 examples/tutorial/Makefile])
AC_OUTPUT
