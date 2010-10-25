# include/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002 Rafael Laboissiere
# Copyright (C) 2002 Alan W. Irwin
# Copyright (C) 2005 Thomas J. Duck
# 
#This file is part of PLplot.
#
#PLplot is free software; you can redistribute it and/or modify
#it under the terms of the GNU Library General Public License as published by
#the Free Software Foundation; version 2 of the License.
#
#PLplot is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU Library General Public License for more details.
#
#You should have received a copy of the GNU Library General Public License
#along with the file PLplot; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

# Installed header files that should be distributed
dist_pkginclude_HEADERS = \
	disptab.h \
	drivers.h \
	pdf.h \
	pldebug.h \
	plevent.h \
	plplot.h \
	plplotP.h \
	plstrm.h \
	plxwd.h \
	gcw.h \
	plplotcanvas.h

# Installed header files that should not be distributed
nodist_pkginclude_HEADERS = \
	plConfig.h \
	plDevs.h

# uninstalled header files that should be distributed
noinst_HEADERS = \
	metadefs.h \
	plcore.h \
	plfreetype.h \
	ps.h \
	plhershey-unicode.h \
	plunicode-type1.h \
	plfci-type1.h \
	plfci-truetype.h \
	plplotcanvas-hacktext.h

##BUILT_SOURCES = plhershey-unicode.h
##MAINTAINERCLEANFILES = plhershey-unicode.h Makefile.in
MAINTAINERCLEANFILES = Makefile.in plplot_config.h.in

##noinst_PROGRAMS = plhershey-unicode-gen
##plhershey_unicode_gen_SOURCES = $(top_srcdir)/fonts/plhershey-unicode-gen.c

##plhershey-unicode.h: plhershey-unicode-gen \
##	$(top_srcdir)/fonts/plhershey-unicode.csv
##	./plhershey-unicode-gen \
##	$(top_srcdir)/fonts/plhershey-unicode.csv plhershey-unicode.h
