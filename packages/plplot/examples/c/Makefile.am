# examples/c/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002, 2004  Alan W. Irwin
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

AM_CPPFLAGS = -I$(top_srcdir)/include $(INCLTDL)
examples_cdir = $(DATA_DIR)/examples/c
sources = \
	plcdemos.h \
	tutor.c \
	x01c.c \
	x02c.c \
	x03c.c \
	x04c.c \
	x05c.c \
	x06c.c \
	x07c.c \
	x08c.c \
	x09c.c \
	x10c.c \
	x11c.c \
	x12c.c \
	x13c.c \
	x14c.c \
	x15c.c \
	x16c.c \
	x17c.c \
	x18c.c \
	x19c.c \
	x20c.c \
	x21c.c \
	x22c.c \
	x23c.c \
	x24c.c

examples_c_DATA = \
	Makefile.examples

dist_examples_c_DATA = \
	plplotcanvas_demo.c \
	plplotcanvas_animation.c \
	README.plplotcanvas \
	lena.pgm \
	$(sources)

install-data-hook:
	( cd $(DESTDIR)$(examples_cdir) ; mv -f Makefile.examples Makefile )

libplplot = $(top_builddir)/src/libplplot$(LIB_TAG).la

check_PROGRAMS = \
	x01c \
	x02c \
	x03c \
	x04c \
	x05c \
	x06c \
	x07c \
	x08c \
	x09c \
	x10c \
	x11c \
	x12c \
	x13c \
	x14c \
	x15c \
	x16c \
	x17c \
	x18c \
	x19c \
	x20c \
	x21c \
	x22c \
	x23c

LDADD = $(libplplot)

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = Makefile.examples.in
