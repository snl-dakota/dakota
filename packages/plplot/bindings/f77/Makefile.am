# bindings/f77/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002, 2003, 2004  Rafael Laboissiere
# Copyright (C) 2002, 2004  Alan W. Irwin
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# PLplot is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with the file PLplot; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

libplplot = $(top_builddir)/src/libplplot$(LIB_TAG).la

if enable_f77
# order is important here since libplplotf77c@LIB_TAG@.la must be
# created first (since libplplotf77@LIB_TAG@.la depends upon it).
lib_LTLIBRARIES = libplplotf77c@LIB_TAG@.la libplplotf77@LIB_TAG@.la
endif

AM_CPPFLAGS = -I$(top_srcdir)/include $(INCLTDL)
AM_FFLAGS = -I$(srcdir)

version = -version-info $(F77_SOVERSION)

# Keep Fortran and C libraries completely separate so libplplot will know how
# to link them without relying on poorly known (for most platforms)
# FLIBS information detailing how to combine C and fortran source in the
# same library.

fortran_sources = \
	strutil.f \
	sfstubs.h

generated_fortran_sources = \
	sfstubs.f \
	configurable.f

c_sources = \
	plstubs.h \
	sc3d.c \
	sccont.c \
	scstubs.c

# This creates the fortran source part of the fortran interface to libplplot.
# It wraps calls to libplplotf77c@LIB_TAG@.la
libplplotf77@LIB_TAG@_la_SOURCES = $(fortran_sources)
nodist_libplplotf77@LIB_TAG@_la_SOURCES = $(generated_fortran_sources)
libplplotf77@LIB_TAG@_la_LDFLAGS  = $(version) $(RPATH) -no-undefined libplplotf77c@LIB_TAG@.la

# This creates the C source part of the fortran interface to libplplot.
# libplplotf77@LIB_TAG@.la wraps libplplotf77c@LIB_TAG@.la which wraps
# libplplot.  Thus, these two libraries together provide
# a fortran wrapper for the "common" API part of libplplot.
libplplotf77c@LIB_TAG@_la_SOURCES  = $(c_sources)
libplplotf77c@LIB_TAG@_la_LDFLAGS  = $(version) $(RPATH) -no-undefined $(libplplot)

# Must be explicit here with dependencies.  Otherwise solaris "make" chokes.
sfstubs.f: sfstubs.h

# cp for the double-precision case, convert with sed for the
# single-precision case.
if with_double
.fm4.f:
	cp $< $*.f
else
.fm4.f:
	sed -f $(srcdir)/double2single.sed $< > $*.f
endif

EXTRA_DIST = sfstubs.fm4 double2single.sed README.f77API
MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = sfstubs.f

SUFFIXES = .fm4
