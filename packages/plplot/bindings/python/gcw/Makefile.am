# bindings/python/gcw/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002, 2003, 2004  Alan W. Irwin
# Copyright (C) 2003, 2004  Rafael Laboissiere
# Copyright (C) 2005  Thomas J. Duck
#
# This file is part of PLplot.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; version 2 of the License.
#
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with the file; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

# N.B.  This treatment probably will need changes or else a complete
# replacement to work cross-platform as indicated by ?cross-platform?
# comment below, but it works quite nicely on Linux.

### FIXME: Brute force inclusion in dist tarball.  The files below may
### be treated in a different way for installation [RL, 2003-03-06]
EXTRA_DIST = \
	gcwmodule.c \
	plplotcanvas.py \
        cplplotcanvasmodule.c \
	cplplotcanvas.override
###	cplplotcanvas.override \
###	$(MAINTAINERCLEANFILES)


# Must have "exec" in the name.
ourexecpythondir = $(pythondir)

libplplot = $(top_builddir)/src/libplplot$(LIB_TAG).la

libgcw = $(top_builddir)/drivers/libgcw.la


# NUMERICINCCMD only required until we quit supporting python 1.5
# X_CFLAGS required (on some platforms, e.g., netbsd) by plplot_widgetmodule.c
AM_CPPFLAGS = -I$(top_srcdir)/include $(INCLTDL) $(PYTHONINCCMD) $(NUMERICINCCMD) $(X_CFLAGS)

# For Mac OS X/fink we have found that dropping -no-undefined $(PYTHON_LDFLAGS)
# solves an unknown linking problem that causes python to be unable to import
# the extension module, but this hack is not necessary on other platforms.
# and it causes errors for Cygwin.  We also hope this hack
# won't be necessary in the future when we have better versions of
# libtool/Mac OS X/fink.
if darwin_os
extra_ldflags = 
else
# -no-undefined should work on all (other) platforms here since symbols should 
# all be resolved by $(PYTHON_LDFLAGS) and libplplot
extra_ldflags = -no-undefined $(PYTHON_LDFLAGS)
endif

UNDERSCORE_plplotcmodule_la_LDFLAGS  = \
	-rpath $(ourexecpythondir) \
	-module \
	-avoid-version \
	$(extra_ldflags) \
	$(libplplot)

PYGTK_DEFS=`pkg-config --variable=defsdir pygtk-2.0`

cplplotcanvas.c: plplotcanvas.defs cplplotcanvas.override            
	pygtk-codegen-2.0 --prefix cplplotcanvas \
	--register $(PYGTK_DEFS)/gdk.defs \
	--register $(PYGTK_DEFS)/gtk.defs \
	--register $(PYGTK_DEFS)/gnome.defs \
	--register $(PYGTK_DEFS)/canvas.defs \
	--override cplplotcanvas.override \
	plplotcanvas.defs > $@

plplotcanvas.defs: 
	/usr/share/pygtk/2.0/codegen/h2def.py \
		$(top_srcdir)/include/plplotcanvas.h \
		| sed -e s/PLINT/long/ -e s/PLFLT/double/ > plplotcanvas.defs

gcwmodule_la_SOURCES  = gcwmodule.c
gcwmodule_la_CFLAGS = $(GCW_CFLAGS) $(PYGCW_CFLAGS)
gcwmodule_la_LDFLAGS = $(UNDERSCORE_plplotcmodule_la_LDFLAGS) $(libgcw) \
	               $(GCW_LIBS) $(PYGCW_LIBS)

cplplotcanvasmodule_la_SOURCES  = cplplotcanvas.c cplplotcanvasmodule.c
cplplotcanvasmodule_la_CFLAGS = $(GCW_CFLAGS) $(PYGCW_CFLAGS)
cplplotcanvasmodule_la_LDFLAGS = $(UNDERSCORE_plplotcmodule_la_LDFLAGS) \
                                 $(libgcw) $(GCW_LIBS) $(PYGCW_LIBS)


if enable_python
if enable_pygcw

gcwmoduleexecpython_LTLIBRARIES = gcwmodule.la
gcwmoduleexecpythondir = $(pythondir)

cplplotcanvasmoduleexecpython_LTLIBRARIES = cplplotcanvasmodule.la
cplplotcanvasmoduleexecpythondir = $(pythondir)
cplplotcanvasmoduleexecpython_DATA = plplotcanvas.py


endif
endif

MAINTAINERCLEANFILES =    \
	cplplotcanvas.c   \
	plplotcanvas.defs \
	Makefile.in
