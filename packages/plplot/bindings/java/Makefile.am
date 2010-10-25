# bindings/java/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002, 2003, 2004  Alan W. Irwin
# Copyright (C) 2003, 2004  Rafael Laboissiere
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

SWIG_SUPPORT_DIR = $(top_srcdir)/bindings/swig-support
PLPLOTCAPI_I = $(SWIG_SUPPORT_DIR)/plplotcapi.i
SWIG_JAVA_DIR = $(top_srcdir)/bindings/java

SWIG_CMD = $(SWIG) -I$(SWIG_SUPPORT_DIR) -I$(SWIG_JAVA_DIR) -DPL_DOUBLE -DSWIG_JAVA -java 

swigjavafiles = \
	plplotjavacJNI.java \
	SWIGTYPE_p_p_char.java \
	plplotjavac.java 

# This required for swig-1.3.21, but not for swig-1.3.17
swigjavafiles += plplotjavacConstants.java

swigcfiles = plplotjavac_wrap.c

swiggenfiles = $(swigjavafiles) $(swigcfiles)

javafiles = \
	$(top_srcdir)/bindings/java/PLStream.java \
	config.java \
	$(swigjavafiles)

# Note : the order of these is important since there is no formal 
# dependency checking.
javaclasses = \
	config.class \
	plplotjavacJNI.class \
	SWIGTYPE_p_p_char.class\
	plplotjavacConstants.class \
	plplotjavac.class \
	PLStream.class

### FIXME: Brute force inclusion in dist tarball.  The files below may
### be treated in a different way for installation [RL, 2003-03-06]
EXTRA_DIST = \
##	PLStream.java \
	README.javaAPI \
	plplotjavac.i
##	plplotjavac.i \
##	$(swiggenfiles)

$(swiggenfiles): $(PLPLOTJAVAC_I) plplotjavac.i $(PLPLOTCAPI_I)
	$(SWIG_CMD) -package plplot.core plplotjavac.i

# Must have "exec" in the name.
execjavawrapperdir = $(JAVAWRAPPER_DIR)

libplplot = $(top_builddir)/src/libplplot$(LIB_TAG).la

AM_CPPFLAGS = -I$(top_srcdir)/include $(INCLTDL) $(JAVAINCCMD)

nodist_plplotjavac_wrap_la_SOURCES  = plplotjavac_wrap.c

# no-undefined should work on all platforms here since libplplot should
# resolve all symbols.
plplotjavac_wrap_la_LDFLAGS = \
	-rpath $(execjavawrapperdir) \
	-module \
	-avoid-version \
	-no-undefined \
	$(libplplot)

if enable_java

# Use javaJAVA for gcj support once gcc-3.2 use becomes
# widespread.

# Can't use java support for now since jikes doesn't handle dependencies 
# properly - instead do it using DATA and with an explicit rule.
#noinst_JAVA = $(javafiles)

.java.class: 
	$(JAVAC) $(AM_JAVACFLAGS) $(JAVACFLAGS) $< -d . -classpath .
	cp plplot/core/$@ .

noinst_DATA = jar-stamp

jar-stamp: $(javaclasses) $(javafiles) $(top_srcdir)/bindings/java/README.javaAPI
	$(mkdir_p) plplot/core
	cp $(javafiles) $(top_srcdir)/bindings/java/README.javaAPI plplot/core
	touch jar-stamp

execjavawrapper_LTLIBRARIES = plplotjavac_wrap.la

# if enable_java
endif

# Remove unneeded *.a *.la files that are installed by Libtool
install-exec-hook:
if enable_java
	( cd $(DESTDIR)$(execjavawrapperdir) ; \
	  rm -f *.a *.la )
endif

clean-local:
	rm -rf plplot

CLEANFILES = $(javaclasses) jar-stamp

MAINTAINERCLEANFILES = $(swiggenfiles) Makefile.in
