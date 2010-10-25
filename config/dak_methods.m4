dnl Methods

AC_DEFUN([DAK_METHODS],[
  dnl ACRO package checks.
  AC_ARG_WITH([acro],AS_HELP_STRING([--without-acro],[turn ACRO support off]),
	      [with_acro=$withval],[with_acro=yes])
  if test "x$with_acro" = xyes -a -d $srcdir/packages/acro; then
    case "${host_os}" in
	*aix*)
		AX_CXXFLAGS_AIX_OPTION([-DAIX_CC])dnl AIX-specific Acro code.
	;;
	*irix*)
		AX_CXXFLAGS_IRIX_OPTION([-DSGI_CC])dnl SGI-specific Acro code.
	;;
	*osf*)
		AX_CXXFLAGS_OSF_OPTION([-DOSF_CC])dnl OSF-specific Acro code.
	;;
	*solaris*)
		dnl Solaris-specific Acro code.
		AX_CXXFLAGS_SUN_OPTION([-DSOLARIS_CC])
	;;
	*)
	;;
    esac
    AC_CONFIG_SUBDIRS([packages/acro])
    AC_DEFINE([DAKOTA_3PO],[1],[Macro to handle code which depends on 3PO.])
    AC_DEFINE([DAKOTA_UTILIB],[1],
              [Macro to handle code which depends on UTILIB.])
    AC_DEFINE([DAKOTA_COLINY],[1],
              [Macro to handle code which depends on COLINY.])
    dnl AC_DEFINE([DAKOTA_AMPL],[1],[Macro to handle code which depends on AMPL.])
    AC_DEFINE([USING_3PO],[1],[Macro to handle code which depends on 3PO.])
    MAYBE_ACRO=acro
  else
    MAYBE_ACRO=
  fi
  AC_SUBST([MAYBE_ACRO])
  AM_CONDITIONAL([WITH_ACRO],[test "x$with_acro" = xyes -a \
				   -d $srcdir/packages/acro])

  dnl CONMIN package check.
  AC_ARG_WITH([conmin],AS_HELP_STRING([--without-conmin],
				      [turn CONMIN support off]),
	      [with_conmin=$withval],[with_conmin=yes])
  if test "x$with_conmin" = xyes; then
    AC_CONFIG_SUBDIRS([packages/CONMIN])
    AC_DEFINE([DAKOTA_CONMIN],[1],
	      [Macro to handle code which depends on CONMIN.])
  fi
  AM_CONDITIONAL([WITH_CONMIN],[test "x$with_conmin" = xyes])

  dnl DDACE package check.
  AC_ARG_WITH([ddace],AS_HELP_STRING([--without-ddace],
				     [turn DDACE support off]),
	      [with_ddace=$withval],[with_ddace=yes])
  if test "x$with_ddace" = xyes -a -d $srcdir/packages/DDACE; then
    AC_CONFIG_SUBDIRS([packages/DDACE])
    AC_DEFINE([DAKOTA_DDACE],[1],
	      [Macro to handle code which depends on DDACE.])
    MAYBE_DDACE=DDACE
  else
    MAYBE_DDACE=
  fi
  AC_SUBST([MAYBE_DDACE])
  AM_CONDITIONAL([WITH_DDACE],[test "x$with_ddace" = xyes -a \
				    -d $srcdir/packages/DDACE])

  dnl DL_SOLVER package check (toggles NIDR DL features and DL_SOLVER).
  AC_ARG_WITH([dl_solver],
              AS_HELP_STRING([--with-dl_solver], [turn DL_SOLVER support on]),
	      [with_dl_solver=$withval], [with_dl_solver=no])
  if test "x$with_dl_solver" = xyes ; then
    case "${host_os}" in
      *mingw*)
	AC_MSG_CHECKING([for LoadLibrary])
        dnl crude workaround for MinGW since we're not using libtool for DL
	AC_LINK_IFELSE(AC_LANG_PROGRAM( [#include <windows.h>],
                                        [LoadLibrary (NULL);]  ),
                       [],
                       AC_MSG_ERROR([Dynamically linked solvers (dl_solvers) require LoadLibrary]))
	AC_MSG_RESULT([yes])
	;;

      *)
        ACX_FUNC_DLOPEN([], 
    	  	        AC_MSG_ERROR([Dynamically linked solvers (dl_solvers) require dlopen]))
	;;

    esac

    AC_DEFINE([DAKOTA_DL_SOLVER],[1],
	      [Macro to handle code that depends on DL_SOLVER.])
  dnl else -DNO_NIDR_DYNLIB set in src/Makefile.am
  fi
  AM_CONDITIONAL([WITH_DL_SOLVER],
                 [test "x$with_dl_solver" = xyes ])

  dnl DOT package check.
  AC_ARG_WITH([dot],AS_HELP_STRING([--without-dot],[turn DOT support off]),
	      [with_dot=$withval],[with_dot=yes])
  if test "x$with_dot" = xyes -a \
          -e $srcdir/packages/DOT/dbl_prec/dot1.f; then
    AC_CONFIG_SUBDIRS([packages/DOT])
    AC_DEFINE([DAKOTA_DOT],[1],[Macro to handle code which depends on DOT.])
  fi
  AM_CONDITIONAL([WITH_DOT],
                 [test "x$with_dot" = xyes -a \
                       -e $srcdir/packages/DOT/dbl_prec/dot1.f])

  dnl FSUDACE package check.
  AC_ARG_WITH([fsudace],AS_HELP_STRING([--without-fsudace],
				       [turn FSUDace support off]),
	      [with_fsudace=$withval],[with_fsudace=yes])
  if test "x$with_fsudace" = xyes; then
    AC_DEFINE([DAKOTA_FSUDACE],[1],
	      [Macro to handle code which depends on FSUDACE.])
    AC_CONFIG_SUBDIRS([packages/FSUDace])
  fi
  AM_CONDITIONAL([WITH_FSUDACE],[test "x$with_fsudace" = xyes])

  dnl GPMSA package check.  (REQUIRES --with-gsl)
  AC_ARG_WITH([gpmsa],AS_HELP_STRING([--with-gpmsa],
				       [turn GPL package GPMSA on]),
	      [with_gpmsa=$withval],[with_gpmsa=no])
  if test "x$with_gpmsa" = xyes; then
    if test ! -f $srcdir/packages/gpmsa/GPmodel.h; then
      AC_MSG_ERROR([Could not find packages/gpmsa/GPmodel.h])
    fi
    if test "x$acx_gsl_ok" != xyes; then
      AC_MSG_ERROR([GPMSA requires configuring --with-gsl])
    fi    
    AC_MSG_NOTICE([GNU GPL package GPMSA enabled])
    AC_MSG_NOTICE([NOTE: your build includes GNU GPL (binary) library GPMSA!])
    AC_DEFINE([DAKOTA_GPMSA],[1],
    	      [Macro to handle code which depends on GPMSA.])
    dnl Nothing to do in GPMSA for now
    dnl AC_CONFIG_SUBDIRS([packages/gpmsa])
  fi
  AM_CONDITIONAL([WITH_GPMSA],[test "x$with_gpmsa" = xyes])

  dnl QUESO package check.  (REQUIRES --with-gsl)
  AC_ARG_WITH([queso],AS_HELP_STRING([--with-queso],
				       [turn QUESO support on]),
	      [with_queso=$withval],[with_queso=no])
  if test "x$with_queso" = xyes; then
    if test ! -f $srcdir/packages/queso/somefilename.h; then
      AC_MSG_ERROR([Could not find packages/queso/somefilename.h])
    fi
    if test "x$acx_gsl_ok" != xyes; then
      AC_MSG_ERROR([QUESO requires configuring --with-gsl])
    fi    
    AC_MSG_NOTICE([QUESO: enabled.])
    AC_DEFINE([DAKOTA_QUESO],[1],
    	      [Macro to handle code which depends on QUESO.])
    dnl Nothing to do in QUESO for now
    dnl AC_CONFIG_SUBDIRS([packages/queso])
  fi
  AM_CONDITIONAL([WITH_QUESO],[test "x$with_queso" = xyes])

  dnl HOPSPACK/APPSPACK package check.
  AC_ARG_WITH([appspack],AS_HELP_STRING([--without-appspack],
				        [turn APPSPACK support off]),
	      [with_appspack=$withval],[with_appspack=yes])
  if test "x$with_appspack" = xyes -a -d $srcdir/packages/hopspack; then
    AC_CONFIG_SUBDIRS([packages/hopspack])
    AC_DEFINE([DAKOTA_APPS],[1],
	      [Macro to handle code which depends on APPSPACK.])
    MAYBE_APPSPACK=hopspack
  else
    MAYBE_APPSPACK=
  fi
  AC_SUBST([MAYBE_APPSPACK])
  AM_CONDITIONAL([WITH_APPSPACK],[test "x$with_appspack" = xyes -a \
				    -d $srcdir/packages/hopspack])

  dnl JEGA package check.
  AC_ARG_WITH([jega],AS_HELP_STRING([--without-jega],[turn JEGA support off]),
	      [with_jega=$withval],[with_jega=yes])
  if test "x$with_jega" = xyes -a -d $srcdir/packages/JEGA; then
    AC_CONFIG_SUBDIRS([packages/JEGA])
    AC_DEFINE([DAKOTA_JEGA],[1],
	      [Macro to handle code which depends on JEGA.])
    MAYBE_JEGA=JEGA
  else
    MAYBE_JEGA=
  fi
  AC_SUBST([MAYBE_JEGA])
  AM_CONDITIONAL([WITH_JEGA],[test "x$with_jega" = xyes -a \
				    -d $srcdir/packages/JEGA])

  dnl NCSUOpt package check.
  AC_ARG_WITH([ncsu],AS_HELP_STRING([--without-ncsu],
				    [turn NCSUOpt support off]),
	      [with_ncsu=$withval],[with_ncsu=yes])
  if test "x$with_ncsu" = xyes; then
    AC_CONFIG_SUBDIRS([packages/NCSUOpt])
    AC_DEFINE([DAKOTA_NCSU],[1],
	      [Macro to handle code which depends on NCSUOpt.])
  fi
  AM_CONDITIONAL([WITH_NCSU],
                 [test "x$with_ncsu" = xyes])

  dnl NL2SOL package check.
  AC_ARG_WITH([nl2sol],AS_HELP_STRING([--without-nl2sol],
				      [turn NL2SOL support off]),
	      [with_nl2sol=$withval],[with_nl2sol=yes])
  if test "x$with_nl2sol" = xyes; then
    AC_CONFIG_SUBDIRS([packages/NL2SOL])
    AC_DEFINE([DAKOTA_NL2SOL],[1],
    	      [Macro to handle code which depends on NL2SOL.])
  fi
  AM_CONDITIONAL([WITH_NL2SOL],[test "x$with_nl2sol" = xyes])

  dnl NLPQL package check.
  AC_ARG_WITH([nlpql],AS_HELP_STRING([--without-nlpql],
				     [turn NLPQL support off]),
	      [with_nlpql=$withval],[with_nlpql=yes])
  if test "x$with_nlpql" = xyes -a \
	  -e $srcdir/packages/NLPQL/NLPQLP.f; then
    AC_CONFIG_SUBDIRS([packages/NLPQL])
    AC_DEFINE([DAKOTA_NLPQL],[1],
	      [Macro to handle code which depends on NLPQL.])
  fi
  AM_CONDITIONAL([WITH_NLPQL],
                 [test "x$with_nlpql" = xyes -a \
	               -e $srcdir/packages/NLPQL/NLPQLP.f])

  dnl NPSOL package check.
  AC_ARG_WITH([npsol],AS_HELP_STRING([--without-npsol],
				     [turn NPSOL support off]),
	      [with_npsol=$withval],[with_npsol=yes])
  if test "x$with_npsol" = xyes -a \
          -e $srcdir/packages/NPSOL/npsolsubs.f; then
    AC_CONFIG_SUBDIRS([packages/NPSOL])
    AC_DEFINE([DAKOTA_NPSOL],[1],
    [Macro to handle code which depends on NPSOL.])
  fi
  AM_CONDITIONAL([WITH_NPSOL],
                 [test "x$with_npsol" = xyes -a \
                       -e $srcdir/packages/NPSOL/npsolsubs.f])

  dnl OPTPP package check.
  AC_ARG_WITH([optpp],AS_HELP_STRING([--without-optpp],
				     [turn OPTPP support off]),
	      [with_optpp=$withval],[with_optpp=yes])
  if test "x$with_optpp" = xyes -a -d $srcdir/packages/OPTPP; then
    AC_CONFIG_SUBDIRS([packages/OPTPP])
    AC_DEFINE([DAKOTA_OPTPP],[1],
	      [Macro to handle code which depends on OPTPP.])
    AC_DEFINE([DAKOTA_NEWMAT],[1],
	      [Macro to handle code which depends on NEWMAT.])
    MAYBE_OPTPP=OPTPP
  else
    MAYBE_OPTPP=
  fi
  AC_SUBST([MAYBE_OPTPP])
  AM_CONDITIONAL([WITH_OPTPP],[test "x$with_optpp" = xyes -a \
				    -d $srcdir/packages/OPTPP])
  dnl PSUADE package check.
  AC_ARG_WITH([psuade],AS_HELP_STRING([--without-psuade],
				      [turn PSUADE support off]),
	      [with_psuade=$withval],[with_psuade=yes])
  if test "x$with_psuade" = xyes -a \
          -e $srcdir/packages/PSUADE/MOATSampling.cpp; then
    AC_CONFIG_SUBDIRS([packages/PSUADE])
    AC_DEFINE([HAVE_PSUADE],[1],
      	      [Macro to handle code which depends on PSUADE.])
  fi
  AM_CONDITIONAL([WITH_PSUADE],
                 [test "x$with_psuade" = xyes -a \
	               -e $srcdir/packages/PSUADE/MOATSampling.cpp])
])
