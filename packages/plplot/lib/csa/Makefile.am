# Makefile.am for PLplot (lib/csa/)
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2003  Joao Cardoso
# Copyright (C) 2003, 2004  Rafael Laboissiere
# Copyright (C) 2004  Alan W. Irwin
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

lib_LTLIBRARIES = libcsirocsa.la
libcsirocsa_la_SOURCES = csa.c
libcsirocsa_la_LDFLAGS = -version-info $(CSIRO_SOVERSION) $(RPATH) \
                         -no-undefined -lm
AM_CPPFLAGS = -I.

docdir = $(DOC_DIR)
docfiles = README README.1st

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(docdir)
	(cd $(top_srcdir)/lib/csa ; \
	for i in $(docfiles) ; do \
	  $(INSTALL) $$i $(DESTDIR)$(docdir)/$$i.csa ; \
	done )

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = csa.h nan.h version.h $(docfiles)
