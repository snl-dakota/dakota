# examples/c++/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002, 2004  Alan W. Irwin
# Copyright (C) 2004  Rafael Labossiere
# Copyright (C) 2004  Andrew Ross
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

sources = \
	x01cc.cc \
	x01.cc \
	x02.cc \
	x03.cc \
	x04.cc \
	x05.cc \
	x06.cc \
	x07.cc \
	x08.cc \
	x09.cc \
	x10.cc \
	x11.cc \
	x12.cc \
	x13.cc \
	x14.cc \
	x15.cc \
	x16.cc \
	x17.cc \
	x18.cc \
	x19.cc \
	x20.cc \
	x21.cc \
	x22.cc \
	x23.cc \
	x24.cc

if enable_cxx

AM_CPPFLAGS = -I$(top_srcdir)/include $(INCLTDL) -I$(top_srcdir)/bindings/c++
examples_cxxdir = $(DATA_DIR)/examples/c++
examples_cxx_DATA = \
	$(sources) \
	Makefile.examples

libplplot = $(top_builddir)/src/libplplot$(LIB_TAG).la
libplplotcxx = $(top_builddir)/bindings/c++/libplplotcxx$(LIB_TAG).la

check_PROGRAMS = \
	x01cc \
	x01 \
	x02 \
	x03 \
	x04 \
	x05 \
	x06 \
	x07 \
	x08 \
	x09 \
	x10 \
	x11 \
	x12 \
	x13 \
	x14 \
	x15 \
	x16 \
	x17 \
	x18 \
	x19 \
	x20 \
	x21 \
	x22 \
	x23 \
	x24

check_DATA = lena.pgm

lena.pgm:
	rm -f lena.pgm
	$(LN_S) ../c/lena.pgm lena.pgm


x01cc_SOURCES = x01cc.cc
x01_SOURCES = x01.cc
x02_SOURCES = x02.cc
x03_SOURCES = x03.cc
x04_SOURCES = x04.cc
x05_SOURCES = x05.cc
x06_SOURCES = x06.cc
x07_SOURCES = x07.cc
x08_SOURCES = x08.cc
x09_SOURCES = x09.cc
x10_SOURCES = x10.cc
x11_SOURCES = x11.cc
x12_SOURCES = x12.cc
x13_SOURCES = x13.cc
x14_SOURCES = x14.cc
x15_SOURCES = x15.cc
x16_SOURCES = x16.cc
x17_SOURCES = x17.cc
x18_SOURCES = x18.cc
x19_SOURCES = x19.cc
x20_SOURCES = x20.cc
x21_SOURCES = x21.cc
x22_SOURCES = x22.cc
x23_SOURCES = x23.cc
x24_SOURCES = x24.cc

LDADD = $(libplplot) $(libplplotcxx)

# enable_cxx
endif

install-data-hook:
if enable_cxx
	( cd $(DESTDIR)$(examples_cxxdir) ; \
	  mv -f Makefile.examples Makefile ; \
	  rm -f lena.pgm                   ; \
	  $(LN_S) ../c/lena.pgm lena.pgm )
endif

EXTRA_DIST = $(sources) Makefile.examples.in README.c++demos
CLEANFILES = lena.pgm
DISTCLEANFILES = Makefile.examples
MAINTAINERCLEANFILES = Makefile.in
