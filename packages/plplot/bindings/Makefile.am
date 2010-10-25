# bindings/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002, 2003  Alan W. Irwin
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
# along with the file PLplot; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

SUBDIRS = c++ f77 python tcl tk octave java

abovedatadir = $(DATA_DIR)

if enable_tcl
  index_tcl = tcl/pkgIndex.tcl
endif

if enable_tk
  index_tk = tk/pkgIndex.tcl
endif

if enable_tkwin
  index_tkwin = tk-x-plat/pkgIndex.tcl
endif

index = $(index_tcl) $(index_tk) $(index_tkwin)

if enable_tcl
abovedata_DATA = pkgIndex.tcl

pkgIndex.tcl: $(index)
	cat $(index) > pkgIndex.tcl

endif
CLEANFILES = pkgIndex.tcl
DISTCLEANFILES = $(index)
MAINTAINERCLEANFILES = Makefile.in

### FIXME: Brute force inclusion in dist tarball.  The files below may
### be treated in a different way for installation [RL, 2003-03-06]
EXTRA_DIST =         \
	tk-x-plat    \
	swig-support

dist-hook:
	rm -rf $(distdir)/tk-x-plat/pkgIndex.tcl
