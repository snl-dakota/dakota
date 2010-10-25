# bindings/tcl/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002, 2003, 2004  Alan W. Irwin
# Copyright (C) 2003, 2004  Rafael Laboissiere
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; version 2 of the License.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

tcldir = $(TCL_DIR)

libplplot = $(top_builddir)/src/libplplot$(LIB_TAG).la
libmatrix = libtclmatrix$(LIB_TAG).la


if enable_tcl
generated_scripts = plplot.tcl
tcl_DATA = $(generated_scripts) tclIndex

if enable_itcl
mkindex = $(top_srcdir)/scripts/mktclIndex -itcl
else
mkindex = $(top_srcdir)/scripts/mktclIndex -tcl
endif

lib_LTLIBRARIES = libtclmatrix@LIB_TAG@.la libplplottcltk@LIB_TAG@.la

endif

plplot.tcl: $(top_srcdir)/bindings/tk/plplot.tcl
	rm -f plplot.tcl
	cp $(top_srcdir)/bindings/tk/plplot.tcl plplot.tcl

tclIndex: $(generated_scripts)
	$(mkindex)

pkginclude_HEADERS = pltcl.h tclMatrix.h

tclgenfiles = tclgen.h tclgen_s.h tclgen.c

PLTCLGEN = $(top_srcdir)/bindings/tcl/pltclgen
$(tclgenfiles): $(PLTCLGEN) plapi.tpl tclcmd.tpl
	$(PERL) $(PLTCLGEN) $(top_srcdir)/bindings/tcl

# Build these files first to sort out dependency issues:
$(top_srcdir)/bindings/tcl/tclAPI.c: $(tclgenfiles)

# uninstalled header files that should be distributed
noinst_HEADERS = $(tclgenfiles)

# ITKINCCMD same as TCLINCCMD so not defined in top-level configure.in
AM_CPPFLAGS = -I$(top_srcdir)/include $(INCLTDL) $(TCLINCCMD) $(ITCLINCCMD) $(TKINCCMD)
version = -version-info $(SOVERSION)

libtclmatrix@LIB_TAG@_la_SOURCES  = \
	tclMatrix.c \
	matrixInit.c

libtclmatrix@LIB_TAG@_la_LDFLAGS  = $(version) $(RPATH) -no-undefined \
	$(TCLLIBCMD) -lm

if enable_tcl
tclsources = \
	$(top_srcdir)/bindings/tcl/tclAPI.c \
	$(top_srcdir)/bindings/tcl/tclMain.c
endif

if enable_tk
tksources = \
	$(top_srcdir)/bindings/tk/Pltk_Init.c \
	$(top_srcdir)/bindings/tk/plframe.c \
	$(top_srcdir)/bindings/tk/plr.c \
	$(top_srcdir)/bindings/tk/tcpip.c \
	$(top_srcdir)/bindings/tk/tkMain.c
endif

## This library should probably be broken up further into at least a tcl
## and tk component.

libplplottcltk@LIB_TAG@_la_SOURCES  = $(tclsources) $(tksources)

## library ordering is important here
libplplottcltk@LIB_TAG@_la_LDFLAGS  = \
	$(version) \
	$(RPATH) \
	-no-undefined \
	$(libplplot) \
	$(libmatrix) \
	$(ITKLIBCMD) \
	$(TKLIBCMD) \
        $(ITCLLIBCMD) \
	$(TCLLIBCMD) \
	$(X_LIBS) -lX11 -lm

CLEANFILES = $(generated_scripts) tclIndex
MAINTAINERCLEANFILES = $(tclgenfiles) Makefile.in

### FIXME: Brute force inclusion in dist tarball.  The files below may
### be treated in a different way for installation [RL, 2003-03-06]
EXTRA_DIST = \
	pltclgen \
	plapi.tpl \
	tclcmd.tpl \
	README.tclAPI \
	plitclgen
