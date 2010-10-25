# bindings/octave/PLplot/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002  Alan W. Irwin
# Copyright (C) 2002, 2003  Joao Cardoso
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

SUBDIRS = support

binding_m_files = \
	arrow.m \
	arrows.m \
	autostyle.m \
	axis.m \
	axis_set.m \
	bgr.m \
	blue.m \
	bone.m \
	bottom_title.m \
	clip.m \
	closeallfig.m \
	closefig.m \
	colormap.m \
	comet.m \
	contour.m \
	cool.m \
	copper.m \
	drawnow.m \
	fig.m \
	fig_raise.m \
	fig_state.m \
	figure.m \
	fill.m \
	free_fig.m \
	ginput.m \
	green.m \
	grid.m \
	griddata.m \
	gtext.m \
	hls2rgb.m \
	hot.m \
	hsv.m \
	label_plot.m \
	legend.m \
	loglog.m \
	lp_setup.m \
	mesh.m \
	meshc.m \
	meshz.m \
	mplot.m \
	multiplot.m \
	oneplot.m \
	peaks.m \
	pink.m \
	plclearplot.m \
	plclg.m \
	plcolormap.m \
	pldef.m \
	plimage.m \
	plot.m \
	plot3.m \
	plot_border.m \
	plot_margin.m \
	plrb.m \
	plsetopt.m \
	polar.m \
	prism.m \
	red.m \
	rgb2hls.m \
	rgbplot.m \
	save_fig.m \
	semilogx.m \
	semilogy.m \
	set_view.m \
	shade.m \
	shading.m \
	shg.m \
	stopdraw.m \
	stripc.m \
	stripc_add.m \
	stripc_del.m \
	struct_contains.m \
	subplot.m \
	subwindow.m \
	surf.m \
	surfc.m \
	surfl.m \
	tdeblank.m \
	text.m \
	title.m \
	top_title.m \
	xlabel.m \
	xticks.m \
	ylabel.m \
	yticks.m \
	zlabel.m \
	zoom.m

user_m_files = \
	use_plplot.m \
	plplot_octave_path.m \
	toggle_plplot_use.m

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = \
	$(binding_m_files) \
	struct_contains.m \
	use_plplot.m \
	toggle_plplot_use.m

if enable_octave

plplot_octavedir = $(PLPLOT_OCTAVE_DIR)
plplot_octave_DATA = $(binding_m_files)

octave_mdir = $(OCTAVE_M_DIR)/PLplot
octave_m_DATA = $(user_m_files)

endif


