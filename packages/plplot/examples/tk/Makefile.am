# examples/tk/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002, 2004  Alan W. Irwin
# Copyright (C) 2004  Rafael Laboissiere
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

if enable_tk

examples_tkdir = $(DATA_DIR)/examples/tk

if enable_itcl
mkindex = $(top_srcdir)/scripts/mktclIndex -itcl
else
mkindex = $(top_srcdir)/scripts/mktclIndex -tcl
endif

check_DATA = \
	x01.tcl \
	x02.tcl \
	x03.tcl \
	x04.tcl \
	x05.tcl \
	x06.tcl \
	x07.tcl \
	x08.tcl \
	x09.tcl \
	x10.tcl \
	x11.tcl \
	x12.tcl \
	x13.tcl \
	x14.tcl \
	x15.tcl \
	x16.tcl \
	x17.tcl \
	x18.tcl \
	x19.tcl \
	x22.tcl

### x01.tcl stands symbolically for x??.tcl.  If specify $(check_DATA)
### instead, then for loop below is usually incorrectly repeated for each file
### in check_DATA (depending on coarseness of timer).
x01.tcl: \
	$(top_srcdir)/examples/tcl/x01.tcl \
	$(top_srcdir)/examples/tcl/x02.tcl \
	$(top_srcdir)/examples/tcl/x03.tcl \
	$(top_srcdir)/examples/tcl/x04.tcl \
	$(top_srcdir)/examples/tcl/x05.tcl \
	$(top_srcdir)/examples/tcl/x06.tcl \
	$(top_srcdir)/examples/tcl/x07.tcl \
	$(top_srcdir)/examples/tcl/x08.tcl \
	$(top_srcdir)/examples/tcl/x09.tcl \
	$(top_srcdir)/examples/tcl/x10.tcl \
	$(top_srcdir)/examples/tcl/x11.tcl \
	$(top_srcdir)/examples/tcl/x12.tcl \
	$(top_srcdir)/examples/tcl/x13.tcl \
	$(top_srcdir)/examples/tcl/x14.tcl \
	$(top_srcdir)/examples/tcl/x15.tcl \
	$(top_srcdir)/examples/tcl/x16.tcl \
	$(top_srcdir)/examples/tcl/x17.tcl \
	$(top_srcdir)/examples/tcl/x18.tcl \
	$(top_srcdir)/examples/tcl/x19.tcl \
	$(top_srcdir)/examples/tcl/x22.tcl
	( cd $(top_srcdir)/examples/tcl ; \
	  for file in x??.tcl ; do \
	    rm -f $(BUILD_DIR)/examples/tk/$$file; $(LN_S) $(top_srcdir)/examples/tcl/$$file $(BUILD_DIR)/examples/tk/$$file ; \
	  done )

data_files = \
	README.tkdemos \
	runAllDemos.tcl \
	runExtendedDemos.tcl \
	tkdemos.tcl \
	xtk01.c \
	xtk02.c \
	xtk04.c

dist_examples_tk_DATA = $(data_files)

nodist_examples_tk_DATA = Makefile.examples tclIndex

tclIndex: x01.tcl
	$(mkindex)

examples_tk_SCRIPTS = \
	tk01 \
	tk02 \
	tk03 \
	tk04

libplplot = $(top_builddir)/src/libplplot$(LIB_TAG).la
libplplottcltk = $(top_builddir)/bindings/tcl/libplplottcltk$(LIB_TAG).la

check_PROGRAMS = \
	xtk01 \
	xtk02 \
	xtk04

cflags = -I$(top_srcdir)/include -I$(top_srcdir)/bindings/tk -I$(top_srcdir)/bindings/tcl $(TCLINCCMD) $(TKINCCMD)
xtk01_CFLAGS = $(cflags)
xtk02_CFLAGS = $(cflags)
xtk04_CFLAGS = $(cflags)

xtk01_LDFLAGS = $(libplplot) $(libplplottcltk)
xtk02_LDFLAGS = $(libplplot) $(libplplottcltk)
xtk04_LDFLAGS = $(libplplot) $(libplplottcltk)

# enable_tk
endif

### Assumes tcl install is done before tk install.
install-data-hook:
if enable_tk
	( cd $(DESTDIR)$(examples_tkdir) ; \
	  mv -f Makefile.examples Makefile ; \
	  cd ../tcl ; \
	  for file in x??.tcl ; do \
	    rm -f ../tk/$$file; $(LN_S) ../tcl/$$file ../tk/$$file ; \
	  done )
endif

EXTRA_DIST = $(data_files) Makefile.examples.in $(examples_tk_SCRIPTS)
CLEANFILES = tclIndex $(check_DATA) check_stamp
DISTCLEANFILES = Makefile.examples
MAINTAINERCLEANFILES = Makefile.in

