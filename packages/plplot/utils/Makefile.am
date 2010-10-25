# drivers/Makefile.am for PLplot
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

libplplot = $(top_builddir)/src/libplplot$(LIB_TAG).la
libplplottcltk = $(top_builddir)/bindings/tcl/libplplottcltk$(LIB_TAG).la

AM_CPPFLAGS = -I$(top_srcdir)/include $(INCLTDL) -I$(top_srcdir)/bindings/tcl \
              $(TCLINCCMD) $(ITCLINCCMD)

bin_PROGRAMS = plrender pltek 

if enable_tcl
bin_PROGRAMS += pltcl
pltcl_SOURCES = pltcl.c
pltcl_LDFLAGS = $(RPATH) -no-undefined
pltcl_LDADD = $(libplplot) $(libplplottcltk)
endif

plrender_SOURCES = plrender.c
plrender_LDFLAGS = $(RPATH) -no-undefined
plrender_LDADD = $(libplplot)

pltek_SOURCES = pltek.c
pltek_LDFLAGS = $(RPATH) -no-undefined
pltek_LDADD = $(libplplot)

MAINTAINERCLEANFILES = Makefile.in

### FIXME: Brute force inclusion in dist tarball.  The files below may
### be treated in a different way for installation [RL, 2003-03-06]
EXTRA_DIST =      \
	tektest.c \
	xform.c
