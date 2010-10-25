# examples/tcl/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002  Alan W. Irwin
# Copyright (C) 2004  Rafael Laboissiere
# Copyright (C) 2004  Andrew Ross
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

if enable_tcl

examples_tcldir = $(DATA_DIR)/examples/tcl

if enable_itcl
  mkindex = ../../scripts/mktclIndex -itcl
else
  mkindex = ../../scripts/mktclIndex -tcl
endif


dist_examples_tcl_DATA = \
	README.tcldemos \
	plgrid.tcl \
	plot.dat \
	plot.tcl \
	r.dat \
	stats.log \
	tcldemos.tcl \
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

nodist_examples_tcl_DATA = $(top_srcdir)/examples/tcl/tclIndex

dist_examples_tcl_SCRIPTS = \
	x01 \
	x02 \
	x03 \
	x04 \
	x05 \
	x06 \
	x07 \
	x08 \
	x09 \
	x10 \
	x11 \
	x12 \
	x13 \
	x14 \
	x15 \
	x16 \
	x17 \
	x18 \
	x19 \
	x22

$(top_srcdir)/examples/tcl/tclIndex:
	cd $(top_srcdir)/examples/tcl ; \
	$(mkindex)

CLEANFILES = $(top_srcdir)/examples/tcl/tclIndex

check_DATA = \
	.check

.check: $(dist_examples_tcl_SCRIPTS) $(dist_examples_tcl_DATA)
	( if [ ! -f x01 ] ; then \
	 cd $(top_srcdir)/examples/tcl ; \
	 for file in $(dist_examples_tcl_SCRIPTS) $(dist_examples_tcl_DATA) ; do \
	   rm -f $(BUILD_DIR)/examples/tcl/$$file; $(LN_S) $(top_srcdir)/examples/tcl/$$file $(BUILD_DIR)/examples/tcl/$$file ; \
	 done ; \
	 fi )
	touch .check 

endif

MAINTAINERCLEANFILES = Makefile.in
