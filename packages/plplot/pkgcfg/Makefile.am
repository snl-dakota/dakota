# Makefile.am for PLplot (directory pkgcfg/)
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2003, 2004  Rafael Laboissiere
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

### Where?

pkgconfigdir = $(PKG_CONFIG_DIR)
docdir = $(DOC_DIR)

### What?

if with_pkg_config

pkgconfig_DATA = plplot@LIB_TAG@.pc

if enable_tcl
pkgconfig_DATA += plplot@LIB_TAG@-tcl.pc
endif

if enable_cxx
pkgconfig_DATA += plplot@LIB_TAG@-c++.pc
endif

if enable_f77
pkgconfig_DATA += plplot@LIB_TAG@-f77.pc
endif

### How?

$(pkgconfig_DATA): pc-stamp
pc-stamp: gen-pc-files.pl
	$(PERL) gen-pc-files.pl
	touch pc-stamp

endif

install-data-hook:
if with_pkg_config
	$(mkinstalldirs) $(DESTDIR)$(docdir)
	$(INSTALL) README $(DESTDIR)$(docdir)/README.pkg-config
endif

EXTRA_DIST = README
CLEANFILES = *.pc pc-stamp
MAINTAINERCLEANFILES = Makefile.in
