# bindings/octave/PLplot/support/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002  Alan W. Irwin
# Copyright (C) 2002, 2003 Joao Cardoso
# Copyright (C) 2003  Rafael Laboissiere
#
#This file is part of PLplot.
#
#PLplot is free software; you can redistribute it and/or modify
#it under the terms of the GNU Library General Public License as published by
#the Free Software Foundation; version 2 of the License.
#
#PLplot is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU Library General Public License for more details.
#
#You should have received a copy of the GNU Library General Public License
#along with the file PLplot; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

m_files = \
	__comet.m \
	__pl_contour.m \
	__pl_draw_legend.m \
	__pl_fill.m \
	__pl_init.m \
	__pl_logplotit.m \
	__pl_matstr.m \
	__pl_mesh.m \
	__pl_meshplotit.m \
	__pl_opt.m \
	__pl_plenv.m \
	__pl_plot3.m \
	__pl_plotit.m \
	__pl_pltext.m \
	__pl_polargrid.m \
	__pl_store.m \
	__plr1__.m \
	__plr2__.m \
	__plr__.m \
	__plt1__.m \
	__plt2__.m \
	__plt2mm__.m \
	__plt2mv__.m \
	__plt2ss__.m \
	__plt2vm__.m \
	__plt2vv__.m \
	__plt__.m \
	is_strmatrix.m \
	is_strvector.m

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = $(m_files)

if enable_octave

plplot_octave_supportdir = $(PLPLOT_OCTAVE_DIR)/support
plplot_octave_support_DATA = $(m_files)

endif
