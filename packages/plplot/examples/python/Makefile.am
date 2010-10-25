# examples/python/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002, 2004  Alan W. Irwin
# Copyright (C) 2004  Rafael Laboissiere
# Copyright (C) 2004  Andrew Ross
# Copyright (C) 2005  Thomas J. Duck
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

if enable_python

examples_pythondir = $(DATA_DIR)/examples/python

# This list contains standalone scripts which are executable.
dist_examples_python_SCRIPTS = \
	plplotcanvas_demo.py \
	plplotcanvas_animation.py \
	prova.py \
	pythondemos.py \
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
	x22 \
	x23 \
	xw14.py \
	xw17.py

# This list contains python modules that can be imported.
dist_examples_python_DATA = \
	README.pythondemos \
	README.plplotcanvas \
	qplplot.py \
	xw01.py \
	xw02.py \
	xw03.py \
	xw04.py \
	xw05.py \
	xw06.py \
	xw07.py \
	xw08.py \
	xw09.py \
	xw10.py \
	xw11.py \
	xw12.py \
	xw13.py \
	xw15.py \
	xw16.py \
	xw18.py \
	xw19.py \
	xw22.py \
	xw23.py

nodist_examples_python_DATA = plplot_python_start.py

CLEANFILES = *.pyc

check_DATA = \
        .check

.check: $(dist_examples_python_SCRIPTS) $(dist_examples_python_DATA)
	( if [ ! -f x01 ] ; then \
	    cd $(top_srcdir)/examples/python ; \
	    for file in $(dist_examples_python_SCRIPTS) $(dist_examples_python_DATA) ; do \
	      rm -f $(BUILD_DIR)/examples/python/$$file; $(LN_S) $(top_srcdir)/examples/python/$$file $(BUILD_DIR)/examples/python/$$file ; \
	    done ; \
	  fi )
	touch .check


endif

MAINTAINERCLEANFILES = Makefile.in

### FIXME: Brute force inclusion in dist tarball.  The files below may
### be treated in a different way for installation [RL, 2003-03-06]
EXTRA_DIST = \
	pytkdemo \
	tutor.py \
	x01.py \
	x02.py \
	x03.py \
	x04.py \
	x05.py \
	x06.py \
	x07.py \
	x08.py \
	x09.py \
	x10.py \
	x11.py \
	x12.py \
	x13.py \
	x14.py \
	x15.py \
	x16.py \
	x17.py \
	x18.py \
	x19.py \
	xtkpy
