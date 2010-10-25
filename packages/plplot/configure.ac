dnl configure.ac for PLplot  -*- autoconf -*-
dnl
dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright (C) 1994, 2004  Maurice LeBrun
dnl Copyright (C) 2002, 2003, 2004  Rafael Laboissiere
dnl Copyright (C) 2002, 2003, 2004  Alan W. Irwin
dnl Copyright (C) 2003, 2004  Joao Cardoso
dnl Copyright (C) 2004  Andrew Roach
dnl Copyright (C) 2005  Thomas J. Duck
dnl
dnl This file is part of PLplot.
dnl
dnl PLplot is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU Library General Public License as published
dnl by the Free Software Foundation; version 2 of the License.
dnl
dnl PLplot is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU Library General Public License for more details.
dnl
dnl You should have received a copy of the GNU Library General Public License
dnl along with the file PLplot; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

dnl Note: to recover legacy behavior as regards (site-specific) defaults
dnl files, configure using:
dnl
dnl $ CONFIG_SITE=~/config/cf_plplot.in ./configure


dnl =======================================================================
dnl Autoconf/automake initializations
dnl =======================================================================

dnl Package version number
dnl [ If changing the line below by hand, please keep the format up to and
dnl   including the version number, otherwise it will fool up the
dnl   bootstrap.sh script.  By the way, the line below can be changed using
dnl   options --version and --date-version of that script. ]

AC_INIT(plplot, 5.5.2, plplot-devel@lists.sourceforge.net)
AC_REVISION($Revision: 5036 $)

dnl Date of release.  This can be set by hand, but the prefered method
dnl is by specifying one of the option --version of --date-version to
dnl bootstrap.sh

RELEASE_DATE=2005-04-20
AC_SUBST(RELEASE_DATE)

dnl Check if we are in the right directory (this used to be done by
dnl AC_INIT)

AC_CONFIG_SRCDIR(src/plcore.c)

dnl Use cf dir instead of the top dir for all autotools-related files

AC_CONFIG_AUX_DIR(cf)
AC_CONFIG_MACRO_DIR(cf)

dnl Configuration headers

AC_CONFIG_HEADERS([include/plplot_config.h include/plConfig.h include/plDevs.h])

dnl Automake initialization

AM_INIT_AUTOMAKE(1.8.2)

dnl Default installation prefix

AC_PREFIX_DEFAULT(/usr/local)
test "$prefix" = NONE && prefix=$ac_default_prefix
test "$exec_prefix" = NONE && exec_prefix=$prefix


dnl =======================================================================
dnl PLplot library sonames and soversions
dnl =======================================================================

dnl Version numbers for the shared C libraries.

SOVERSION=10:0:1
AC_SUBST(SOVERSION)

dnl Version number for the shared C++ library.  After version 5.2.1 was
dnl released, the C++ bindings were changed and backward incompatibilities
dnl were introduced.  Hence the different strings for SOVERSION and
dnl CPP_SOVERSION.

CPP_SOVERSION=10:0:1
AC_SUBST(CPP_SOVERSION)

dnl Version number for the shared F77 library.  Starting with release 5.3.1
dnl of PLplot, the Fortran shared library will have a separate SOVERSION
dnl than that of the C PLplot library.  This is necessary because there is a
dnl specialized Fortran API.

F77_SOVERSION=9:0:0
AC_SUBST(F77_SOVERSION)

dnl Version number for the CSIRO libraries distributed in lib/csa and
dnl lib/nn.  This information was buried into lib/{csa,nn}/Makefile.am and it
dnl is made explicit below.  It may happen that, in the future, we will need
dnl separate soversions for libcsirocsa and libcsironn, but let us keep an
dnl unique soversion for now.

CSIRO_SOVERSION=0:0:0
AC_SUBST(CSIRO_SOVERSION)

dnl According to man package, PLPLOT_TCL_VERSION (used in the various
dnl pkgIndex.tcl scripts) should be strictly numbers alternativing with
dnl decimal points with the first number (the major number) signifying
dnl incompatible changes in the scripts.  To keep our life simple, let us
dnl use the major number from the package version for now.
dnl N.B. both LIBRARY_VERSION and PLPLOT_TCL_VERSION are used for separate
dnl purposes in the various configured pkgIndex.tcl scripts.

PLPLOT_TCL_VERSION=5
AC_SUBST(PLPLOT_TCL_VERSION)


dnl =======================================================================
dnl Check for programs
dnl =======================================================================

dnl Require C compile

AC_PROG_CC
AM_CONDITIONAL(cc_is_gcc, [test "$GCC" = yes])


