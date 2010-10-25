# examples/f77/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002, 2004  Alan W. Irwin
# Copyright (C) 2004  Rafael Laboissiere
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

# x19f.f excluded from this list because the plmap and plmeridians fortran
# API is not implemented as part of the common API (and probably won't be
# for a long time because of having to work around the mapform type of
# argument for these functions).

f77_examples = \
	x01f.f \
	x02f.f \
	x03f.f \
	x04f.f \
	x05f.f \
	x06f.f \
	x07f.f \
	x08f.f \
	x09f.f \
	x10f.f \
	x11f.f \
	x12f.f \
	x13f.f \
	x14f.f \
	x15f.f \
	x16f.f \
	x16af.f \
	x17f.f \
	x18f.f \
	x22f.f

AM_CPPFLAGS = -I$(top_srcdir)/include $(INCLTDL)

f77m4_sources = \
	x01f.fm4 \
	x02f.fm4 \
	x03f.fm4 \
	x04f.fm4 \
	x05f.fm4 \
	x06f.fm4 \
	x07f.fm4 \
	x08f.fm4 \
	x09f.fm4 \
	x10f.fm4 \
	x11f.fm4 \
	x12f.fm4 \
	x13f.fm4 \
	x14f.fm4 \
	x15f.fm4 \
	x16f.fm4 \
	x16af.fm4 \
	x17f.fm4 \
	x18f.fm4 \
	x22f.fm4 

# Need explicit dependencies for Solaris make.
x01f.f: x01f.fm4 $(top_builddir)/config.status

x02f.f: x02f.fm4 $(top_builddir)/config.status

x03f.f: x03f.fm4 $(top_builddir)/config.status

x04f.f: x04f.fm4 $(top_builddir)/config.status

x05f.f: x05f.fm4 $(top_builddir)/config.status

x06f.f: x06f.fm4 $(top_builddir)/config.status

x07f.f: x07f.fm4 $(top_builddir)/config.status

x08f.f: x08f.fm4 $(top_builddir)/config.status

x09f.f: x09f.fm4 $(top_builddir)/config.status

x10f.f: x10f.fm4 $(top_builddir)/config.status

x11f.f: x11f.fm4 $(top_builddir)/config.status

x12f.f: x12f.fm4 $(top_builddir)/config.status

x13f.f: x13f.fm4 $(top_builddir)/config.status

x14f.f: x14f.fm4 $(top_builddir)/config.status

x15f.f: x15f.fm4 $(top_builddir)/config.status

x16f.f: x16f.fm4 $(top_builddir)/config.status

x16af.f: x16af.fm4 $(top_builddir)/config.status

x17f.f: x17f.fm4 $(top_builddir)/config.status

x18f.f: x18f.fm4 $(top_builddir)/config.status

x22f.f: x22f.fm4 $(top_builddir)/config.status

SUFFIXES = .fm4

.fm4.f: 
if with_double
	cp $< $*.f
else
	sed -f $(srcdir)/double2single.sed $< > $*.f
endif

if enable_f77

examples_f77dir = $(DATA_DIR)/examples/f77
examples_f77_DATA = $(f77_examples) Makefile.examples

libplplot = $(top_builddir)/src/libplplot$(LIB_TAG).la
libplplotf77c = $(top_builddir)/bindings/f77/libplplotf77c$(LIB_TAG).la
libplplotf77 = $(top_builddir)/bindings/f77/libplplotf77$(LIB_TAG).la

check_PROGRAMS = \
	x01f \
	x02f \
	x03f \
	x04f \
	x05f \
	x06f \
	x07f \
	x08f \
	x09f \
	x10f \
	x11f \
	x12f \
	x13f \
	x14f \
	x15f \
	x16f \
	x16af \
	x17f \
	x18f \
	x22f

nodist_x01f_SOURCES = x01f.f
nodist_x02f_SOURCES = x02f.f
nodist_x03f_SOURCES = x03f.f
nodist_x04f_SOURCES = x04f.f
nodist_x05f_SOURCES = x05f.f
nodist_x06f_SOURCES = x06f.f
nodist_x07f_SOURCES = x07f.f
nodist_x08f_SOURCES = x08f.f
nodist_x09f_SOURCES = x09f.f
nodist_x10f_SOURCES = x10f.f
nodist_x11f_SOURCES = x11f.f
nodist_x12f_SOURCES = x12f.f
nodist_x13f_SOURCES = x13f.f
nodist_x14f_SOURCES = x14f.f
nodist_x15f_SOURCES = x15f.f
nodist_x16f_SOURCES = x16f.f
nodist_x16af_SOURCES = x16af.f
nodist_x17f_SOURCES = x17f.f
nodist_x18f_SOURCES = x18f.f
nodist_x22f_SOURCES = x22f.f

LDADD = $(libplplot) $(libplplotf77c) $(libplplotf77)

install-data-hook:
	( cd $(DESTDIR)$(examples_f77dir) ; mv -f Makefile.examples Makefile )

else

# The following empty rule is needed because we are in the "else" part of
# an AM_CONDITIONAL
install-data-hook:

# enable_f77
endif

CLEANFILES = $(f77_examples)
MAINTAINERCLEANFILES = Makefile.in
# Note, all of these (including README.f77demos) should not be installed, but
# should all be in the build tree.
EXTRA_DIST = $(f77m4_sources) double2single.sed README.f77demos
