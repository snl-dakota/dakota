# examples/java/Makefile.am for PLplot ###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002, 2004  Alan W. Irwin
# Copyright (C) 2004  Andrew Ross
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

javafiles = \
	$(top_srcdir)/examples/java/x01.java \
	$(top_srcdir)/examples/java/x02.java \
	$(top_srcdir)/examples/java/x03.java \
	$(top_srcdir)/examples/java/x04.java \
	$(top_srcdir)/examples/java/x05.java \
	$(top_srcdir)/examples/java/x06.java \
	$(top_srcdir)/examples/java/x07.java \
	$(top_srcdir)/examples/java/x08.java \
	$(top_srcdir)/examples/java/x09.java \
	$(top_srcdir)/examples/java/x10.java \
	$(top_srcdir)/examples/java/x11.java \
	$(top_srcdir)/examples/java/x12.java \
	$(top_srcdir)/examples/java/x13.java \
	$(top_srcdir)/examples/java/x14.java \
	$(top_srcdir)/examples/java/x15.java \
	$(top_srcdir)/examples/java/x16.java \
	$(top_srcdir)/examples/java/x17.java \
	$(top_srcdir)/examples/java/x18.java \
	$(top_srcdir)/examples/java/x19.java \
	$(top_srcdir)/examples/java/x22.java \
	$(top_srcdir)/examples/java/x23.java

javademos = \
	$(top_srcdir)/examples/java/README.javademos \
	$(javafiles) \
	Makefile.examples

# These are for make check compilation of examples.
PLPLOT_CLASSPATH= `pwd`/$(top_builddir)/bindings/java

example_classes = \
	x01.class \
	x02.class \
	x03.class \
	x04.class \
	x05.class \
	x06.class \
	x07.class \
	x08.class \
	x09.class \
	x10.class \
	x11.class \
	x12.class \
	x13.class \
	x14.class \
	x15.class \
	x16.class \
	x17.class \
	x18.class \
	x19.class \
	x22.class \
	x23.class

if enable_java

# Duplicate this so the source appears in the examples/java directory for
# the user to read
examples_javadir = $(DATA_DIR)/examples/java
examples_java_DATA = $(javademos)

# Note the Java binding classes are actually generated in the java bindings
# directory (path = "plplot.core").  Since the examples classes are #
# generated here (path = "plplot.examples") but both must be included in the
# jar file, this later is generated here.

plplotjavadir = $(datadir)/java
plplotjava_DATA = plplot.jar

$(top_builddir)/bindings/java/jar-stamp:
	( cd $(top_builddir)/bindings/java ; make jar-stamp )

plplot.jar: $(example_classes) $(top_builddir)/bindings/java/jar-stamp \
            $(top_srcdir)/examples/java/README.javademos $(javafiles)
	$(mkdir_p) plplot/examples
	cp $(top_srcdir)/examples/java/README.javademos $(javafiles) $(example_classes) plplot/examples
	$(JAR) -cf plplot.jar \
	       -C $(top_builddir)/bindings/java/ plplot/core \
	       -C . plplot/examples

endif

install-data-hook:
if enable_java
	( cd $(DESTDIR)$(examples_javadir) ; mv -f Makefile.examples Makefile )
endif

.java.class:
	$(JAVAC) $(AM_JAVACFLAGS) $(JAVACFLAGS) $< -d . -classpath $(PLPLOT_CLASSPATH)
	cp plplot/examples/$@ .

CLEANFILES = $(example_classes) plplot.jar

MAINTAINERCLEANFILES = Makefile.in

clean-local:
	rm -rf plplot

EXTRA_DIST = \
	$(top_srcdir)/examples/java/README.javademos \
	$(javafiles)