dnl =======================================================================
dnl Libraires
dnl =======================================================================

dnl Add libm to LIBS, as some non linux unices might require this.

AC_CHECK_LIB(m, sin)

m4_include(cf/pthread.ac)
m4_include(cf/freetype.ac)
m4_include(cf/unicode.ac)
m4_include(cf/csiro.ac)


dnl =======================================================================
dnl Headers
dnl =======================================================================

AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h termios.h)
AC_HEADER_SYS_WAIT
AC_HEADER_DIRENT

dnl Check for inclusion of sys/types.h before dirent.h in broken MacOSX
dnl systems

case "$host_os" in
    darwin*)
        AC_DEFINE(NEED_SYS_TYPE_H, [], [Include sys/type.h if needed])
    ;;
esac


dnl =======================================================================
dnl Typedefs
dnl =======================================================================

dnl Some X11 headers require "caddr_t" even on systems that claim POSIX.1
dnl compliance, which is illegal.  This makes it impossible to compile
dnl programs that include X11 headers if _POSIX_SOURCE is defined.  I work
dnl around this potential problem by just defining caddr_t to 'char *' on all
dnl systems (unless it is set already), whether it will be needed or not.

AC_CHECK_TYPE(caddr_t, char *)

dnl The following is primarily for use in drivers/get-drv-info.c

AC_TYPE_SIGNAL


dnl =======================================================================
dnl Functions
dnl =======================================================================

AC_FUNC_VFORK
AC_CHECK_FUNC(popen, AC_DEFINE(HAVE_POPEN, [],
                               [Define if popen is available]))
AC_CHECK_FUNC(usleep, AC_DEFINE(HAVE_USLEEP, [],
                               [Define if usleep is available]))
AC_CHECK_FUNC(isinf, AC_DEFINE(HAVE_ISINF, [],
                               [Define if isinf is available]))
AC_CHECK_FUNC(finite, AC_DEFINE(HAVE_FINITE, [],
                               [Define if finite is available]))
AC_CHECK_FUNC(isnan,
    [AC_DEFINE(HAVE_ISNAN, [], [Define if isnan is available])
     have_isnan=yes],
    [have_isnan=no])

dnl =======================================================================
dnl Compilation and build options (PLFLT, rpath, install locations)
dnl =======================================================================

m4_include(cf/double.ac)
m4_include(cf/rpath.ac)
m4_include(cf/instdirs.ac)


dnl =======================================================================
dnl Language bindings
dnl =======================================================================

m4_include(cf/python.ac)
m4_include(cf/java.ac)
m4_include(cf/swig.ac)
m4_include(cf/octave.ac)
m4_include(cf/tcl.ac)
m4_include(cf/c++.ac)
m4_include(cf/f77.ac)
m4_include(cf/pdl.ac)


dnl =======================================================================
dnl pkg-config support
dnl =======================================================================

m4_include(cf/pkg-config.ac)


dnl =======================================================================
dnl Device drivers
dnl =======================================================================

m4_include(cf/drivers-init.ac)
m4_include(cf/cgm.ac)
m4_include(cf/gcw.ac)
m4_include(cf/gd.ac)
m4_include(cf/gnome.ac)
m4_include(cf/linuxvga.ac)
m4_include(cf/xwin.ac)
m4_include(cf/tk.ac)
m4_include(cf/ps.ac)
m4_include(cf/wingcc.ac)
m4_include(cf/aqt.ac)
m4_include(cf/drivers-finish.ac)


dnl Libtool configuration
dnl (moved down to prevent error messages about circular definitions)

m4_include(cf/libtool.ac)


dnl =======================================================================
dnl Other parts of PLplot (test suite, DocBook manual, pkg-config support)
dnl =======================================================================

m4_include(cf/test.ac)
m4_include(cf/docbook.ac)


dnl =======================================================================
dnl Build configuration files and show summary
dnl =======================================================================

dnl Notice that there are other calls to AC_CONFIG_FILES in the cf/*.ac
dnl files.

dnl Note: Must build the python/gcw binding *after* building drivers; hence
dnl separate entry.  TJD

AC_CONFIG_FILES([
    Makefile
    fonts/Makefile
    src/Makefile
    include/Makefile
    data/Makefile
    bindings/Makefile
    bindings/python/gcw/Makefile
    cf/Makefile
    drivers/Makefile
    examples/Makefile
    examples/c/Makefile
    examples/c/Makefile.examples
    examples/perl/Makefile
    scripts/Makefile
    doc/Makefile
    scripts/plplot-config
])

AC_OUTPUT

m4_include(cf/summary.ac)
