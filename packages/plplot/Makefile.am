# Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002, 2003, 2004  Alan W. Irwin
# Copyright (C) 2003  Joao Cardoso
# Copyright (C) 2003, 2004  Rafael Laboissiere
# Copyright (C) 2005  Thomas J. Duck
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

# This flag only required for top-level Makefile.am.  
# It insures you only have to run cf/bootstrap.sh once on the first
# clean install.  After that, aclocal (and automake, autoconf, and 
# ./configure) called automatically whenever required by a change to 
# configure.ac or any file in the cf subdirectory.

ACLOCAL_AMFLAGS = -I cf 2>&1 | grep -v underquoted
# The above is an inspired hack which may not work indefinitely.  
# Return to simpler more reliable form below as soon as 
# automake-1.8 underquoted warning messages quit being such an annoyance.
#ACLOCAL_AMFLAGS = -I cf

# Note: Must build the python/gcw binding *after* building drivers.  TJD

src_dirs = fonts lib include src data bindings
##other_dirs = bindings/python/gcw examples utils test scripts doc pkgcfg cf
other_dirs = bindings/python/gcw examples test scripts doc pkgcfg cf

if enable_dyndrivers
  SUBDIRS =
if !with_ltdlsystem
    SUBDIRS += libltdl
endif
  SUBDIRS += $(src_dirs) drivers
else
  SUBDIRS = drivers $(src_dirs)
endif

SUBDIRS += $(other_dirs)

##DIST_SUBDIRS = libltdl $(src_dirs) drivers $(other_dirs)
if !with_ltdlsystem
  DIST_SUBDIRS = $(src_dirs) drivers $(other_dirs)
else
  DIST_SUBDIRS = libltdl $(src_dirs) drivers $(other_dirs)
endif

bin_SCRIPTS = plplot_libtool

plplot_libtool: libtool
	cp libtool plplot_libtool

docdir = $(datadir)/doc/plplot

doc_DATA = \
	AUTHORS \
	COPYING.LIB \
	ChangeLog \
	Copyright \
	FAQ \
	NEWS \
	PROBLEMS \
	README \
	README.release

CLEANFILES = plplot_libtool jadeout.log config.summary

clean-local:
	-rm -fr libltdl/.libs

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 plplot_config.h.in configure \
		       cf/aclocal.m4 cf/config.guess cf/config-h.in \
		       cf/config.sub cf/configure cf/install-sh \
		       cf/ltmain.sh cf/missing libltdl/Makefile.in \
		       libltdl/config.log libltdl/config.status \
		       libltdl/libtool libltdl/plplot_ltdl_config.h \
		       libltdl/stamp-h1 libltdl/aclocal.m4 \
		       libltdl/config.guess libltdl/config.sub \
		       libltdl/configure libltdl/install-sh libltdl/ltmain.sh \
		       libltdl/missing libltdl/Makefile 

maintainer-clean-local:
	-rm -fr libltdl/autom4te.cache libltdl/.libs

EXTRA_DIST = \
	rpm \
	sys \
	Copyright \
	FAQ \
	PROBLEMS \
	README.developers \
	README.release \
	SERVICE \
	TODO.AM-LT \
	ToDo

dist-hook:
	rm -rf `find $(distdir) -name CVS -o -name .cvsignore`
