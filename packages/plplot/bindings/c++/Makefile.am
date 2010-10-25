# bindings/c++/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002, 2003, 2004  Rafael Laboissiere
# Copyright (C) 2002  Alan W. Irwin
#
# This file is part of PLplot.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; version 2 of the License.
#
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with the file; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

libplplot = $(top_builddir)/src/libplplot$(LIB_TAG).la

if enable_cxx

pkginclude_HEADERS = plstream.h

lib_LTLIBRARIES = libplplotcxx@LIB_TAG@.la

endif

AM_CPPFLAGS = -I$(top_srcdir)/include $(INCLTDL)
version = -version-info $(CPP_SOVERSION)

sources = plstream.cc

libplplotcxx@LIB_TAG@_la_SOURCES  = $(sources)
libplplotcxx@LIB_TAG@_la_LDFLAGS  = $(version) $(RPATH) -no-undefined $(libplplot)

MAINTAINERCLEANFILES = Makefile.in
