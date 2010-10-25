# src/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002 Rafael Laboissiere
# Copyright (C) 2002, 2004 Alan W. Irwin
# Copyright (C) 2004 Andrew Ross
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

lib_LTLIBRARIES = libplplot@LIB_TAG@.la

version = -version-info $(SOVERSION)

sources = \
  pdfutils.c \
  plargs.c \
  plbox.c \
  plcont.c \
  plcore.c \
  plctrl.c \
  plcvt.c \
  pldtik.c \
  plfill.c \
  plfreetype.c \
  plhist.c \
  plimage.c \
  plline.c \
  plmap.c \
  plot3d.c \
  plpage.c \
  plsdef.c \
  plshade.c \
  plstripc.c \
  plsym.c \
  pltick.c \
  plvpor.c \
  plwind.c \
  plbuf.c \
  plgridd.c \
  plvect.c

AM_CPPFLAGS = -I$(top_srcdir)/include $(FREETYPEINCCMD) $(INCLTDL) \
  $(UNICODEINCS)

libplplot@LIB_TAG@_la_SOURCES  = $(sources)
libplplot@LIB_TAG@_la_LDFLAGS  = $(version) $(RPATH) -no-undefined \
  $(FREETYPELIBCMD) $(CSALIBS) $(NNLIBS) -lm $(UNICODELIBS)
if !enable_dyndrivers
libplplot@LIB_TAG@_la_LIBADD = $(top_builddir)/drivers/libplplotdrv.la
else
libplplot@LIB_TAG@_la_LIBADD = $(LIBLTDL)
endif

MAINTAINERCLEANFILES = Makefile.in
